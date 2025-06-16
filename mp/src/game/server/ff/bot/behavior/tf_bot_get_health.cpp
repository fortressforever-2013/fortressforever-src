//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_get_health.h
// Pick up any nearby health kit
// Michael Booth, May 2009

#include "cbase.h"
#include "ff_gamerules.h"
#include "ff_obj.h"
#include "bot/ff_bot.h"
#include "bot/behavior/ff_bot_get_health.h"

extern ConVar ff_bot_path_lookahead_range;

ConVar ff_bot_health_critical_ratio( "ff_bot_health_critical_ratio", "0.3", FCVAR_CHEAT );
ConVar ff_bot_health_ok_ratio( "ff_bot_health_ok_ratio", "0.8", FCVAR_CHEAT );
ConVar ff_bot_health_search_near_range( "ff_bot_health_search_near_range", "1000", FCVAR_CHEAT );
ConVar ff_bot_health_search_far_range( "ff_bot_health_search_far_range", "2000", FCVAR_CHEAT );


//---------------------------------------------------------------------------------------------
class CHealthFilter : public INextBotFilter
{
public:
	CHealthFilter( CFFBot *me )
	{
		m_me = me;
	}

	bool IsSelected( const CBaseEntity *constCandidate ) const
	{
		if ( !constCandidate )
			return false;

		CBaseEntity *candidate = const_cast< CBaseEntity * >( constCandidate );

		CTFNavArea *area = (CTFNavArea *)TheNavMesh->GetNearestNavArea( candidate->WorldSpaceCenter() );
		if ( !area )
			return false;

		CClosestTFPlayer close( candidate );
		ForEachPlayer( close );

		// if the closest player to this candidate object is an enemy, don't use it
		if ( close.m_closePlayer && !m_me->InSameTeam( close.m_closePlayer ) )
			return false;

		// resupply cabinets (not assigned a team)
		if ( candidate->ClassMatches( "func_regenerate" ) )
		{
			if ( !area->HasAttributeTF( TF_NAV_SPAWN_ROOM_BLUE | TF_NAV_SPAWN_ROOM_RED ) )
			{
				// Assume any resupply cabinets not in a teamed spawn room are inaccessible.
				// Ex: pl_upward has forward spawn rooms that neither team can use until 
				// certain checkpoints are reached.
				return false;
			}

			if ( ( m_me->GetTeamNumber() == FF_TEAM_RED && area->HasAttributeTF( TF_NAV_SPAWN_ROOM_RED ) ) ||
				 ( m_me->GetTeamNumber() == FF_TEAM_BLUE && area->HasAttributeTF( TF_NAV_SPAWN_ROOM_BLUE ) ) )
			{
				// the supply cabinet is in my spawn room
				return true;
			}

			return false;
		}

		// ignore non-existent ammo to ensure we collect nearby existing ammo
		if ( candidate->IsEffectActive( EF_NODRAW ) )
			return false;

		if ( candidate->ClassMatches( "item_healthkit*" ) )
			return true;

		if ( m_me->InSameTeam( candidate ) )
		{
			// friendly engineer's dispenser
			if ( candidate->ClassMatches( "obj_dispenser*" ) )
			{
				CBaseObject	*dispenser = (CBaseObject *)candidate;
				if ( !dispenser->IsBuilding() && !dispenser->IsPlacing() && !dispenser->IsDisabled() )
				{
					return true;
				}
			}
		}

		return false;
	}

	CFFBot *m_me;
};


//---------------------------------------------------------------------------------------------
static CFFBot *s_possibleBot = NULL;
static CHandle< CBaseEntity > s_possibleHealth = NULL;
static int s_possibleFrame = 0;


//---------------------------------------------------------------------------------------------
/** 
 * Return true if this Action has what it needs to perform right now
 */
bool CFFBotGetHealth::IsPossible( CFFBot *me )
{
	VPROF_BUDGET( "CFFBotGetHealth::IsPossible", "NextBot" );





	float healthRatio = (float)me->GetHealth() / (float)me->GetMaxHealth();

	float t = ( healthRatio - ff_bot_health_critical_ratio.GetFloat() ) / ( ff_bot_health_ok_ratio.GetFloat() - ff_bot_health_critical_ratio.GetFloat() );
	t = clamp( t, 0.0f, 1.0f );



	// the more we are hurt, the farther we'll travel to get health
	float searchRange = ff_bot_health_search_far_range.GetFloat() + t * ( ff_bot_health_search_near_range.GetFloat() - ff_bot_health_search_far_range.GetFloat() );

	CUtlVector< CHandle< CBaseEntity > > healthVector;
	CHealthFilter healthFilter( me );

	me->SelectReachableObjects( FFGameRules()->GetHealthEntityVector(), &healthVector, healthFilter, me->GetLastKnownArea(), searchRange );

	if ( healthVector.Count() == 0 )
	{
		if ( me->IsDebugging( NEXTBOT_BEHAVIOR ) )
		{
			Warning( "%3.2f: No health nearby\n", gpGlobals->curtime );
		}
		return false;
	}

	// use the first item in the list, since it will be the closest to us (or nearly so)
	CBaseEntity *health = healthVector[0];
	for( int i=0; i<healthVector.Count(); ++i )
	{
		if ( healthVector[i]->GetTeamNumber() != GetEnemyTeam( me->GetTeamNumber() ) )
		{
			health = healthVector[i];
			break;
		}
	}

	if ( health == NULL )
	{
		if ( me->IsDebugging( NEXTBOT_BEHAVIOR ) )
		{
			Warning( "%3.2f: No health available to my team nearby\n", gpGlobals->curtime );
		}
		return false;
	}

	CFFBotPathCost cost( me, FASTEST_ROUTE );
	PathFollower path;
	if ( !path.Compute( me, health->WorldSpaceCenter(), cost ) )
	{
		if ( me->IsDebugging( NEXTBOT_BEHAVIOR ) )
		{
			Warning( "%3.2f: No path to health!\n", gpGlobals->curtime );
		}
		return false;
	}

	s_possibleBot = me;
	s_possibleHealth = health;
	s_possibleFrame = gpGlobals->framecount;

	return true;
}

//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotGetHealth::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	VPROF_BUDGET( "CFFBotGetHealth::OnStart", "NextBot" );

	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	// if IsPossible() has already been called, use its cached data
	if ( s_possibleFrame != gpGlobals->framecount || s_possibleBot != me )
	{
		if ( !IsPossible( me ) || s_possibleHealth == NULL )
		{
			return Done( "Can't get health" );
		}
	}

	m_healthKit = s_possibleHealth;
	m_isGoalDispenser = m_healthKit->ClassMatches( "obj_dispenser*" );

	CFFBotPathCost cost( me, SAFEST_ROUTE );
	if ( !m_path.Compute( me, m_healthKit->WorldSpaceCenter(), cost ) )
	{
		return Done( "No path to health!" );
	}



	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotGetHealth::Update( CFFBot *me, float interval )
{
	if ( m_healthKit == NULL || ( m_healthKit->IsEffectActive( EF_NODRAW ) && !FClassnameIs( m_healthKit, "func_regenerate" ) ) )
	{
		return Done( "Health kit I was going for has been taken" );
	}

	// if a medic is healing us, give up on getting a kit


	if ( me->GetHealth() >= me->GetMaxHealth() )
	{
		return Done( "I've been healed" );
	}

	// if the closest player to the item we're after is an enemy, give up
	CClosestTFPlayer close( m_healthKit );
	ForEachPlayer( close );
	if ( close.m_closePlayer && !me->InSameTeam( close.m_closePlayer ) )
		return Done( "An enemy is closer to it" );

       // un-zoom
       CFFWeaponBase *myWeapon = me->GetActiveFFWeapon();

	if ( !m_path.IsValid() )
	{
		// this can occur if we overshoot the health kit's location
		// because it is momentarily gone
		CFFBotPathCost cost( me, SAFEST_ROUTE );
		if ( !m_path.Compute( me, m_healthKit->WorldSpaceCenter(), cost ) )
		{
			return Done( "No path to health!" );
		}
	}

	m_path.Update( me );

	// may need to switch weapons (ie: engineer holding toolbox now needs to heal and defend himself)
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	me->EquipBestWeaponForThreat( threat );

	return Continue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotGetHealth::OnStuck( CFFBot *me )
{
	return TryDone( RESULT_CRITICAL, "Stuck trying to reach health kit" );
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotGetHealth::OnMoveToSuccess( CFFBot *me, const Path *path )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotGetHealth::OnMoveToFailure( CFFBot *me, const Path *path, MoveToFailureType reason )
{
	return TryDone( RESULT_CRITICAL, "Failed to reach health kit" );
}


//---------------------------------------------------------------------------------------------
// We are always hurrying if we need to collect health
QueryResultType CFFBotGetHealth::ShouldHurry( const INextBot *me ) const
{
	return ANSWER_YES;
}
