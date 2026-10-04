//==================================//
//
// Purpose: FF's C_PlayerResource
//
// $NoKeywords: $
//==================================//

#ifndef C_FF_PLAYERRESOURCE_H
#define C_FF_PLAYERRESOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_shareddefs.h"
#include "c_playerresource.h"

class C_FF_PlayerResource : public C_PlayerResource
{
	DECLARE_CLASS( C_FF_PlayerResource, C_PlayerResource );
public:
	DECLARE_CLIENTCLASS();

	C_FF_PlayerResource();
	virtual ~C_FF_PlayerResource();

	// Team data
	float		GetTeamScoreTime(int index);
	int		GetTeamFortPoints(int index);
	int		GetTeamDeaths(int index);

	// Player data
	int		GetFortPoints(int index);
	int		GetArmor(int index);
	int		GetAssists(int index);

	// --> Mirv: Extra's needed for menus
	int		GetClass(int index);
	int		GetChannel(int index);
	int		GetTeamClassLimits(int index, int classindex);
	int		GetTeamLimits(int index);
	// <-- Mirv: Extra's needed for menus

	bool		m_bIsIntermission = false;

protected:
	int		m_iFortPoints[MAX_PLAYERS + 1];	//BreakinBenny: This will be MAX_PLAYERS_ARRAY_SAFE in TF2 SDK
	int		m_iArmor[MAX_PLAYERS + 1];
	int		m_iClass[MAX_PLAYERS + 1];	// |-- Mirv: Current class
	int		m_iChannel[MAX_PLAYERS + 1];	// |-- Mirv: For voice channels
	int		m_iAssists[MAX_PLAYERS + 1];
};

extern C_FF_PlayerResource *g_FF_PR;

#endif // C_FF_PLAYERRESOURCE_H