#pragma once

#include <sourcesdk/ILuaInterface.h>
#include "eiface.h"

class IVEngineServer;

// Added to not break some sourcesdk things. Use Util::engineserver!
extern IVEngineServer* engine;

extern GarrysMod::Lua::IUpdatedLuaInterface* g_Lua;

class CBasePlayer;
class CBaseClient;
class CGlobalEntityList;
class CUserMessages;
namespace Util
{
	extern void StartTable();
	extern void AddValue(int, const char*);
	extern void AddFunc(GarrysMod::Lua::CFunc, const char*);
	extern void FinishTable(const char*);
	extern void NukeTable(const char*);
	extern bool PushTable(const char*);
	extern void PopTable();
	extern void RemoveField(const char*);

	// Gmod's functions:
	extern CBasePlayer* Get_Player(int iStackPos, bool unknown);
	extern CBaseEntity* Get_Entity(int iStackPos, bool unknown);
	extern void Push_Entity(CBaseEntity* pEnt);

	extern CBaseClient* GetClientByUserID(int userid);

	extern void AddDetour(); // We load Gmod's functions in there.
	
	extern IVEngineServer* engineserver;
	extern IServerGameClients* servergameclients;
	extern IServerGameEnts* servergameents;
	extern IServer* server;
	extern CGlobalEntityList* entitylist;

	inline CBaseEntity* GetCBaseEntityFromEdict(edict_t* edict)
	{
		return servergameents->EdictToBaseEntity(edict);
	}

	inline edict_t* GetEdictOfEnt(CBaseEntity* entity)
	{
		return servergameents->BaseEntityToEdict(entity);
	}

	extern CBaseClient* GetClientByPlayer(CBasePlayer* ply);
	extern CBaseClient* GetClientByIndex(int index);
	extern std::vector<CBaseClient*> GetClients();
	extern CBasePlayer* GetPlayerByClient(CBaseClient* client);

	extern bool ShouldLoad();
}

Vector* Get_Vector(int iStackPos, bool bError = true);
QAngle* Get_Angle(int iStackPos, bool bError = true);

class IRecipientFilter;
extern IRecipientFilter* Get_IRecipientFilter(int iStackPos, bool bError);

class IConVar;
extern IConVar* Get_IConVar(int iStackPos, bool bError);