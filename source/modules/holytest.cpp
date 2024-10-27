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

void CHolyTestModule::LuaInit(bool bServerInit)
{
	if (bServerInit)
		return;

	Util::StartTable();
		Util::AddFunc(ServerExecute, "ServerExecute");
	Util::FinishTable("holytest");
}

void CHolyTestModule::LuaShutdown()
{
	Util::NukeTable("holytest");
}