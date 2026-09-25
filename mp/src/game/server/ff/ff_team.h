//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_TEAM_H
#define FF_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "team.h"
#include "ff_shareddefs.h"

//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CFFTeam : public CTeam
{
	DECLARE_CLASS( CFFTeam, CTeam );
	DECLARE_SERVERCLASS();

public:

	// Initialization
	void	Init( const char *pName, int iNumber );
	void	SetName( const char *pszName );

	// --> Mirv: Team classes available and allies
	//int m_iAllies;
	CNetworkVar( int, m_iAllies ); // |-- Mulch: as per mirv 02/03/06

	CNetworkArray( int, m_iClasses, 12 );	// this is the actual limit, needed by the client
	int m_iClassesMap[12];					// this is just the map limits

	CNetworkVar( int, m_iMaxPlayers );
	CNetworkVar(int, m_iFortPoints);
	// Bug #0000529: Total death column doesn't work
	CNetworkVar(int, m_iDeaths);	// Mulch: send deaths to client
	CNetworkVar(float, m_flScoreTime); // Mulch: time when this team last scored

	CNetworkString( m_szTeamIcon, 128 );

private:
	CNetworkVar( bool, m_bFFA );
	

public:
	void SetClassLimit( int, int );	// Set the map's class limit
	int GetClassLimit( int );			// Get the class limit (inc. cr_)

	void SetTeamLimits( int );
	int GetTeamLimits( void );

	float	GetScoreTime(void);
	void	AddFortPoints(int iFortPoints);
	void	SetFortPoints(int iFortPoints);
	int	GetFortPoints(void);
	// Bug #0000529: Total death column doesn't work
	void	AddDeaths(int iScore);	// Mulch
	int	GetDeaths(void);	// Mulch
	void	SetDeaths(int iDeaths);
	
	void AddScore( int iScore );
	void SetScore( int iScore );
	
	void SetAllies( int );
	void SetEasyAllies( int );
	void ClearAllies();
	int GetAllies( void );

	void UpdateLimits( void );

	bool IsFFA() { return m_bFFA; };
	void SetFFA( bool bFFA ) { m_bFFA = bFFA; };
	// <-- Mirv: Team classes available and allies

	// LUA custom team icons
	void SetTeamIcon( const char* szIcon );
	void ResetTeamIcon( void );
	const char* GetTeamIcon( void );
};


extern CFFTeam *GetGlobalFFTeam( int iIndex );


#endif // TF_TEAM_H
