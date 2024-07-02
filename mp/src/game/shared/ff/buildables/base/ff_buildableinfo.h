// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableinfo.h
// @author Patrick O'Leary (Mulchman)
// @date ?/?/2005
// @brief BuildableInfo class
//
// ===============================================

#ifndef FF_BUILDABLEINFO_H
#define FF_BUILDABLEINFO_H

#ifdef CLIENT_DLL
	#define CFFPlayer C_FFPlayer
#endif
class CFFPlayer;

#include "ff_buildabledefs.h"

class CFFBuildableInfo
{
private:
	CFFBuildableInfo( void ) {}

public:
	CFFBuildableInfo( CFFPlayer *pPlayer, int iBuildObject );
	~CFFBuildableInfo( void ) {}

	// Returns why you can/can't build
	BuildInfoResult_t BuildResult( void ) const { return m_BuildResult; }

	// Get final build position
	Vector	GetBuildOrigin( void ) const { return m_vecBuildGroundOrigin; }
	// Get final build angles
	QAngle	GetBuildAngles( void ) const { return m_angBuildGroundAngles; }

	// Tells the player why they couldn't build - sets an error message
	// and displays it
	void	GetBuildError( void );

protected:
	// Type of object we're trying to build
	int		m_iBuildObject;

	// How far out in front of the player are we building
	float	m_flBuildDist;

	// Player's info
	CFFPlayer *m_pPlayer;
	// Just some quick accessors instead of having to calculate
	// these over and over...
	Vector	m_vecPlayerForward;
	Vector	m_vecPlayerRight;
	Vector	m_vecPlayerOrigin;	// this is CFFPlayer::GetAbsOrigin() (so the waist!)

	// This is our origin/angles we mess with while building/trying to build
	Vector	m_vecBuildAirOrigin;
	QAngle	m_angBuildAirAngles;

	// Final position/angle on the ground of the object (if it can be built, of course)
	Vector	m_vecBuildGroundOrigin;
	QAngle	m_angBuildGroundAngles;

	// Stores the build result
	BuildInfoResult_t	m_BuildResult;

protected:
	// Checks if geometry or other objects are in the way of building
	bool				IsGeometryInTheWay( void );
	//bool				IsGroundTooSteep( void );
	// See's if the ground is suitable for building
	BuildInfoResult_t	CanOrientToGround( void );

};

#endif