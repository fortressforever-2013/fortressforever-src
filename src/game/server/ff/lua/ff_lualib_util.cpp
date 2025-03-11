
// ff_lualib_util.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_player.h"
#include "ff_projectile_base.h"
#include "ff_info_script.h"
#include "ff_triggerclip.h"
#include "ff_utils.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "LuaBridge/LuaBridge.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------

enum CollectionFilter
{
	CF_NONE = 0,

	CF_PLAYERS,
	CF_HUMAN_PLAYERS,
	CF_BOT_PLAYERS,
	CF_PLAYER_SCOUT,
	CF_PLAYER_SNIPER,
	CF_PLAYER_SOLDIER,
	CF_PLAYER_DEMOMAN,
	CF_PLAYER_MEDIC,
	CF_PLAYER_HWGUY,
	CF_PLAYER_PYRO,
	CF_PLAYER_SPY,
	CF_PLAYER_ENGY,
	CF_PLAYER_CIVILIAN,

	CF_TEAMS,
	CF_TEAM_SPECTATOR,
	CF_TEAM_BLUE,
	CF_TEAM_RED,
	CF_TEAM_YELLOW,
	CF_TEAM_GREEN,

	CF_PROJECTILES,
	CF_GRENADES,
	CF_INFOSCRIPTS,

	CF_INFOSCRIPT_CARRIED,
	CF_INFOSCRIPT_DROPPED,
	CF_INFOSCRIPT_RETURNED,
	CF_INFOSCRIPT_ACTIVE,
	CF_INFOSCRIPT_INACTIVE,
	CF_INFOSCRIPT_REMOVED,

	CF_BUILDABLES,
	CF_BUILDABLE_DISPENSER,
	CF_BUILDABLE_SENTRYGUN,
	CF_BUILDABLE_DETPACK,
	CF_BUILDABLE_JUMPPAD,

	CF_TRACE_BLOCK_WALLS,

	CF_MAX_FLAG
};

using namespace luabridge;

//---------------------------------------------------------------------------
void CFFLuaLib::InitUtil(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)
		.beginNamespace("CF")
			.addProperty("kNone",					[]() -> int { return CF_NONE; })

			.addProperty("kPlayers",				[]() -> int { return CF_PLAYERS; })
			.addProperty("kHumanPlayers",			[]() -> int { return CF_HUMAN_PLAYERS; })
			.addProperty("kBotPlayers",				[]() -> int { return CF_BOT_PLAYERS; })
			.addProperty("kPlayerScout",			[]() -> int { return CF_PLAYER_SCOUT; })
			.addProperty("kPlayerSniper",			[]() -> int { return CF_PLAYER_SNIPER; })
			.addProperty("kPlayerSoldier",			[]() -> int { return CF_PLAYER_SOLDIER; })
			.addProperty("kPlayerDemoman",			[]() -> int { return CF_PLAYER_DEMOMAN; })
			.addProperty("kPlayerMedic",			[]() -> int { return CF_PLAYER_DEMOMAN; })
			.addProperty("kPlayerHWGuy",			[]() -> int { return CF_PLAYER_HWGUY; })
			.addProperty("kPlayerPyro",				[]() -> int { return CF_PLAYER_PYRO; })
			.addProperty("kPlayerSpy",				[]() -> int { return CF_PLAYER_SPY; })
			.addProperty("kPlayerEngineer",			[]() -> int { return CF_PLAYER_ENGY; })
			.addProperty("kPlayerCivilian",			[]() -> int { return CF_PLAYER_CIVILIAN; })

			.addProperty("kTeams",					[]() -> int { return CF_TEAMS; })
			.addProperty("kTeamSpec",				[]() -> int { return CF_TEAM_SPECTATOR; })
			.addProperty("kTeamBlue",				[]() -> int { return CF_TEAM_BLUE; })
			.addProperty("kTeamRed",				[]() -> int { return CF_TEAM_RED; })
			.addProperty("kTeamYellow",				[]() -> int { return CF_TEAM_YELLOW; })
			.addProperty("kTeamGreen",				[]() -> int { return CF_TEAM_GREEN; })

			.addProperty("kProjectiles",			[]() -> int { return CF_PROJECTILES; })
			.addProperty("kGrenades",				[]() -> int { return CF_GRENADES; })
			.addProperty("kInfoScipts",				[]() -> int { return CF_INFOSCRIPTS; }) // typo; kept for backwards compatibility
			.addProperty("kInfoScripts",			[]() -> int { return CF_INFOSCRIPTS; })

			.addProperty("kInfoScript_Carried",		[]() -> int { return CF_INFOSCRIPT_CARRIED; })
			.addProperty("kInfoScript_Dropped",		[]() -> int { return CF_INFOSCRIPT_DROPPED; })
			.addProperty("kInfoScript_Returned",	[]() -> int { return CF_INFOSCRIPT_RETURNED; })
			.addProperty("kInfoScript_Active",		[]() -> int { return CF_INFOSCRIPT_ACTIVE; })
			.addProperty("kInfoScript_Inactive",	[]() -> int { return CF_INFOSCRIPT_INACTIVE; })
			.addProperty("kInfoScript_Removed",		[]() -> int { return CF_INFOSCRIPT_REMOVED; })

			.addProperty("kTraceBlockWalls",		[]() -> int { return CF_TRACE_BLOCK_WALLS; })

			.addProperty("kBuildables",				[]() -> int { return CF_BUILDABLES; })
			.addProperty("kDispenser",				[]() -> int { return CF_BUILDABLE_DISPENSER; })
			.addProperty("kSentrygun",				[]() -> int { return CF_BUILDABLE_SENTRYGUN; })
			.addProperty("kDetpack",				[]() -> int { return CF_BUILDABLE_DETPACK; })
			.addProperty("kJumpPad",				[]() -> int { return CF_BUILDABLE_JUMPPAD; })
		.endNamespace();
};
