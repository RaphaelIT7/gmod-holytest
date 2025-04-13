#include "plugin.h"
#include "lua.h"
#include <tier1.h>
#include <tier2/tier2.h>
#include <GarrysMod/Lua/LuaShared.h>
#include "module.h"
#include "detours.h"
#include <playerinfomanager.h>
#include <GarrysMod/InterfacePointers.hpp>

// The plugin is a static singleton that is exported as an interface
CServerPlugin g_HolyTestServerPlugin;
IServerPluginCallbacks* g_pHolyTestServerPlugin = &g_HolyTestServerPlugin;
#ifdef LIB_HOLYLIB
IServerPluginCallbacks* GetHolyLibPlugin()
{
	return g_pHolyTestServerPlugin;
}
#else
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServerPlugin, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_HolyTestServerPlugin);
#endif

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
CServerPlugin::CServerPlugin()
{
	m_iClientCommandIndex = 0;
}

CServerPlugin::~CServerPlugin()
{
}

#ifdef LIB_HOLYLIB
void HolyLib_PreLoad()
#else
DLL_EXPORT void HolyLib_PreLoad() // ToDo: Make this a CServerPlugin member later!
#endif
{
	if (!Util::ShouldLoad())
	{
		Msg("HolyLib already exists? Stopping.\n");
		return;
	}

#ifdef LIB_HOLYLIB
	g_pModuleManager.LoadModules();
#endif

	g_pModuleManager.SetGhostInj();
	g_pModuleManager.InitDetour(true);
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
CGlobalVars *gpGlobals = NULL;
static bool bIgnoreNextUnload = false;
bool CServerPlugin::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	Msg("--- HolyTest Plugin loading ---\n");

	if (!Util::ShouldLoad())
	{
		Msg("HolyTest already exists? Stopping.\n");
		Msg("--- HolyTest Plugin finished loading ---\n");
		bIgnoreNextUnload = true;
		return false; // What if we return false?
	}
	 
	if (interfaceFactory)
	{
		ConnectTier1Libraries(&interfaceFactory, 1);
		ConnectTier2Libraries(&interfaceFactory, 1);

		engine = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	} else {
		engine = InterfacePointers::VEngineServer();
		g_pFullFileSystem = InterfacePointers::FileSystemServer();
		g_pCVar = InterfacePointers::Cvar();
	}

	IPlayerInfoManager* playerinfomanager = NULL;
	if (gameServerFactory)
		playerinfomanager = (IPlayerInfoManager*)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER, NULL);
	else {
		SourceSDK::FactoryLoader server_loader("server");
		playerinfomanager = server_loader.GetInterface<IPlayerInfoManager>(INTERFACEVERSION_PLAYERINFOMANAGER);
	}
	Detour::CheckValue("get interface", "playerinfomanager", playerinfomanager != NULL);

	if ( playerinfomanager )
		gpGlobals = playerinfomanager->GetGlobalVars();

	g_pModuleManager.Setup(interfaceFactory, gameServerFactory); // Setup so that Util won't cause issues
	Lua::AddDetour();
	Util::AddDetour();
	g_pModuleManager.Init();
	g_pModuleManager.InitDetour(false);

	Msg("--- HolyLib Plugin finished loading ---\n");

	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void CServerPlugin::Unload(void)
{
	if (bIgnoreNextUnload)
	{
		bIgnoreNextUnload = false;
		return;
	}

	g_pModuleManager.Shutdown();
	Detour::Remove(0);
	Detour::ReportLeak();

	DisconnectTier1Libraries();
	DisconnectTier2Libraries();
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void CServerPlugin::Pause(void)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void CServerPlugin::UnPause(void)
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char* CServerPlugin::GetPluginDescription(void)
{
#if GITHUB_RUN_DATA == 0 // DATA should always fallback to 0. We will set it to 1 in releases.
	return "HolyTest Serverplugin V0.1 DEV (Workflow: " GITHUB_RUN_NUMBER ")";
#else
	return "HolyTest Serverplugin V0.1";
#endif
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CServerPlugin::LevelInit(char const *pMapName)
{

}

//---------------------------------------------------------------------------------
// Purpose: called after LUA Initialized.
//---------------------------------------------------------------------------------
bool CServerPlugin::LuaInit()
{
	GarrysMod::Lua::ILuaInterface* LUA = Lua::GetRealm(GarrysMod::Lua::State::SERVER);
	if (LUA == nullptr) {
		Msg("Failed to get ILuaInterface! (Realm: Server)\n");
		return false;
	}

	Lua::Init(LUA);

	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called just before LUA is shutdown.
//---------------------------------------------------------------------------------
void CServerPlugin::LuaShutdown()
{
	Lua::Shutdown();
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void CServerPlugin::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	if (!g_Lua) {
		LuaInit();
	}

	Lua::ServerInit();
	g_pModuleManager.ServerActivate(pEdictList, edictCount, clientMax);
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void CServerPlugin::GameFrame(bool simulating)
{
	g_pModuleManager.Think(simulating);
	g_pModuleManager.LuaThink(g_Lua);
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void CServerPlugin::LevelShutdown(void) // !!!!this can get called multiple times per map change
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void CServerPlugin::ClientActive(edict_t *pEntity)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void CServerPlugin::ClientDisconnect(edict_t *pEntity)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns?
//---------------------------------------------------------------------------------
void CServerPlugin::ClientPutInServer(edict_t *pEntity, char const *playername)
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CServerPlugin::SetCommandClient(int index)
{
	m_iClientCommandIndex = index;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void CServerPlugin::ClientSettingsChanged(edict_t *pEdict)
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT CServerPlugin::ClientConnect(bool* bAllowConnect, edict_t* pEntity, const char* pszName, const char* pszAddress, char* reject, int maxrejectlen)
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT CServerPlugin::ClientCommand(edict_t *pEntity, const CCommand &args)
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT CServerPlugin::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID)
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void CServerPlugin::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue)
{
}

void CServerPlugin::OnEdictAllocated(edict_t *edict)
{
}

void CServerPlugin::OnEdictFreed(const edict_t *edict)
{
}

GMOD_MODULE_OPEN()
{
	g_HolyTestServerPlugin.Load(NULL, NULL); // Yes. I don't like it but I can't get thoes fancy interfaces.

	return 0;
}

GMOD_MODULE_CLOSE()
{
	g_HolyTestServerPlugin.Unload();

	return 0;
}