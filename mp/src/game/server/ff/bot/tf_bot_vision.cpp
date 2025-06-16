//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_vision.cpp
// Team Fortress NextBot vision interface
// Michael Booth, May 2009

#include "cbase.h"
#include "vprof.h"

#include "ff_bot.h"
#include "ff_bot_vision.h"
#include "ff_player.h"
#include "ff_gamerules.h"
#include "ff_obj_sentrygun.h"

ConVar ff_bot_choose_target_interval( "ff_bot_choose_target_interval", "0.3f", FCVAR_CHEAT, "How often, in seconds, a TFBot can reselect his target" );
ConVar ff_bot_sniper_choose_target_interval( "ff_bot_sniper_choose_target_interval", "3.0f", FCVAR_CHEAT, "How often, in seconds, a zoomed-in Sniper can reselect his target" );


//------------------------------------------------------------------------------------------
// Update internal state
void CFFBotVision::Update( void )
{
	if ( false )
	{
		if ( !m_scanTimer.IsElapsed() )
		{
			return;
		}

		m_scanTimer.Start( RandomFloat( 0.9f, 1.1f ) );
	}

	IVision::Update();

	CFFBot *me = (CFFBot *)GetBot()->GetEntity();
	if ( !me )
		return;

	// forget spies we have lost sight of
	CUtlVector< CFFPlayer * > playerVector;
	CollectPlayers( &playerVector, GetEnemyTeam( me->GetTeamNumber() ), COLLECT_ONLY_LIVING_PLAYERS );

	for( int i=0; i<playerVector.Count(); ++i )
	{
		if ( !playerVector[i]->IsPlayerClass( CLASS_SPY ) )
			continue;

		const CKnownEntity *known = GetKnown( playerVector[i] );

		if ( !known || !known->IsVisibleRecently() )
		{
			// if a hidden spy changes disguises, we no longer recognize him
			{
				me->ForgetSpy( playerVector[i] );				
			}
		}
	}
}


//------------------------------------------------------------------------------------------
void CFFBotVision::CollectPotentiallyVisibleEntities( CUtlVector< CBaseEntity * > *potentiallyVisible )
{
	VPROF_BUDGET( "CFFBotVision::CollectPotentiallyVisibleEntities", "NextBot" );

	potentiallyVisible->RemoveAll();

	// include all players
	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = UTIL_PlayerByIndex( i );

		if ( player == NULL )
			continue;

		if ( FNullEnt( player->edict() ) )
			continue;

		if ( !player->IsPlayer() )
			continue;

		if ( !player->IsConnected() )
			continue;

		if ( !player->IsAlive() )
			continue;

		potentiallyVisible->AddToTail( player );
	}

	// include sentry guns
	UpdatePotentiallyVisibleNPCVector();

	FOR_EACH_VEC( m_potentiallyVisibleNPCVector, it )
	{
		potentiallyVisible->AddToTail( m_potentiallyVisibleNPCVector[ it ] );
	}
}


//------------------------------------------------------------------------------------------
void CFFBotVision::UpdatePotentiallyVisibleNPCVector( void )
{
	if ( m_potentiallyVisibleUpdateTimer.IsElapsed() )
	{
		m_potentiallyVisibleUpdateTimer.Start( RandomFloat( 3.0f, 4.0f ) );

		// collect list of active buildings
		m_potentiallyVisibleNPCVector.RemoveAll();

		bool bShouldSeeTeleporter = true;
		for ( int i=0; i<IBaseObjectAutoList::AutoList().Count(); ++i )
		{
			CBaseObject* pObj = static_cast< CBaseObject* >( IBaseObjectAutoList::AutoList()[i] );
			if ( pObj->ObjectType() == OBJ_SENTRYGUN )
			{
				m_potentiallyVisibleNPCVector.AddToTail( pObj );
			}
			else if ( pObj->ObjectType() == OBJ_DISPENSER && pObj->ClassMatches( "obj_dispenser" ) )
			{
				m_potentiallyVisibleNPCVector.AddToTail( pObj );
			}
			else if ( bShouldSeeTeleporter && pObj->ObjectType() == OBJ_TELEPORTER )
			{
				m_potentiallyVisibleNPCVector.AddToTail( pObj );
			}
		}

		CUtlVector< INextBot * > botVector;
		TheNextBots().CollectAllBots( &botVector );
		for( int i=0; i<botVector.Count(); ++i )
		{
			CBaseCombatCharacter *botEntity = botVector[i]->GetEntity();
			if ( botEntity && !botEntity->IsPlayer() )
			{
				// NPC
				m_potentiallyVisibleNPCVector.AddToTail( botEntity );
			}
		}
	}
}


//------------------------------------------------------------------------------------------
/**
 * Return true to completely ignore this entity.
 * This is mostly for enemy spies.  If we don't ignore them, we will look at them.
 */
bool CFFBotVision::IsIgnored( CBaseEntity *subject ) const
{
	CFFBot *me = (CFFBot *)GetBot()->GetEntity();



	if ( me->IsAttentionFocused() )
	{
		// our attention is restricted to certain subjects
		if ( !me->IsAttentionFocusedOn( subject ) )
		{
			return false;
		}
	}

	if ( !me->IsEnemy( subject ) )
	{
		// don't ignore friends
		return false;
	}

	if ( subject->IsEffectActive( EF_NODRAW ) )
	{
		return true;
	}

	if ( subject->IsPlayer() )
	{
		CFFPlayer *enemy = static_cast< CFFPlayer * >( subject );

		// test for designer-defined ignorance
		switch( enemy->GetPlayerClass()->GetClassIndex() )
		{
		case CLASS_MEDIC:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_MEDICS ) )
			{
				return true;
			}
			break;

		case CLASS_ENGINEER:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_ENGINEERS ) )
			{
				return true;
			}
			break;

		case CLASS_SNIPER:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_SNIPERS ) )
			{
				return true;
			}
			break;

		case CLASS_SCOUT:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_SCOUTS ) )
			{
				return true;
			}
			break;

		case CLASS_SPY:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_SPIES ) )
			{
				return true;
			}
			break;

		case CLASS_DEMOMAN:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_DEMOMEN ) )
			{
				return true;
			}
			break;

		case CLASS_SOLDIER:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_SOLDIERS ) )
			{
				return true;
			}
			break;

		case CLASS_HEAVYWEAPONS:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_HEAVIES ) )
			{
				return true;
			}
			break;

		case CLASS_PYRO:
			if ( me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_PYROS ) )
			{
				return true;
			}
			break;
		}


		if ( me->IsKnownSpy( enemy ) )
		{
			// don't ignore revealed spies
			return false;
		}

		if ( enemy->m_Shared.InCond( TF_COND_BURNING ) ||
			 enemy->m_Shared.InCond( TF_COND_URINE ) ||
			 enemy->m_Shared.InCond( TF_COND_STEALTHED_BLINK ) ||
			 enemy->m_Shared.InCond( TF_COND_BLEEDING ) )
		{
			// always notice players with these conditions
			return false;
		}

		// while in stealth, and for a short period after it drops
		if ( enemy->m_Shared.InCond( TF_COND_STEALTHED_USER_BUFF_FADING ) )
		{
			return true;
		}

		{
			{
				// spy is partially cloaked, and therefore attracts our attention
				return false;
			}

			// invisible!
			return true;
		}


		{
			return false;
		}
		
		{
			// spy is disguised as a member of my team
			return true;
		}
	}
	else if ( subject->IsBaseObject() ) // not a player
	{
		CBaseObject *object = assert_cast< CBaseObject * >( subject );
		if ( object )
		{
			// ignore sapped enemy objects
			if ( object->HasSapper() )
			{
				// so an engineer can die and run back in time to repair their stuff
				if ( FFGameRules() && false )
				{
					return false;
				}

				return true;
			}
			
			// ignore carried objects
			if ( object->IsPlacing() || object->IsCarried() )
			{
				return true;
			}
			
			if ( object->GetType() == OBJ_SENTRYGUN && me->IsBehaviorFlagSet( TFBOT_IGNORE_ENEMY_SENTRY_GUNS ) )
			{
				return true;
			}
		}
	}

	return false;
}


//------------------------------------------------------------------------------------------
// Return true if we 'notice' the subject, even though we have LOS to it
bool CFFBotVision::IsVisibleEntityNoticed( CBaseEntity *subject ) const
{
	CFFBot *me = (CFFBot *)GetBot()->GetEntity();

	if ( subject->IsPlayer() && me->IsEnemy( subject ) )
	{
		CFFPlayer *player = static_cast< CFFPlayer * >( subject );

		if ( player->m_Shared.InCond( TF_COND_BURNING ) ||
			 player->m_Shared.InCond( TF_COND_URINE ) ||
			 player->m_Shared.InCond( TF_COND_STEALTHED_BLINK ) ||
			 player->m_Shared.InCond( TF_COND_BLEEDING ) )
		{
			// always notice players with these conditions
			if ( player->m_Shared.InCond( TF_COND_STEALTHED ) )
			{
				me->RealizeSpy( player );
			}
			return true;
		}


		// while in stealth, and for a short period after it drops
		if ( player->m_Shared.InCond( TF_COND_STEALTHED_USER_BUFF_FADING ) )
		{
			me->ForgetSpy( player );
			return false;
		}

		{
			{
				// spy is partially cloaked, and therefore attracts our attention
				me->RealizeSpy( player );
				return true;
			}

			// invisible!
			me->ForgetSpy( player );
			return false;
		}

		{
			CFFBot::SuspectedSpyInfo_t* pSuspectInfo = me->IsSuspectedSpy( player );
			// But only if we aren't suspecting them currently.  This happens when we bump into them.
			if( !pSuspectInfo || !pSuspectInfo->IsCurrentlySuspected() )
			{
				{
					me->ForgetSpy( player );
					return false;
				}
			}
		}

		if ( me->IsKnownSpy( player ) )
		{
			// always notice non-invisible revealed spies
			return true;
		}

		{
			// spotted a spy!
			me->RealizeSpy( player );
			return true;
		}

		{
			// spy is disguised as a member of my team, don't notice him
			return false;
		}
	}

	return true;
}


//------------------------------------------------------------------------------------------
// Return VISUAL reaction time
float CFFBotVision::GetMinRecognizeTime( void ) const
{
	CFFBot *me = (CFFBot *)GetBot();

	switch ( me->GetDifficulty() )
	{
	case CFFBot::EASY:		return 1.0f;
	case CFFBot::NORMAL:	return 0.5f;
	case CFFBot::HARD:		return 0.3f;
	case CFFBot::EXPERT:	return 0.2f;
	}

	return 1.0f;
}



//------------------------------------------------------------------------------------------
float CFFBotVision::GetMaxVisionRange( void ) const
{
	CFFBot *me = (CFFBot *)GetBot();

	if ( me->GetMaxVisionRangeOverride() > 0.0f )
	{
		// designer specified vision range
		return me->GetMaxVisionRangeOverride();
	}

	// long range, particularly for snipers
	return 6000.0f;
}
