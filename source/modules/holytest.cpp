#include "module.h"
#include <GarrysMod/Lua/Interface.h>
#include "lua.h"

class CHolyTestModule : public IModule
{
public:
	virtual void LuaInit(bool bServerInit);
	virtual void LuaShutdown();
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

void CHolyTestModule::LuaInit(bool bServerInit)
{
	if (bServerInit)
		return;

	Util::StartTable();
		Util::AddFunc(ServerExecute, "ServerExecute");
		Util::AddFunc(UnregisterConVar, "UnregisterConVar");
	Util::FinishTable("holytest");
}

void CHolyTestModule::LuaShutdown()
{
	Util::NukeTable("holytest");
}