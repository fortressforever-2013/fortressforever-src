//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_sniper_lurk.h
// Move into position and wait for victims
// Michael Booth, October 2009

#include "cbase.h"
#include "ff_player.h"


#include "bot/ff_bot.h"
#include "bot/behavior/sniper/ff_bot_sniper_lurk.h"
#include "bot/behavior/sniper/ff_bot_sniper_attack.h"
#include "bot/behavior/ff_bot_retreat_to_cover.h"
#include "bot/behavior/ff_bot_melee_attack.h"
#include "bot/map_entities/ff_bot_hint.h"

#include "nav_mesh.h"

extern ConVar ff_bot_path_lookahead_range;
extern ConVar ff_bot_sniper_flee_range;
extern ConVar ff_bot_sniper_melee_range;
extern ConVar ff_bot_debug_sniper;

extern float SkewedRandomValue( void );

ConVar ff_bot_sniper_patience_duration( "ff_bot_sniper_patience_duration", "10", FCVAR_CHEAT, "How long a Sniper bot will wait without seeing an enemy before picking a new spot" );
ConVar ff_bot_sniper_target_linger_duration( "ff_bot_sniper_target_linger_duration", "2", FCVAR_CHEAT, "How long a Sniper bot will keep toward at a target it just lost sight of" );
ConVar ff_bot_sniper_allow_opportunistic( "ff_bot_sniper_allow_opportunistic", "1", FCVAR_NONE, "If set, Snipers will stop on their way to their preferred lurking spot to snipe at opportunistic targets" );


#ifdef STAGING_ONLY
extern ConVar ff_bot_use_items;
#endif

//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSniperLurk::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_boredTimer.Start( RandomFloat( 0.9f, 1.1f ) * ff_bot_sniper_patience_duration.GetFloat() );

	m_homePosition = me->GetAbsOrigin();
	m_isHomePositionValid = false;
	m_isAtHome = false;
	m_failCount = 0;

	m_isOpportunistic = ff_bot_sniper_allow_opportunistic.GetBool();

	CFFBotHint *hint = NULL;
	while( ( hint = (CFFBotHint *)( gEntList.FindEntityByClassname( hint, "func_tfbot_hint" ) ) ) != NULL )
	{
		if ( hint->IsA( CFFBotHint::HINT_SNIPER_SPOT ) )
		{
			m_hintVector.AddToTail( hint );

			// make sure we don't yet own any of these hints
			if ( me->IsSelf( hint->GetOwnerEntity() ) )
			{
				hint->SetOwnerEntity( NULL );
			}
		}
	}

	m_priorHint = NULL;




	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSniperLurk::Update( CFFBot *me, float interval )
{
       {
		// continuously search for good sniping spots
		me->AccumulateSniperSpots();

		if ( !m_isHomePositionValid )
		{
			// just found our first sniper spot - update our home position
			FindNewHome( me );
		}
	}

	// aim at bad guys
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();

	if ( threat && !threat->GetEntity()->IsAlive() )
	{
		// he's dead
		threat = NULL;
	}

	if ( threat && me->GetIntentionInterface()->ShouldAttack( me, threat ) == ANSWER_NO )
	{
		threat = NULL;
	}

	if ( threat && threat->IsVisibleInFOVNow() )
	{
		m_failCount = 0;

		if ( me->IsDistanceBetweenLessThan( threat->GetLastKnownPosition(), ff_bot_sniper_melee_range.GetFloat() ) )
		{
			const float giveUpRange = 1.25f * ff_bot_sniper_melee_range.GetFloat();
			return SuspendFor( new CFFBotMeleeAttack( giveUpRange ), "Melee attacking nearby threat" );
		}
	}

	bool isSightingRifle = false;

	if ( threat && 
		 threat->GetTimeSinceLastSeen() < ff_bot_sniper_target_linger_duration.GetFloat() &&
		 me->IsLineOfFireClear( threat->GetEntity() ) )
	{
		// we see something...
		if ( m_isOpportunistic )
		{
			// switch to our sniper rifle
			CBaseCombatWeapon *myGun = me->Weapon_GetSlot( TF_WPN_TYPE_PRIMARY );
			if ( myGun )
			{
				me->Weapon_Switch( myGun );
			}

			isSightingRifle = true;
			m_boredTimer.Reset();

			if ( !m_isHomePositionValid )
			{
				// make this our opportunistic home for awhile
				m_homePosition = me->GetAbsOrigin();
				m_boredTimer.Start( RandomFloat( 0.9f, 1.1f ) * ff_bot_sniper_patience_duration.GetFloat() );
			}
		}
		else
		{
			// switch to our SMG and fire while we run
			CBaseCombatWeapon *myGun = me->Weapon_GetSlot( TF_WPN_TYPE_SECONDARY );
			if ( myGun )
			{
				me->Weapon_Switch( myGun );
			}
		}
	}

	const float homeRange = 25.0f; // 100.0f;
	m_isAtHome = ( me->GetAbsOrigin() - m_homePosition ).AsVector2D().IsLengthLessThan( homeRange );

	if ( m_isAtHome )
	{
		isSightingRifle = true;

		// once we've reached a good home spot, opportunistically attack from there
		m_isOpportunistic = ff_bot_sniper_allow_opportunistic.GetBool();

		if ( m_boredTimer.IsElapsed() )
		{
			++m_failCount;

			if ( FindNewHome( me ) )
			{
				me->SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_NEGATIVE );
				m_boredTimer.Start( RandomFloat( 0.9f, 1.1f ) * ff_bot_sniper_patience_duration.GetFloat() );
			}
			else
			{
				// try again soon
				m_boredTimer.Start( 1.0f );
			}
		}
	}
	else
	{
		// not yet at home - can't start to be bored
		m_boredTimer.Reset();
	}

	if ( isSightingRifle )
	{
		// switch to our sniper rifle
		CFFWeaponBase *myGun = (CFFWeaponBase *)me->Weapon_GetSlot( TF_WPN_TYPE_PRIMARY );
		if ( myGun )
		{
			me->Weapon_Switch( myGun );

			if ( !me->m_Shared.InCond( TF_COND_ZOOMED ) && !myGun->IsWeapon( FF_WEAPON_COMPOUND_BOW ) )
			{
				// zoom in and stand still
				me->PressAltFireButton();
			}
		}
	}
	else 
	{
		// move to our home position
		if ( m_repathTimer.IsElapsed() )
		{
			m_repathTimer.Start( RandomFloat( 1.0f, 2.0f ) );

			CFFBotPathCost cost( me, SAFEST_ROUTE );
			m_path.Compute( me, m_homePosition, cost );
		}

		m_path.Update( me );
		
		if ( me->m_Shared.InCond( TF_COND_ZOOMED ) )
		{
			me->PressAltFireButton();
		}
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
void CFFBotSniperLurk::OnEnd( CFFBot *me, Action< CFFBot > *nextAction )
{
	if ( me->m_Shared.InCond( TF_COND_ZOOMED ) )
	{
		// we're leaving to do something else - unzoom
		me->PressAltFireButton();
	}

	if ( m_priorHint != NULL )
	{
		// release my hint
		m_priorHint->SetOwnerEntity( NULL );

		if ( ff_bot_debug_sniper.GetBool() )
		{
			DevMsg( "%3.2f: %s: Releasing hint.\n", gpGlobals->curtime, me->GetPlayerName() );
		}
	}
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSniperLurk::OnSuspend( CFFBot *me, Action< CFFBot > *interruptingAction )
{
	if ( me->m_Shared.InCond( TF_COND_ZOOMED ) )
	{
		// we're leaving to do something else - unzoom
		me->PressAltFireButton();
	}

	if ( m_priorHint != NULL )
	{
		// release my hint
		m_priorHint->SetOwnerEntity( NULL );

		if ( ff_bot_debug_sniper.GetBool() )
		{
			DevMsg( "%3.2f: %s: Releasing hint.\n", gpGlobals->curtime, me->GetPlayerName() );
		}
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotSniperLurk::OnResume( CFFBot *me, Action< CFFBot > *interruptingAction )
{
	m_repathTimer.Invalidate();
	m_priorHint = NULL;

	// we probably just fetched some health because the enemy shot us - pick a new place to lurk
	FindNewHome( me );

	return Continue();
}


//---------------------------------------------------------------------------------------------
bool CFFBotSniperLurk::FindHint( CFFBot *me )
{
	// if any sniper spot hints exist, pick one of them
	CUtlVector< CFFBotHint * > activeHintVector;
	for( int i=0; i<m_hintVector.Count(); ++i )
	{
		if ( m_hintVector[i] != NULL && m_hintVector[i]->IsFor( me ) )
		{
			activeHintVector.AddToTail( m_hintVector[i] );
		}
	}

	if ( activeHintVector.Count() == 0 )
	{
		return false;
	}

	if ( m_priorHint != NULL )
	{
		// release my hint
		m_priorHint->SetOwnerEntity( NULL );

		if ( ff_bot_debug_sniper.GetBool() )
		{
			DevMsg( "%3.2f: %s: Releasing hint.\n", gpGlobals->curtime, me->GetPlayerName() );
		}
	}

	CFFBotHint *hint = NULL;

	if ( m_priorHint != NULL && m_failCount < 2 )
	{
		// there used to be targets here - pick nearby hint
		float nearRange = 500.0f;
		CUtlVector< CFFBotHint * > nearHintVector;
		for( int i=0; i<activeHintVector.Count(); ++i )
		{
			if ( activeHintVector[i] == m_priorHint )
				continue;

			if ( ( activeHintVector[i]->WorldSpaceCenter() - m_priorHint->WorldSpaceCenter() ).IsLengthGreaterThan( nearRange ) )
				continue;

			if ( activeHintVector[i]->GetOwnerEntity() != NULL )
				continue;

			nearHintVector.AddToTail( activeHintVector[i] );
		}

		if ( nearHintVector.Count() == 0 )
		{
			++m_failCount;
			return false;
		}

		int whichHint = RandomInt( 0, nearHintVector.Count()-1 );
		hint = nearHintVector[ whichHint ];
	}
	else
	{
		// picking either our first hint, or we haven't seen a victim in a long time - pick a hint that can actually see someone
		CUtlVector< CFFPlayer * > victimVector;
		CollectPlayers( &victimVector, GetEnemyTeam( me->GetTeamNumber() ), COLLECT_ONLY_LIVING_PLAYERS );

		CUtlVector< CFFBotHint * > hotHintVector;
		CUtlVector< CFFBotHint * > freeHintVector;

		for( int i=0; i<activeHintVector.Count(); ++i )
		{
			if ( activeHintVector[i]->GetOwnerEntity() != NULL )
				continue;

			freeHintVector.AddToTail( activeHintVector[i] );

			for( int p=0; p<victimVector.Count(); ++p )
			{
				if ( victimVector[p]->IsLineOfSightClear( activeHintVector[i]->WorldSpaceCenter(), CBaseCombatCharacter::IGNORE_ACTORS ) )
				{
					// at least one victim is visible from this hint
					hotHintVector.AddToTail( activeHintVector[i] );
					break;
				}
			}
		}

		if ( hotHintVector.Count() == 0 )
		{
			// no hints can see any victims - pick at random
			if ( freeHintVector.Count() == 0 )
			{
				// all hints are owned by another sniper - double up
				int whichHint = RandomInt( 0, activeHintVector.Count()-1 );
				hint = activeHintVector[ whichHint ];

				if ( ff_bot_debug_sniper.GetBool() )
				{
					DevMsg( "%3.2f: %s: No un-owned hints available! Doubling up.\n", gpGlobals->curtime, me->GetPlayerName() );
				}
			}
			else
			{
				int whichHint = RandomInt( 0, freeHintVector.Count()-1 );
				hint = freeHintVector[ whichHint ];
			}
		}
		else
		{
			int whichHint = RandomInt( 0, hotHintVector.Count()-1 );
			hint = hotHintVector[ whichHint ];
		}
	}

	if ( hint == NULL )
	{
		return false;
	}

	Extent hintExtent;
	hintExtent.Init( hint );

	Vector hintSpot;
	hintSpot.x = RandomFloat( hintExtent.lo.x, hintExtent.hi.x );
	hintSpot.y = RandomFloat( hintExtent.lo.y, hintExtent.hi.y );
	hintSpot.z = ( hintExtent.lo.z + hintExtent.hi.z ) / 2.0f;

	TheNavMesh->GetSimpleGroundHeight( hintSpot, &hintSpot.z );

	m_homePosition = hintSpot;
	m_isHomePositionValid = true;
	m_priorHint = hint;

	// my hint
	hint->SetOwnerEntity( me );

	return true;
}


//---------------------------------------------------------------------------------------------
bool CFFBotSniperLurk::FindNewHome( CFFBot *me )
{
	if ( !m_findHomeTimer.IsElapsed() )
	{
		return false;
	}

	m_findHomeTimer.Start( RandomFloat( 1.0f, 2.0f ) );


       {
		// if any sniper spot hints exist, pick one of them
		if ( FindHint( me ) )
		{
			return true;
		}

		// pick a sniper spot from our ongoing search
		const CUtlVector< CFFBot::SniperSpotInfo > *sniperSpotVector = me->GetSniperSpots();
		if ( sniperSpotVector->Count() > 0 )
		{
			m_homePosition = sniperSpotVector->Element( RandomInt( 0, sniperSpotVector->Count()-1 ) ).m_vantageSpot;
			m_isHomePositionValid = true;
			return true;
		}
	}

	// can't find a real sniper spot - pick another goal that will get us out into the fray
	m_isHomePositionValid = false;

	// head toward the point
	CTeamControlPoint *point = me->GetMyControlPoint();
	if ( point && !point->IsLocked() )
	{
		const CUtlVector< CTFNavArea * > *pointAreaVector = TheTFNavMesh()->GetControlPointAreas( point->GetPointIndex() );

		if ( pointAreaVector && pointAreaVector->Count() > 0 )
		{
			int which = RandomInt( 0, pointAreaVector->Count()-1 );

			m_homePosition = pointAreaVector->Element( which )->GetRandomPoint();

			return false;
		}
	}

	// no available point at the moment - head toward the enemy spawn room and opportunistically snipe
	CUtlVector< CTFNavArea * > enemySpawnThresholdVector;
	TheTFNavMesh()->CollectSpawnRoomThresholdAreas( &enemySpawnThresholdVector, GetEnemyTeam( me->GetTeamNumber() ) );

	if ( enemySpawnThresholdVector.Count() > 0 )
	{
		m_homePosition = enemySpawnThresholdVector[ RandomInt( 0, enemySpawnThresholdVector.Count()-1 ) ]->GetCenter();
	}
	else
	{
		m_homePosition = me->GetAbsOrigin();
	}

	return false;
}


//---------------------------------------------------------------------------------------------
QueryResultType CFFBotSniperLurk::ShouldAttack( const INextBot *bot, const CKnownEntity *them ) const
{
	CFFBot *me = (CFFBot *)bot->GetEntity();

	CTFNavArea *area = me->GetLastKnownArea();

	{
		// don't fire while in the spawn area
		return ANSWER_NO;
	}

	// take the shot if you've got it
	return ANSWER_YES;
}


//---------------------------------------------------------------------------------------------
QueryResultType CFFBotSniperLurk::ShouldRetreat( const INextBot *me ) const
{
	{
		return ANSWER_NO;
	}

	return ANSWER_UNDEFINED;
}

//---------------------------------------------------------------------------------------------
// Return the more dangerous of the two threats to 'subject', or NULL if we have no opinion
const CKnownEntity *CFFBotSniperLurk::SelectMoreDangerousThreat( const INextBot *meBot, 
																 const CBaseCombatCharacter *subject,
																 const CKnownEntity *threat1, 
																 const CKnownEntity *threat2 ) const
{
       return NULL;
}
