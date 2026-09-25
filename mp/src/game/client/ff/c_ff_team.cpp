//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_FFTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_ff_team.h"

#include <vgui/VGUI.h>
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_FFTeam, DT_FFTeam, CFFTeam)
	// --> Mirv: Some limits that the client needs to know about for the menu
	RecvPropInt(RECVINFO(m_iFortPoints)),
	// Bug #0000529: Total death column doesn't work
	RecvPropInt(RECVINFO(m_iDeaths)),	// Mulch: receive team deaths from server
	RecvPropFloat(RECVINFO(m_flScoreTime)), // Mulch: time this team last scored
	RecvPropInt( RECVINFO( m_iAllies ) ),
	RecvPropInt( RECVINFO( m_iMaxPlayers ) ),
	RecvPropBool( RECVINFO( m_bFFA ) ),
	RecvPropArray3( RECVINFO_ARRAY(m_iClasses), RecvPropInt( RECVINFO(m_iClasses[0]))),
	// <-- Mirv: Some limits that the client needs to know about for the menu

	RecvPropString( RECVINFO( m_szTeamIcon ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_FFTeam )
	DEFINE_PRED_FIELD( m_iFortPoints, FIELD_INTEGER, FTYPEDESC_PRIVATE ),
END_PREDICTION_DATA();

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FFTeam::C_FFTeam() : C_Team()
{
	memset( &m_iClasses, 0, sizeof(m_iClasses) );	// |-- Mirv: Classes
	memset( m_szTeamIcon, 0, sizeof( m_szTeamIcon ) );

	m_iFortPoints = 0;
	m_flScoreTime = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FFTeam::~C_FFTeam()
{
}

// --> Mirv: Menues need to know limits
int C_FFTeam::Get_Classes( int classnum )
{
	return m_iClasses[classnum];
}

int C_FFTeam::Get_Teams( void )
{
	return m_iMaxPlayers;
}

int C_FFTeam::GetAllies( void )
{
	return m_iAllies;
}
// <-- Mirv: Menues need to know limits

// --> hlstriker: Sets array of allies by ref, and returns the number of allies
int C_FFTeam::GetAlliedTeams( int (&iAlliedTeams)[TEAM_COUNT] )
{
	int iCount = 0;
	for (int i = FF_TEAM_BLUE; i < TEAM_COUNT; i++ )
	{
		if ( m_iTeamNum == i )
			continue;

		if ( GetAllies() & ( 1 << i ) )
		{
			iAlliedTeams[iCount++] = i;
			if ( iCount >= TEAM_COUNT )
				break;
		}
	}
	return iCount;
}
// <--
char* C_FFTeam::GetTeamIcon( void )
{
	return m_szTeamIcon;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float C_FFTeam::Get_ScoreTime(void)
{
	return m_flScoreTime;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int C_FFTeam::Get_FortPoints(void)
{
	return m_iFortPoints;
}

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
C_FFTeam *GetGlobalFFTeam( int iTeamNumber )
{
	return (C_FFTeam *)GetGlobalTeam( iTeamNumber );
}
