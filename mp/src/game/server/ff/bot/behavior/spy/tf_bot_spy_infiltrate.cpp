//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_spy_infiltrate.cpp
// Move into position behind enemy lines and wait for victims
// Michael Booth, June 2010

#include "cbase.h"
#include "ff_player.h"
#include "ff_obj_sentrygun.h"
#include "bot/ff_bot.h"
#include "bot/behavior/spy/ff_bot_spy_infiltrate.h"
#include "bot/behavior/spy/ff_bot_spy_sap.h"
#include "bot/behavior/spy/ff_bot_spy_attack.h"
#include "bot/behavior/ff_bot_retreat_to_cover.h"

#include "nav_mesh.h"

extern ConVar ff_bot_path_lookahead_range;

ConVar ff_bot_debug_spy( "ff_bot_debug_spy", "0", FCVAR_CHEAT );

//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSpyInfiltrate::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_hideArea = NULL;

	m_hasEnteredCombatZone = false;

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSpyInfiltrate::Update( CFFBot *me, float interval )
{
	// switch to our pistol
	CBaseCombatWeapon *myGun = me->Weapon_GetSlot( TF_WPN_TYPE_PRIMARY );
	if ( myGun )
	{
		me->Weapon_Switch( myGun );
	}

	CTFNavArea *myArea = me->GetLastKnownArea();

	if ( !myArea )
	{
		return Continue();
	}

	bool isInMySpawn = myArea->HasAttributeTF( TF_NAV_SPAWN_ROOM_BLUE | TF_NAV_SPAWN_ROOM_RED );
	if ( myArea->HasAttributeTF( TF_NAV_SPAWN_ROOM_EXIT ) )
	{
		// don't count exits so we cloak as we leave
		isInMySpawn = false;
	}

       // begin attack when we first enter an area of active combat
       if ( !isInMySpawn &&
                myArea->IsInCombat() &&
                !m_hasEnteredCombatZone )
       {
               m_hasEnteredCombatZone = true;
       }

	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat && threat->GetEntity() && threat->GetEntity()->IsBaseObject() )
	{
		CBaseObject *enemyObject = (CBaseObject *)threat->GetEntity();
		if ( !enemyObject->HasSapper() && me->IsEnemy( enemyObject ) )
		{
			return SuspendFor( new CFFBotSpySap( enemyObject ), "Sapping an enemy object" );
		}
	}

	if ( me->GetEnemySentry() && !me->GetEnemySentry()->HasSapper() )
	{
		return SuspendFor( new CFFBotSpySap( me->GetEnemySentry() ), "Sapping a Sentry" );
	}

	if ( !m_hideArea && m_findHidingSpotTimer.IsElapsed() )
	{
		FindHidingSpot( me );
		m_findHidingSpotTimer.Start( 3.0f );
	}

	if ( !FFGameRules()->InSetup() )
	{
		// go after victims we've gotten behind
		if ( threat && threat->GetTimeSinceLastKnown() < 3.0f )
		{
			CFFPlayer *victim = ToFFPlayer( threat->GetEntity() );
			if ( victim )
			{
				CTFNavArea *victimArea = (CTFNavArea *)victim->GetLastKnownArea();
				if ( victimArea )
				{
					int victimTeam = victim->GetTeamNumber();

                                       if ( victimArea->GetIncursionDistance( victimTeam ) > myArea->GetIncursionDistance( victimTeam ) )
                                       {
                                               return SuspendFor( new CFFBotSpyAttack( victim ), "Going after a backstab victim" );
                                       }
				}					
			}
		}
	}

	if ( m_hideArea )
	{
		if ( ff_bot_debug_spy.GetBool() )
		{
			m_hideArea->DrawFilled( 255, 255, 0, 255, NDEBUG_PERSIST_TILL_NEXT_SERVER );
		}

		if ( myArea == m_hideArea )
		{
			// stay hidden during setup time
			if ( FFGameRules()->InSetup() )
			{
				m_waitTimer.Start( RandomFloat( 0.0f, 5.0f ) );
			}
			else
			{
				// wait in our hiding spot for a bit, then try another
				if ( !m_waitTimer.HasStarted() )
				{
					m_waitTimer.Start( RandomFloat( 5.0f, 10.0f ) );
				}
				else if ( m_waitTimer.IsElapsed() )
				{
					// time to find a new hiding spot
					m_hideArea = NULL;
				}
			}
		}
		else
		{
			// move to our ambush position
			if ( m_repathTimer.IsElapsed() )
			{
				m_repathTimer.Start( RandomFloat( 1.0f, 2.0f ) );

				// we may not be able to path to our hiding spot, but get as close as we can
				// (dropdown mid spawn in cp_gorge)
				CFFBotPathCost cost( me, SAFEST_ROUTE );
				m_path.Compute( me, m_hideArea->GetCenter(), cost );
			}

			m_path.Update( me );

			m_waitTimer.Invalidate();
		}
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
void CFFBotSpyInfiltrate::OnEnd( CFFBot *me, Action< CFFBot > *nextAction )
{
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSpyInfiltrate::OnSuspend( CFFBot *me, Action< CFFBot > *interruptingAction )
{
	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSpyInfiltrate::OnResume( CFFBot *me, Action< CFFBot > *interruptingAction )
{
	m_repathTimer.Invalidate();
	m_hideArea = NULL;

	return Continue();
}


//---------------------------------------------------------------------------------------------
bool CFFBotSpyInfiltrate::FindHidingSpot( CFFBot *me )
{
	m_hideArea = NULL;

	if ( me->GetAliveDuration() < 5.0f && FFGameRules()->InSetup() )
	{
		// wait a bit until the nav mesh has updated itself
		return false;
	}

	int myTeam = me->GetTeamNumber();
	const CUtlVector< CTFNavArea * > *enemySpawnExitVector = TheTFNavMesh()->GetSpawnRoomExitAreas( GetEnemyTeam( myTeam ) );


	if ( !enemySpawnExitVector || enemySpawnExitVector->Count() == 0 )
	{
		if ( ff_bot_debug_spy.GetBool() )
		{
			DevMsg( "%3.2f: No enemy spawn room exit areas found\n", gpGlobals->curtime );
		}
		return false;
	}

	// find nearby place to hide hear enemy spawn exit(s)
	CUtlVector< CNavArea * > nearbyAreaVector;
	const float nearbyHideRange = 2500.0f;
	for( int x=0; x<enemySpawnExitVector->Count(); ++x )
	{
		CTFNavArea *enemySpawnExitArea = enemySpawnExitVector->Element( x );

		CUtlVector< CNavArea * > nearbyThisExitAreaVector;
		CollectSurroundingAreas( &nearbyThisExitAreaVector, enemySpawnExitArea, nearbyHideRange, me->GetLocomotionInterface()->GetStepHeight(), me->GetLocomotionInterface()->GetStepHeight() );

		// concat vectors (assuming N^2 unique search would cost more than ripping through some duplicates)
		nearbyAreaVector.AddVectorToTail( nearbyThisExitAreaVector );
	}

	// find area not visible to any enemy spawn exits
	CUtlVector< CTFNavArea * > hideAreaVector;
	int i;

	for( i=0; i<nearbyAreaVector.Count(); ++i )
	{
		CTFNavArea *area = (CTFNavArea *)nearbyAreaVector[i];

		if ( !me->GetLocomotionInterface()->IsAreaTraversable( area ) )
			continue;

		bool isHidden = true;
		for( int j=0; j<enemySpawnExitVector->Count(); ++j )
		{
			if ( area->IsPotentiallyVisible( enemySpawnExitVector->Element(j) ) )
			{
				isHidden = false;
				break;
			}
		}

		if ( isHidden )
		{
			hideAreaVector.AddToTail( area );
		}
	}

	if ( hideAreaVector.Count() == 0 )
	{
		if ( ff_bot_debug_spy.GetBool() )
		{
			DevMsg( "%3.2f: Can't find any non-visible hiding areas, trying for anything near the spawn exit...\n", gpGlobals->curtime );
		}

		for( i=0; i<nearbyAreaVector.Count(); ++i )
		{
			CTFNavArea *area = (CTFNavArea *)nearbyAreaVector[i];

			if ( !me->GetLocomotionInterface()->IsAreaTraversable( area ) )
				continue;

			hideAreaVector.AddToTail( area );
		}
	}

	if ( hideAreaVector.Count() == 0 )
	{
		if ( ff_bot_debug_spy.GetBool() )
		{
			DevMsg( "%3.2f: Can't find any areas near the enemy spawn exit - just heading to the enemy spawn and hoping...\n", gpGlobals->curtime );
		}

		m_hideArea = enemySpawnExitVector->Element( RandomInt( 0, enemySpawnExitVector->Count()-1 ) );

		return false;
	}

	// pick a specific hiding spot
	m_hideArea = hideAreaVector[ RandomInt( 0, hideAreaVector.Count()-1 ) ];

	return true;
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotSpyInfiltrate::OnStuck( CFFBot *me )
{
	m_hideArea = NULL;
	m_findHidingSpotTimer.Invalidate();

	return TryContinue( RESULT_TRY );
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotSpyInfiltrate::OnTerritoryCaptured( CFFBot *me, int territoryID )
{
	// enemy spawn likely changed - find new hiding spot after internal data has updated
	m_hideArea = NULL;
	m_findHidingSpotTimer.Start( 5.0f );

	return TryContinue( RESULT_TRY );
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotSpyInfiltrate::OnTerritoryLost( CFFBot *me, int territoryID )
{
	// enemy spawn likely changed - find new hiding spot after internal data has updated
	m_hideArea = NULL;
	m_findHidingSpotTimer.Start( 5.0f );

	return TryContinue( RESULT_TRY );
}


//---------------------------------------------------------------------------------------------
QueryResultType CFFBotSpyInfiltrate::ShouldAttack( const INextBot *me, const CKnownEntity *them ) const
{
	return ANSWER_NO;
}
