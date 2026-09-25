//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side team manager class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_FF_TEAM_H
#define C_FF_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "ff_shareddefs.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//-----------------------------------------------------------------------------
// Purpose: FF's Team manager
//-----------------------------------------------------------------------------
class C_FFTeam : public C_Team
{
	DECLARE_CLASS( C_FFTeam, C_Team );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

public:

					C_FFTeam();
	virtual			~C_FFTeam();

	int GetAlliedTeams( int (&iAlliedTeams)[TEAM_COUNT] );

	// --> Mirv: Menus need to know limits
	virtual int		Get_Classes( int );
	int			Get_FortPoints( void );
	float		Get_ScoreTime( void );
	virtual int		Get_Teams( void );
	virtual int		GetAllies( void );

	float	m_flScoreTime; // Mulch: time this team last scored
	int		m_iFortPoints;

	bool IsFFA() { return m_bFFA; };
	void SetFFA( bool bFFA ) { m_bFFA = bFFA; };

	// LUA custom team icons
	char* GetTeamIcon( void );
	char m_szTeamIcon[128];

private:

	int		m_iClasses[12];
	int		m_iMaxPlayers;
	int		m_iAllies;
	bool	m_bFFA;
	// <-- Mirv: Menus need to know limits

};

// Global team handling functions
C_FFTeam *GetGlobalFFTeam( int iTeamNumber );


#endif // C_FF_TEAM_H
