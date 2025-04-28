#include <GarrysMod/FactoryLoader.hpp>
#include "filesystem.h"
#include "lua.h"
#include "iluashared.h"
#include <GarrysMod/InterfacePointers.hpp>
#include "detours.h"
#include "module.h"
#include "util.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool Lua::PushHook(const char* hook, GarrysMod::Lua::ILuaInterface* pLua)
{
	if ( !pLua)
	{
		Warning(PROJECT_NAME ": Lua::PushHook was called while pLua was NULL! (%s)\n", hook);
		return false;
	}

	if (!ThreadInMainThread())
	{
		Warning(PROJECT_NAME ": Lua::PushHook was called ouside of the main thread! (%s)\n", hook);
		return false;
	}

	pLua->GetField(LUA_GLOBALSINDEX, "hook");
		if (pLua->GetType(-1) != GarrysMod::Lua::Type::Table)
		{
			pLua->Pop(1);
			DevMsg(PROJECT_NAME ": Missing hook table!\n");
			return false;
		}

		pLua->GetField(-1, "Run");
			if (pLua->GetType(-1) != GarrysMod::Lua::Type::Function)
			{
				pLua->Pop(2);
				DevMsg(PROJECT_NAME ": Missing hook.Run function!\n");
				return false;
			} else {
				pLua->Remove(-2);
				pLua->PushString(hook);
			}

	return true;
}

void Lua::Init(GarrysMod::Lua::ILuaInterface* LUA)
{
	if (g_Lua)
	{
		Warning(PROJECT_NAME ": g_Lua is already Initialized! Skipping... (%p, %p)\n", g_Lua, LUA);
		return;
	}

	g_Lua = LUA;
	Lua::CreateLuaData(g_Lua, true);
	g_pModuleManager.LuaInit(g_Lua, false);
}

void Lua::ServerInit()
{
	if (g_Lua == NULL) {
		DevMsg(1, "Lua::ServerInit failed! g_Lua is NULL\n");
		return;
	}

	g_pModuleManager.LuaInit(g_Lua, true);
}

void Lua::Shutdown()
{
	g_pModuleManager.LuaShutdown(g_Lua);

#if HOLYLIB_UTIL_DEBUG_LUAUSERDATA
	g_pRemoveLuaUserData = false; // We are iterating over g_pLuaUserData so DON'T modify it.
	for (auto& ref : g_pLuaUserData)
	{
		// If it doesn't hold a reference of itself, the gc will take care of it & call __gc in the lua_close call.
		// This check only focuses on LuaUserData that holds a reference to itself as thoes are never freed by the GC even when lua_close is called.
		if (ref->GetReference() == -1)
			continue;

		if (Util::holylib_debug_mainutil.GetBool())
			Msg(PROJECT_NAME ": This should NEVER happen! Discarding of old userdata %p\n", ref);

		delete ref;
	}
	g_pRemoveLuaUserData = true;
	g_pLuaUserData.clear();
#endif
}

void Lua::FinalShutdown()
{
	Lua::RemoveLuaData(g_Lua);

	// g_Lua is bad at this point / the lua_State is already gone so we CAN'T allow any calls
	g_Lua = NULL;

	for (auto& ref : Util::g_pReference)
	{
		if (Util::holylib_debug_mainutil.GetBool())
			Msg(PROJECT_NAME ": This should NEVER happen! Discarding of old reference %i\n", ref);
	}
	Util::g_pReference.clear();
}

void Lua::ManualShutdown()
{
	if (!g_Lua)
		return;

	Lua::Shutdown();
	Lua::FinalShutdown();
}

static bool bManualShutdown = false;
static Detouring::Hook detour_InitLuaClasses;
static void hook_InitLuaClasses(GarrysMod::Lua::ILuaInterface* LUA) // ToDo: Add a hook to Lua::Startup or whatever it's name was and use that for the ServerInit.
{
	detour_InitLuaClasses.GetTrampoline<Symbols::InitLuaClasses>()(LUA);

	Lua::Init(LUA);
}

static Detouring::Hook detour_CLuaInterface_Shutdown;
static void hook_CLuaInterface_Shutdown(GarrysMod::Lua::ILuaInterface* LUA)
{
	if ((void*)LUA == (void*)g_Lua)
		Lua::Shutdown();

	detour_CLuaInterface_Shutdown.GetTrampoline<Symbols::CLuaInterface_Shutdown>()(LUA); 
	// Garbage collection will kick in so our remaining objects could call theirs __gc function so g_Lua still needs to be valid.

	Lua::FinalShutdown();
}

void Lua::AddDetour() // Our Lua Loader.
{
	if (!bManualShutdown)
	{
		SourceSDK::ModuleLoader server_loader("server");
		Detour::Create(
			&detour_InitLuaClasses, "InitLuaClasses",
			server_loader.GetModule(), Symbols::InitLuaClassesSym,
			(void*)hook_InitLuaClasses, 0
		);

		SourceSDK::ModuleLoader lua_shared_loader("lua_shared");
		Detour::Create(
			&detour_CLuaInterface_Shutdown, "CLuaInterface::Shutdown",
			lua_shared_loader.GetModule(), Symbols::CLuaInterface_ShutdownSym,
			(void*)hook_CLuaInterface_Shutdown, 0
		);
	}
}

void Lua::SetManualShutdown()
{
	bManualShutdown = true;
}

GarrysMod::Lua::ILuaInterface* Lua::GetRealm(unsigned char realm) {
	SourceSDK::FactoryLoader luashared_loader("lua_shared");
	GarrysMod::Lua::ILuaShared* LuaShared = (GarrysMod::Lua::ILuaShared*)luashared_loader.GetFactory()(GMOD_LUASHARED_INTERFACE, nullptr);
	if (LuaShared == nullptr) {
		Msg(PROJECT_NAME ": failed to get ILuaShared!\n");
		return nullptr;
	}

	return LuaShared->GetLuaInterface(realm);
}

GarrysMod::Lua::ILuaShared* Lua::GetShared() {
	SourceSDK::FactoryLoader luashared_loader("lua_shared");
	if ( !luashared_loader.GetFactory() )
		Msg(PROJECT_NAME ": About to crash!\n");

	return luashared_loader.GetInterface<GarrysMod::Lua::ILuaShared>(GMOD_LUASHARED_INTERFACE);
}

/*
 * Unlike HolyLib we store it ourself in a map and do not touch the base game.
 */
static std::unordered_map<GarrysMod::Lua::ILuaInterface*, Lua::StateData*> g_pLuaStateLinks;
Lua::StateData* Lua::GetLuaData(GarrysMod::Lua::ILuaInterface* LUA)
{
	auto it = g_pLuaStateLinks.find(LUA);
	if (it == g_pLuaStateLinks.end())
		return NULL;

	return it->second;
}

static std::unordered_set<Lua::StateData*> g_pLuaStates; // unused for now.
void Lua::CreateLuaData(GarrysMod::Lua::ILuaInterface* LUA, bool bNullOut)
{
	if (Lua::GetLuaData(LUA))
	{
		Msg("holytest - Skipping thread data creation since we already found data %p\n", Lua::GetLuaData(LUA));
		return;
	}

	Lua::StateData* data = new Lua::StateData;
	g_pLuaStateLinks[LUA] = data;
	Msg("holytest - Created thread data %p\n", data);
}

void Lua::RemoveLuaData(GarrysMod::Lua::ILuaInterface* LUA)
{
	auto data = Lua::GetLuaData(LUA);
	if (!data)
		return;

	g_pLuaStateLinks.erase(LUA);
	delete data;
	Msg("holytest - Removed thread data %p\n", data);
}

const std::unordered_set<Lua::StateData*>& Lua::GetAllLuaData()
{
	return g_pLuaStates;
}