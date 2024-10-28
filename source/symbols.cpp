#include "detours.h"

static Symbol NULL_SIGNATURE = Symbol::FromSignature("");

namespace Symbols
{
	//---------------------------------------------------------------------------------
	// Purpose: All Base Symbols
	//---------------------------------------------------------------------------------
	const std::vector<Symbol> InitLuaClassesSym = {
		Symbol::FromName("_Z14InitLuaClassesP13ILuaInterface"),
		Symbol::FromSignature("\x48\x8B\x05****\x48\x85\xC0**\x8B\x50\x10\x85\xD2**\x55\x48\x89\xE5\x53\x31\xDB******\x48\x8B*\x48\x8B\x3C\xD8\xE8\xD4"), // 48 8B 05 ?? ?? ?? ?? 48 85 C0 ?? ?? 8B 50 10 85 D2 ?? ?? 55 48 89 E5 53 31 DB ?? ?? ?? ?? ?? ?? 48 8B ?? 48 8B 3C D8 E8 D4
	};

	const std::vector<Symbol> CLuaInterface_ShutdownSym = {
		Symbol::FromName("_ZN13CLuaInterface8ShutdownEv"),
		Symbol::FromSignature("\x55\x48\x89\xE5\x41\x55\x41\x54\x49\x89\xFC\x53\x4D\x8D"), // 55 48 89 E5 41 55 41 54 49 89 FC 53 4D 8D
	};

	const std::vector<Symbol> Get_PlayerSym = {
		Symbol::FromName("_Z10Get_Playerib"),
		Symbol::FromSignature("\x55\x48\x89\xE5\x41\x54\x41\x89\xF4\x53\x40\x0F\xB6\xF6\xE8\xED\xF3"), // 55 48 89 E5 41 54 41 89 F4 53 40 0F B6 F6 E8 ED F3
	};

	const std::vector<Symbol> Push_EntitySym = {
		Symbol::FromName("_Z11Push_EntityP11CBaseEntity"),
		Symbol::FromSignature("\x55\x48\x89\xE5\x41\x54\x53\x48\x83\xEC\x20\x48\x8B\x1D"), // 55 48 89 E5 41 54 53 48 83 EC 20 48 8B 1D
	};

	const std::vector<Symbol> Get_EntitySym = {
		Symbol::FromName("_Z10Get_Entityib"),
		Symbol::FromSignature("\x55\x48\x89\xE5\x41\x55\x41\x89\xF5\x41\x54\x41\x89\xFC\x53"), // 55 48 89 E5 41 55 41 89 F5 41 54 41 89 FC 53
	};

	const std::vector<Symbol> g_pEntityListSym = { // 64x = ents.GetAll
		Symbol::FromName("g_pEntityList"),
	};

	const std::vector<Symbol> CBaseEntity_CalcAbsolutePositionSym = { // Can't find a 64x symbol yet....
		Symbol::FromName("_ZN11CBaseEntity20CalcAbsolutePositionEv"),
	};
}