#include "util.h"
#include "GarrysMod/Lua/LuaObject.h"
#include <string>
#include "GarrysMod/InterfacePointers.hpp"
#include "sourcesdk/baseclient.h"
#include "iserver.h"
#include "module.h"
#include "icommandline.h"
#include "player.h"
#include "detours.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Try not to use it. We want to move away from it.
// Additionaly, we will add checks in many functions.
GarrysMod::Lua::ILuaInterface* g_Lua;

IVEngineServer* engine;

bool g_pRemoveLuaUserData = true;
std::unordered_set<LuaUserData*> g_pLuaUserData;
std::unordered_map<void*, BaseUserData*> g_pGlobalLuaUserData;

std::unordered_set<int> Util::g_pReference;
ConVar Util::holylib_debug_mainutil("holylib_debug_mainutil", "1");

/*
 * NOTE: The Set/Get player & entity functions use a ILuaObject to ensure it mimics gmod's behavior instead of doing the funny holylib stuff.
 * It might be slower but we want to ensure the game won't possibly break because of our funnies.
 */
CBasePlayer* Util::Get_Player(GarrysMod::Lua::ILuaInterface* LUA, int iStackPos, bool bError) // bError = error if not a valid player
{
	EHANDLE* pEntHandle = LUA->GetUserType<EHANDLE>(iStackPos, GarrysMod::Lua::Type::Entity);
	if (!pEntHandle)
	{
		if (bError)
			LUA->ThrowError("Tried to use a NULL Entity!");

		return NULL;
	}

	GarrysMod::Lua::ILuaObject* pObj = LUA->NewTemporaryObject();
	pObj->SetFromStack(iStackPos);

	return (CBasePlayer*)pObj->GetEntity();
}

void Util::Push_Entity(GarrysMod::Lua::ILuaInterface* LUA, CBaseEntity* pEnt)
{
	if (!pEnt)
	{
		LUA->GetField(LUA_GLOBALSINDEX, "NULL");
		return;
	}

	GarrysMod::Lua::ILuaObject* pObj = LUA->NewTemporaryObject();
	pObj->SetEntity(pEnt);
	pObj->Push();
}

CBaseEntity* Util::Get_Entity(GarrysMod::Lua::ILuaInterface* LUA, int iStackPos, bool bError)
{
	EHANDLE* pEntHandle = LUA->GetUserType<EHANDLE>(iStackPos, GarrysMod::Lua::Type::Entity);
	if (!pEntHandle && bError)
		LUA->ThrowError("Tried to use a NULL Entity!");

	GarrysMod::Lua::ILuaObject* pObj = LUA->NewTemporaryObject();
	pObj->SetFromStack(iStackPos);
	CBaseEntity* pEntity = pObj->GetEntity();
	if (!pEntity && bError)
		LUA->ThrowError("Tried to use a NULL Entity! (The weird case?)");
		
	return pEntity;
}

IServer* Util::server;
CBaseClient* Util::GetClientByUserID(int userid)
{
	for (int i = 0; i < Util::server->GetClientCount(); i++)
	{
		IClient* pClient = Util::server->GetClient(i);
		if ( pClient && pClient->GetUserID() == userid)
			return (CBaseClient*)pClient;
	}

	return NULL;
}

IVEngineServer* Util::engineserver = NULL;
IServerGameEnts* Util::servergameents = NULL;
IServerGameClients* Util::servergameclients = NULL;
CBaseClient* Util::GetClientByPlayer(const CBasePlayer* ply)
{
	return Util::GetClientByUserID(Util::engineserver->GetPlayerUserId(((CBaseEntity*)ply)->edict()));
}

CBaseClient* Util::GetClientByIndex(int index)
{
	if (server->GetClientCount() <= index || index < 0)
		return NULL;

	return (CBaseClient*)server->GetClient(index);
}

std::vector<CBaseClient*> Util::GetClients()
{
	std::vector<CBaseClient*> pClients;

	for (int i = 0; i < server->GetClientCount(); i++)
	{
		IClient* pClient = server->GetClient(i);
		pClients.push_back((CBaseClient*)pClient);
	}

	return pClients;
}

CBasePlayer* Util::GetPlayerByClient(CBaseClient* client)
{
	return (CBasePlayer*)servergameents->EdictToBaseEntity(engineserver->PEntityOfEntIndex(client->GetPlayerSlot() + 1));
}

CBaseEntity* Util::GetCBaseEntityFromEdict(edict_t* edict)
{
	return Util::servergameents->EdictToBaseEntity(edict);
}

CBaseEntityList* g_pEntityList = NULL;
IServerGameDLL* Util::servergamedll;
void Util::AddDetour()
{
	if (g_pModuleManager.GetAppFactory())
		engineserver = (IVEngineServer*)g_pModuleManager.GetAppFactory()(INTERFACEVERSION_VENGINESERVER, NULL);
	else
		engineserver = InterfacePointers::VEngineServer();
	Detour::CheckValue("get interface", "IVEngineServer", engineserver != NULL);

	SourceSDK::FactoryLoader server_loader("server");
	if (g_pModuleManager.GetAppFactory())
		servergameents = (IServerGameEnts*)g_pModuleManager.GetGameFactory()(INTERFACEVERSION_SERVERGAMEENTS, NULL);
	else
		servergameents = server_loader.GetInterface<IServerGameEnts>(INTERFACEVERSION_SERVERGAMEENTS);
	Detour::CheckValue("get interface", "IServerGameEnts", servergameents != NULL);

	if (g_pModuleManager.GetAppFactory())
		servergameclients = (IServerGameClients*)g_pModuleManager.GetGameFactory()(INTERFACEVERSION_SERVERGAMECLIENTS, NULL);
	else
		servergameclients = server_loader.GetInterface<IServerGameClients>(INTERFACEVERSION_SERVERGAMECLIENTS);
	Detour::CheckValue("get interface", "IServerGameClients", servergameclients != NULL);

	if (g_pModuleManager.GetAppFactory())
		servergamedll = (IServerGameDLL*)g_pModuleManager.GetGameFactory()(INTERFACEVERSION_SERVERGAMEDLL, NULL);
	else
		servergamedll = server_loader.GetInterface<IServerGameDLL>(INTERFACEVERSION_SERVERGAMEDLL);
	Detour::CheckValue("get interface", "IServerGameDLL", servergamedll != NULL);

	server = InterfacePointers::Server();
	Detour::CheckValue("get class", "IServer", server != NULL);
}

void Util::RemoveDetour()
{
}

// If HolyLib was already loaded, we won't load a second time.
// How could this happen?
// In cases some other module utilizes HolyLib/compiled it to a .lib and uses the lib file they can load/execute HolyLib themself.
// & yes, you can compile HolyLib to a .lib file & load it using the 
static bool g_pShouldLoad = false;
bool Util::ShouldLoad()
{
	if (CommandLine()->FindParm("-holylibexists") && !g_pShouldLoad) // Don't set this manually!
		return false;

	if (g_pShouldLoad)
		return true;

	g_pShouldLoad = true;
	CommandLine()->AppendParm("-holylibexists", "true");

	return true;
}

GMODGet_LuaClass(IRecipientFilter, GarrysMod::Lua::Type::RecipientFilter, "RecipientFilter")
GMODGet_LuaClass(Vector, GarrysMod::Lua::Type::Vector, "Vector")
GMODGet_LuaClass(QAngle, GarrysMod::Lua::Type::Angle, "Angle")
GMODGet_LuaClass(ConVar, GarrysMod::Lua::Type::ConVar, "ConVar")