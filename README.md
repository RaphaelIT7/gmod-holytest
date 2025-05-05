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

#### holytest.ReceiveClientMessage(number userID, Entity ent, bf_read buffer, number bits)
`bits` - should always be the value of `bf_read:GetNumBits()`  
`ent` - The entity to use as the sender, should be a player.  

Allows you to fake client messages.  
Internally its a direct binding to `IServerGameClients::GMOD_ReceiveClientMessage`  

Example of faking a net message:  
```lua
net.Receive("Example", function(len, ply)
    print("Received example message: " .. tostring(ply) .. " (" .. len .. ")")
    print("Message contained: " .. net.ReadString())
end)

local bf = bitbuf.CreateWriteBuffer(64)
bf:WriteUBitLong(0, 8) -- The message type. 0 = Lua net message

bf:WriteUBitLong(util.AddNetworkString("Example"), 16) -- Header for net.ReadHeader
bf:WriteString("Hello World") -- Message content

local readBF = bitbuf.CreateReadBuffer(bf:GetData()) -- Make it a read buffer.
local entity = Entity(0) -- We can use the world but normally we shouldn't.
local userID = entity:IsPlayer() and entity:UserID() or -1
holytest.ReceiveClientMessage(userID, entity, readBF, readBF:GetNumBits())
```

### Entire HttpServer module
The entire HttpServer module from HolyLib was implemented.  
All functions can be seen here when searching for `Http`: https://holylib.raphaelit7.com/

### Entire bitbuf module
The entire bitbuf module from HolyLib was implemented.  
All functions can be seen here when searching for `bf_`: https://holylib.raphaelit7.com/