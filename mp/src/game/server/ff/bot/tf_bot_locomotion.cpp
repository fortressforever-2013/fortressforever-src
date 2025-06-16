//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_locomotion.cpp
// Team Fortress NextBot locomotion interface
// Michael Booth, May 2010

#include "cbase.h"

#include "ff_bot.h"
#include "ff_bot_locomotion.h"
#include "particle_parse.h"


//-----------------------------------------------------------------------------------------
void CFFBotLocomotion::Update( void )
{
	BaseClass::Update();

	CFFBot *me = ToTFBot( GetBot()->GetEntity() );
	if ( !me )
	{
		return;
	}

	// always 'crouch jump'
	if ( IsOnGround() )
	{
		if ( !me->IsPlayerClass( CLASS_ENGINEER ) )
		{
			// engineers need to crouch behind their guns
			me->ReleaseCrouchButton();
		}
	}
	else
	{
		me->PressCrouchButton( 0.3f );
	}
}


//-----------------------------------------------------------------------------------------
// Move directly towards the given position
void CFFBotLocomotion::Approach( const Vector &pos, float goalWeight )
{
       if ( !IsOnGround() && !IsClimbingOrJumping() )
       {
               // no air control
               return;
       }

	BaseClass::Approach( pos, goalWeight );
}


//-----------------------------------------------------------------------------------------
// Distance at which we will die if we fall
float CFFBotLocomotion::GetDeathDropHeight( void ) const
{
	return 1000.0f;
}


//-----------------------------------------------------------------------------------------
// Get maximum running speed
float CFFBotLocomotion::GetRunSpeed( void ) const
{
	CFFBot *me = (CFFBot *)GetBot()->GetEntity();
	return me->GetPlayerClass()->GetMaxSpeed();
}


//-----------------------------------------------------------------------------------------
// Return true if given area can be used for navigation
bool CFFBotLocomotion::IsAreaTraversable( const CNavArea *baseArea ) const
{
	CFFBot *me = (CFFBot *)GetBot()->GetEntity();
	CTFNavArea *area = (CTFNavArea *)baseArea;

	if ( area->IsBlocked( me->GetTeamNumber() ) )
	{
		return false;
	}

	if ( !FFGameRules()->RoundHasBeenWon() || FFGameRules()->GetWinningTeam() != me->GetTeamNumber() )
	{
		if ( area->HasAttributeTF( TF_NAV_SPAWN_ROOM_RED ) && me->GetTeamNumber() == FF_TEAM_BLUE )
		{
			return false;
		}

		if ( area->HasAttributeTF( TF_NAV_SPAWN_ROOM_BLUE ) && me->GetTeamNumber() == FF_TEAM_RED )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------
bool CFFBotLocomotion::IsEntityTraversable( CBaseEntity *obstacle, TraverseWhenType when ) const
{
	// assume all players are "traversable" in that they will move or can be killed
	if ( obstacle && obstacle->IsPlayer() )
	{
		return true;
	}

	return PlayerLocomotion::IsEntityTraversable( obstacle, when );
}


void CFFBotLocomotion::Jump( void )
{
	BaseClass::Jump();

	CFFBot *me = ToTFBot( GetBot()->GetEntity() );
	if ( !me )
	{
		return;
	}


}
