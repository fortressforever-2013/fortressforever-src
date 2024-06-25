
// ff_lualib_team.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_team.h"
#include "ff_scriptman.h"

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
using namespace luabridge;

/// tostring implemenation for CTeam
std::string TeamToString(const CTeam& team)
{
	std::stringstream stream;
	stream << "team" << ":" << const_cast<CTeam&>(team).GetName() << ":" << const_cast<CTeam&>(team).GetTeamNumber();
	return stream.str();
}

namespace FFLib
{
	// helper function to turn the bitmask into a lua table of teams
	LuaRef GetAllies(CFFTeam *pTeam)
	{
		LuaRef luatblAllies = newTable(_scriptman.GetLuaState());

		int iTableKey = 1;
		int alliesMask = pTeam->GetAllies();
		for (int teamId = TEAM_UNASSIGNED; teamId < TEAM_COUNT; teamId++)
		{
			if (!(alliesMask & (1<<teamId)))
				continue;

			CFFTeam *pAlliedTeam = dynamic_cast<CFFTeam*>(g_Teams[teamId]);
			luatblAllies[iTableKey++] = LuaRef(_scriptman.GetLuaState(), pAlliedTeam);
		}

		return luatblAllies;
	}
};

//---------------------------------------------------------------------------
void CFFLuaLib::InitTeam(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)
		.beginClass<CTeam>("BaseTeam")
			.addFunction("__tostring",		&TeamToString)

			.addFunction("AddScore",		&CTeam::AddScore)
			.addFunction("GetScore",		&CTeam::GetScore)
			.addFunction("SetScore",		&CTeam::SetScore)
			.addFunction("GetScoreTime",	&CTeam::GetScoreTime)
			.addFunction("AddFortPoints",	&CTeam::AddFortPoints)
			.addFunction("SetFortPoints",	&CTeam::SetFortPoints)
			.addFunction("GetFortPoints",	&CTeam::GetFortPoints)
			.addFunction("AddDeaths",		&CTeam::AddDeaths)
			.addFunction("GetDeaths",		&CTeam::GetDeaths)
			.addFunction("SetDeaths",		&CTeam::SetDeaths)
			.addFunction("GetNumPlayers",	&CTeam::GetNumPlayers)
			.addFunction("GetPlayer",		&CTeam::GetPlayer)
			.addFunction("GetTeamId",		&CTeam::GetTeamNumber)
			.addFunction("GetName",			&CTeam::GetName)
			.addFunction("SetName",			&CTeam::SetName)
		.endClass()

		// CFFTeam
		.deriveClass<CFFTeam, CTeam>("FF_Team")
			.addFunction("SetAllies",			&CFFTeam::SetEasyAllies)
			.addFunction("ClearAllies",			&CFFTeam::ClearAllies)
			.addFunction("GetAllies",			&FFLib::GetAllies)
			.addFunction("SetClassLimit",		&CFFTeam::SetClassLimit)
			.addFunction("GetClassLimit",		&CFFTeam::GetClassLimit)
			.addFunction("SetPlayerLimit",		&CFFTeam::SetTeamLimits)
			.addFunction("GetPlayerLimit",		&CFFTeam::GetTeamLimits)
			.addFunction("IsFFA",				&CFFTeam::IsFFA)
			.addFunction("SetFFA",				&CFFTeam::SetFFA)
			.addFunction("GetTeamIcon",			&CFFTeam::GetTeamIcon)
			.addFunction("SetTeamIcon",			&CFFTeam::SetTeamIcon)
			.addFunction("ResetTeamIcon",		&CFFTeam::ResetTeamIcon)
		.endClass()

		.beginNamespace("Team")
			.addProperty("kUnassigned",		[]() -> int { return TEAM_UNASSIGNED; })
			.addProperty("kSpectator",		[]() -> int { return TEAM_SPECTATOR; })
			.addProperty("kBlue",			[]() -> int { return TEAM_BLUE; })
			.addProperty("kRed",			[]() -> int { return TEAM_RED; })
			.addProperty("kYellow",			[]() -> int { return TEAM_YELLOW; })
			.addProperty("kGreen",			[]() -> int { return TEAM_GREEN; })
		.endNamespace();
};
