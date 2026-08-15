//==================================//
//
// Purpose: FF's C_PlayerResource
//
// $NoKeywords: $
//==================================//
#include "cbase.h"
#include "c_ff_playerresource.h"
#include "c_team.h"
#include "c_ff_team.h"
#include <shareddefs.h>
#include <ff_shareddefs.h>
#include "ff_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_FF_PlayerResource *g_FF_PR;

IMPLEMENT_CLIENTCLASS_DT( C_FF_PlayerResource, DT_FFPlayerResource, CFFPlayerResource )
	RecvPropArray3( RECVINFO_ARRAY(m_iFortPoints), RecvPropInt( RECVINFO(m_iFortPoints[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iArmor), RecvPropInt(RECVINFO(m_iArmor[0]))),
	RecvPropArray3( RECVINFO_ARRAY(m_iClass), RecvPropInt(RECVINFO(m_iClass[0]))), // |-- Mirv: Current class
	RecvPropArray3( RECVINFO_ARRAY(m_iChannel), RecvPropInt(RECVINFO(m_iChannel[0]))), // |-- Mirv: Channel information
	RecvPropArray3( RECVINFO_ARRAY(m_iAssists), RecvPropInt(RECVINFO(m_iAssists[0]))),
	RecvPropBool( RECVINFO(m_bIsIntermission)),
END_RECV_TABLE()

extern ConVar hud_newteamcolors;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FF_PlayerResource::C_FF_PlayerResource()
{
	memset( m_iFortPoints, 0, sizeof(m_iFortPoints) );
	memset( m_iArmor, 0, sizeof(m_iArmor) );
	memset( m_iClass, 0, sizeof(m_iClass) );	// |-- Mirv: Current class

	memset( m_iChannel, 0, sizeof(m_iChannel) ); // |-- Mirv: Channel information

	memset( m_iAssists, 0, sizeof(m_iAssists) );
	
	m_Colors[TEAM_SPECTATOR] = TEAM_COLOR_SPECTATOR;
	m_Colors[FF_TEAM_BLUE] = TEAM_COLOR_BLUE;
	m_Colors[FF_TEAM_RED] = TEAM_COLOR_RED;
	m_Colors[FF_TEAM_YELLOW] = TEAM_COLOR_YELLOW;
	m_Colors[FF_TEAM_GREEN] = TEAM_COLOR_GREEN;

	g_FF_PR = this;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FF_PlayerResource::~C_FF_PlayerResource()
{
	g_FF_PR = NULL;
}


int C_FF_PlayerResource::GetTeamFortPoints(int index)
{
	C_FFTeam *team = GetGlobalTeam(index);

	if (!team)
		return 0;

	return team->Get_FortPoints();
}

float C_FF_PlayerResource::GetTeamScoreTime(int index)
{
	C_FFTeam *pTeam = GetGlobalTeam(index);

	if (!pTeam)
		return 0.0f;

	return pTeam->Get_ScoreTime();
}

int C_FF_PlayerResource::GetTeamDeaths(int index)
{
	C_FFTeam *team = GetGlobalTeam(index);

	if (!team)
		return 0;

	return team->Get_Deaths();
}

// --> Mirv: So menus can show correct limits
int C_FF_PlayerResource::GetTeamClassLimits(int index, int classindex)
{
	C_FFTeam *team = (C_FFTeam*)GetGlobalTeam(index);

	if (!team)
		return 0;

	return team->Get_Classes(classindex);
}

int C_FF_PlayerResource::GetTeamLimits(int index)
{
	C_FFTeam *team = (C_FFTeam*)GetGlobalTeam(index);

	if (!team)
		return -1;

	return team->Get_Teams();
}
// <-- Mirv: So menus can show correct limits

int C_FF_PlayerResource::GetFrags(int index )
{
	//return 666;
	// BEG: Added by Mulchman
	if (!IsConnected(index))
		return 0;

	return m_iScore[index];
	// END: Added by Mulchman
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FF_PlayerResource::GetFortPoints(int iIndex)
{
	if (!IsConnected(iIndex))
		return 0;

	return m_iFortPoints[iIndex];
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FF_PlayerResource::GetArmor(int iIndex)
{
	if (!IsConnected(iIndex))
		return 0;

	return m_iArmor[iIndex];
}

// --> Mirv: Get the player's class
int C_FF_PlayerResource::GetClass(int iIndex)
{
	if (!IsConnected(iIndex))
		return 0;

	return m_iClass[iIndex];
}
// <-- Mirv: Get the player's class

// --> Mirv: Channel info
//-----------------------------------------------------------------------------
// Purpose: Return the voice channel that this player is using
//-----------------------------------------------------------------------------
int C_FF_PlayerResource::GetChannel(int iIndex)
{
	if (iIndex < 1 || iIndex > MAX_PLAYERS)
	{
		Assert(0);
		return 0;
	}
	else
		return m_iChannel[iIndex];
}
// <-- Mirv: Channel info

#ifdef CLIENT_DLL
bool Client_IsIntermission()
{
	C_FF_PlayerResource *pr = dynamic_cast <C_FF_PlayerResource*> (GameResources());
	if (!pr)
		return false;
	return pr->m_bIsIntermission;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FF_PlayerResource::GetAssists(int iIndex)
{
	if (!IsConnected(iIndex))
		return 0;

	return m_iAssists[iIndex];
}