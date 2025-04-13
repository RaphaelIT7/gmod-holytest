# HolyTest

Contains functions used for testing gmod.  

# Modules

## holytest

### Functions

#### holytest.ServerExecute()
Forces all queried commands to execute.  

#### holytest.UnregisterConVar(ConVar cvar)
Unregisters the given convar.  

#### (int or table) holytest.GetEventListeners(string name)
string name(optional) - The event to return the count of listeners for.  
If name is not a string, it will return a table containing all events and their listener count:  
```lua
{
	["player_spawn"] = 1
}
```

#### bool holytest.RemoveEventListener(string name)
string name - The event to remove the Lua gameevent listener from.  

### Entire HttpServer module
The entire HttpServer module from Http was implemented.  
All functions can be seen here when searching for `Http`: https://holylib.raphaelit7.com/