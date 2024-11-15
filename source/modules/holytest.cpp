#include "module.h"
#include <GarrysMod/Lua/Interface.h>
#include "sourcesdk/GameEventManager.h"
#include "lua.h"
#include "detours.h"

class CHolyTestModule : public IModule
{
public:
	virtual void Init(CreateInterfaceFn* appfn, CreateInterfaceFn* gamefn) OVERRIDE;
	virtual void LuaInit(bool bServerInit) OVERRIDE;
	virtual void LuaShutdown() OVERRIDE;
	virtual const char* Name() { return "holytest"; };
	virtual int Compatibility() { return LINUX32; };
};

CHolyTestModule g_pHolyTestModule;
IModule* pHolyTestModule = &g_pHolyTestModule;

LUA_FUNCTION_STATIC(ServerExecute)
{
	Util::engineserver->ServerExecute();
	return 0;
}

LUA_FUNCTION_STATIC(UnregisterConVar)
{
	ConVar* pConVar = (ConVar*)Get_IConVar(1, true);

	g_pCVar->UnregisterConCommand(pConVar);
	LUA->SetUserType(1, NULL); // Set the reference to NULL
	//delete pConVar; // We don't delete it since we don't want to break anything.

	return 0;
}

static CGameEventManager* pManager;
LUA_FUNCTION_STATIC(GetEventListeners)
{
	if (LUA->IsType(1, GarrysMod::Lua::Type::String))
	{
		CGameEventDescriptor* desciptor = pManager->GetEventDescriptor(LUA->GetString(1));
		if (!desciptor)
			return 0; // Return nothing -> nil on failure

		LUA->PushNumber(desciptor->listeners.Count());
	} else {
		LUA->CreateTable();
			FOR_EACH_VEC(pManager->m_GameEvents, i)
			{
				CGameEventDescriptor& descriptor = pManager->m_GameEvents[i];
				LUA->PushNumber(descriptor.listeners.Count());
				LUA->SetField(-2, (const char*)&descriptor.name); // Does it even need to be a const char* ?
			}
	}

	return 1;
}

static IGameEventListener2* pLuaGameEventListener;
LUA_FUNCTION_STATIC(RemoveEventListener)
{
	const char* strEvent = LUA->CheckString(1);

	bool bSuccess = false;
	if (pLuaGameEventListener)
	{
		CGameEventDescriptor* desciptor = pManager->GetEventDescriptor(strEvent);
		if (!desciptor)
		{
			LUA->PushBool(false);
			return 1;
		}

		FOR_EACH_VEC(desciptor->listeners, i)
		{
			CGameEventCallback* callback = desciptor->listeners[i];
			if (callback->m_nListenerType != CGameEventManager::SERVERSIDE)
				continue;

			IGameEventListener2* listener = (IGameEventListener2*)callback->m_pCallback;

			if (listener == pLuaGameEventListener)
			{
				desciptor->listeners.Remove(i);
				bSuccess = true;
				break;
			}
		}
	} else {
		Warning("holylib: Failed to find LuaGameEventListener in GameSystems?\n");
	}

	LUA->PushBool(bSuccess);

	return 1;
}

void CHolyTestModule::Init(CreateInterfaceFn* appfn, CreateInterfaceFn* gamefn)
{
	pManager = (CGameEventManager*)appfn[0](INTERFACEVERSION_GAMEEVENTSMANAGER2, NULL);
	Detour::CheckValue("get interface", "CGameEventManager", pManager != NULL);
}

void CHolyTestModule::LuaInit(bool bServerInit)
{
	if (bServerInit)
		return;

	Util::StartTable();
		Util::AddFunc(ServerExecute, "ServerExecute");
		Util::AddFunc(UnregisterConVar, "UnregisterConVar");
		Util::AddFunc(GetEventListeners, "GetEventListeners");
		Util::AddFunc(RemoveEventListener, "RemoveEventListener");
	Util::FinishTable("holytest");

	g_Lua->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		g_Lua->GetField(-1, "Listen");
			g_Lua->PushString("vote_cast");
			g_Lua->CallFunctionProtected(1, 0, true);
			CGameEventDescriptor* descriptor = pManager->GetEventDescriptor("vote_cast");
			FOR_EACH_VEC(descriptor->listeners, i)
			{
				pLuaGameEventListener = (IGameEventListener2*)descriptor->listeners[i]->m_pCallback;
				descriptor->listeners.Remove(i);
				break;
			}
			if (!pLuaGameEventListener)
				Warning("holytest: Failed to find pLuaGameEventListener!\n");
	g_Lua->Pop(1);
}

void CHolyTestModule::LuaShutdown()
{
	Util::NukeTable("holytest");
}