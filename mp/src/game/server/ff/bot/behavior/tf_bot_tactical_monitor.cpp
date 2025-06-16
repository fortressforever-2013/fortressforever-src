//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_tactical_monitor.cpp
// Behavior layer that interrupts for ammo/health/retreat/etc
// Michael Booth, June 2009

#include "cbase.h"
#include "fmtstr.h"

#include "ff_gamerules.h"
#include "ff_weapon_pipebomblauncher.h"
#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"

#include "bot/ff_bot.h"
#include "bot/ff_bot_manager.h"

#include "bot/behavior/ff_bot_tactical_monitor.h"
#include "bot/behavior/ff_bot_scenario_monitor.h"

#include "bot/behavior/ff_bot_seek_and_destroy.h"
#include "bot/behavior/ff_bot_retreat_to_cover.h"
#include "bot/behavior/ff_bot_taunt.h"
#include "bot/behavior/ff_bot_get_health.h"
#include "bot/behavior/ff_bot_get_ammo.h"
#include "bot/behavior/ff_bot_destroy_enemy_sentry.h"
#include "bot/behavior/ff_bot_use_teleporter.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_destroy_entity.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_move_to.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_wait.h"
#include "bot/behavior/engineer/ff_bot_engineer_building.h"
#include "bot/behavior/squad/ff_bot_escort_squad_leader.h"

#include "bot/behavior/training/ff_bot_training.h"
#include "bot/map_entities/ff_bot_hint_sentrygun.h"

#include "ff_obj_sentrygun.h"
#include "ff_item_system.h"

extern ConVar ff_bot_health_ok_ratio;
extern ConVar ff_bot_health_critical_ratio;

ConVar ff_bot_force_jump( "ff_bot_force_jump", "0", FCVAR_CHEAT, "Force bots to continuously jump" );


Action< CFFBot > *CFFBotTacticalMonitor::InitialContainedAction( CFFBot *me )
{
	return new CFFBotScenarioMonitor;
}


//-----------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotTacticalMonitor::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	return Continue();
}


//-----------------------------------------------------------------------------------------
void CFFBotTacticalMonitor::MonitorArmedStickyBombs( CFFBot *me )
{
	if ( m_stickyBombCheckTimer.IsElapsed() )
	{
		m_stickyBombCheckTimer.Start( RandomFloat( 0.3f, 1.0f ) );

		// are there any enemies on/near my sticky bombs?
		CTFPipebombLauncher *gun = dynamic_cast< CTFPipebombLauncher * >( me->Weapon_GetSlot( TF_WPN_TYPE_SECONDARY ) );
		if ( gun )
		{
			const CUtlVector< CHandle< CTFGrenadePipebombProjectile > > &pipeBombVector = gun->GetPipeBombVector();

			if ( pipeBombVector.Count() > 0 )
			{
				CUtlVector< CKnownEntity > knownVector;
				me->GetVisionInterface()->CollectKnownEntities( &knownVector );

				for( int p=0; p<pipeBombVector.Count(); ++p )
				{
					CTFGrenadePipebombProjectile *sticky = pipeBombVector[p];
					if ( !sticky )
					{
						continue;
					}

					for( int k=0; k<knownVector.Count(); ++k )
					{
						if ( knownVector[k].IsObsolete() )
						{
							continue;
						}

						if ( knownVector[k].GetEntity()->IsBaseObject() )
						{
							// we want to put several stickies on a sentry and det at once
							continue;
						}

						if ( sticky->GetTeamNumber() != GetEnemyTeam( knownVector[k].GetEntity()->GetTeamNumber() ) )
						{
							// "known" is either a spectator, or on our team
							continue;
						}

						const float closeRange = 150.0f;
						if ( ( knownVector[k].GetLastKnownPosition() - sticky->GetAbsOrigin() ).IsLengthLessThan( closeRange ) )
						{
							// they are close - blow it!
							me->PressAltFireButton();
							return;
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------
void CFFBotTacticalMonitor::AvoidBumpingEnemies( CFFBot *me )
{
	if ( me->GetDifficulty() < CFFBot::HARD )
		return;

	const float avoidRange = 200.0f;

	CUtlVector< CFFPlayer * > enemyVector;
	CollectPlayers( &enemyVector, GetEnemyTeam( me->GetTeamNumber() ), COLLECT_ONLY_LIVING_PLAYERS );

	CFFPlayer *closestEnemy = NULL;
	float closestRangeSq = avoidRange * avoidRange;

	for( int i=0; i<enemyVector.Count(); ++i )
	{
		CFFPlayer *enemy = enemyVector[i];


		float rangeSq = ( enemy->GetAbsOrigin() - me->GetAbsOrigin() ).LengthSqr();
		if ( rangeSq < closestRangeSq )
		{
			closestEnemy = enemy;
			closestRangeSq = rangeSq;
		}
	}

	if ( !closestEnemy )
		return;

	// avoid unless hindrance returns a definitive "no"
	if ( me->GetIntentionInterface()->IsHindrance( me, closestEnemy ) == ANSWER_UNDEFINED )
	{
		me->ReleaseForwardButton();
		me->ReleaseLeftButton();
		me->ReleaseRightButton();
		me->ReleaseBackwardButton();

		Vector away = me->GetAbsOrigin() - closestEnemy->GetAbsOrigin();

		me->GetLocomotionInterface()->Approach( me->GetLocomotionInterface()->GetFeet() + away );
	}
}


//-----------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotTacticalMonitor::Update( CFFBot *me, float interval )
{
	if ( FFGameRules()->RoundHasBeenWon() )
	{
		if ( FFGameRules()->GetWinningTeam() == me->GetTeamNumber() )
		{
			// we won - kill all losers we see
			return SuspendFor( new CFFBotSeekAndDestroy, "Get the losers!" );
		}
		else
		{
			// lost - run and hide
			if ( me->GetVisionInterface()->GetPrimaryKnownThreat( true ) )
			{
				return SuspendFor( new CFFBotRetreatToCover, "Run away from threat!" );
			}

			me->PressCrouchButton();
		}

		return Continue();
	}

	if ( ff_bot_force_jump.GetBool() )
	{
		if ( !me->GetLocomotionInterface()->IsClimbingOrJumping() )
		{
			me->GetLocomotionInterface()->Jump();
		}
	}

	if ( FFGameRules()->State_Get() == GR_STATE_PREROUND )
	{
		// clear stuck monitor so we dont jump when the preround elapses
		me->GetLocomotionInterface()->ClearStuckStatus( "In preround" );
	}

	Action< CFFBot > *result = me->OpportunisticallyUseWeaponAbilities();
	if ( result )
	{
		return SuspendFor( result, "Opportunistically using buff item" );
	}

	if ( FFGameRules()->InSetup() )
	{
		// if a human is staring at us, face them and taunt
		if ( m_acknowledgeRetryTimer.IsElapsed() )
		{
			CFFPlayer *watcher = me->GetClosestHumanLookingAtMe();
			if ( watcher )
			{
				if ( !m_attentionTimer.HasStarted() )
					m_attentionTimer.Start( 0.5f );

				if ( m_attentionTimer.HasStarted() && m_attentionTimer.IsElapsed() )
				{
					// a human has been staring at us - acknowledge them
					if ( !m_acknowledgeAttentionTimer.HasStarted() )
					{
						// look toward them
						me->GetBodyInterface()->AimHeadTowards( watcher, IBody::IMPORTANT, 3.0f, NULL, "Acknowledging friendly human attention" );
						m_acknowledgeAttentionTimer.Start( RandomFloat( 0.0f, 2.0f ) );
					}
					else if ( m_acknowledgeAttentionTimer.IsElapsed() )
					{
						m_acknowledgeAttentionTimer.Invalidate();

						// don't ack again for awhile
						m_acknowledgeRetryTimer.Start( RandomFloat( 10.0f, 20.0f ) );

						return SuspendFor( new CFFBotTaunt, "Acknowledging friendly human attention" );
					}
				}
			}
			else
			{
				// no-one is looking at me
				m_attentionTimer.Invalidate();
			}
		}
	}

	// check if we need to get to cover
	QueryResultType shouldRetreat = me->GetIntentionInterface()->ShouldRetreat( me );


	if ( shouldRetreat == ANSWER_YES )
	{
		return SuspendFor( new CFFBotRetreatToCover, "Backing off" );
	}
	else if ( shouldRetreat != ANSWER_NO )
	{
		// retreat if we need to do a full reload (ie: soldiers shot all their rockets)
		if ( !me->m_Shared.InCond( TF_COND_INVULNERABLE ) )
		{
			if ( me->IsDifficulty( CFFBot::HARD ) || me->IsDifficulty( CFFBot::EXPERT ) )
			{
				CFFWeaponBase *myPrimary = (CFFWeaponBase *)me->Weapon_GetSlot( TF_WPN_TYPE_PRIMARY );
				if ( myPrimary && me->GetAmmoCount( TF_AMMO_PRIMARY ) > 0 && me->IsBarrageAndReloadWeapon( myPrimary ) )
				{
					if ( myPrimary->Clip1() <= 1 )
					{
						return SuspendFor( new CFFBotRetreatToCover, "Moving to cover to reload" );
					}
				}
			}
		}
	}

	bool isAvailable = ( me->GetIntentionInterface()->ShouldHurry( me ) != ANSWER_YES );


	// collect ammo and health kits, unless we're in a big hurry
	if ( isAvailable && m_maintainTimer.IsElapsed() )
	{
		m_maintainTimer.Start( RandomFloat( 0.3f, 0.5f ) );

		bool isHurt = false;

		if ( me->IsInCombat() || me->IsPlayerClass( CLASS_SNIPER ) )
		{
			// stay in the fight until we're nearly dead
			isHurt = ( (float)me->GetHealth() / (float)me->GetMaxHealth() ) < ff_bot_health_critical_ratio.GetFloat();
		}
		else
		{
			isHurt = me->m_Shared.InCond( TF_COND_BURNING ) || ( (float)me->GetHealth() / (float)me->GetMaxHealth() ) < ff_bot_health_ok_ratio.GetFloat();
		}

		if ( isHurt && CFFBotGetHealth::IsPossible( me ) )
		{
			return SuspendFor( new CFFBotGetHealth, "Grabbing nearby health" );
		}

		if ( me->IsAmmoLow() && CFFBotGetAmmo::IsPossible( me ) )
		{
			return SuspendFor( new CFFBotGetAmmo, "Grabbing nearby ammo" );
		}

		bool shouldDestroySentries = true;


		// destroy enemy sentry guns we've encountered
		if ( shouldDestroySentries && me->GetEnemySentry() && CFFBotDestroyEnemySentry::IsPossible( me ) )
		{
			return SuspendFor( new CFFBotDestroyEnemySentry, "Going after an enemy sentry to destroy it" );
		}
	}

	// opportunistically use nearby teleporters
	if ( ShouldOpportunisticallyTeleport( me ) )
	{
		CObjectTeleporter *nearbyTeleporter = FindNearbyTeleporter( me );
		if ( nearbyTeleporter )
		{
			CTFNavArea *teleporterArea = (CTFNavArea *)TheTFNavMesh()->GetNearestNavArea( nearbyTeleporter );
			CTFNavArea *myArea = (CTFNavArea *)me->GetLastKnownArea();

			// only use teleporter if it is ahead of us
			if ( teleporterArea && myArea && myArea->GetIncursionDistance( me->GetTeamNumber() ) < 350.0f + teleporterArea->GetIncursionDistance( me->GetTeamNumber() ) )
			{
				return SuspendFor( new CFFBotUseTeleporter( nearbyTeleporter ), "Using nearby teleporter" );
			}
		}
	}

	// detonate sticky bomb traps when victims are near
	MonitorArmedStickyBombs( me );

	// if we're a Spy, avoid bumping into enemies and giving ourselves away
	if ( me->IsPlayerClass( CLASS_SPY ) )
	{
		AvoidBumpingEnemies( me );
	}

	me->UpdateDelayedThreatNotices();

	// if I'm a squad leader, wait for out of position squadmates
	if ( me->IsInASquad() && me->GetSquad()->IsLeader( me ) && me->GetSquad()->ShouldSquadLeaderWaitForFormation() )
	{
		return SuspendFor( new CFFBotWaitForOutOfPositionSquadMember, "Waiting for squadmates to get back into formation" );
	}


	return Continue();
}


//-----------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotTacticalMonitor::OnOtherKilled( CFFBot *me, CBaseCombatCharacter *victim, const CTakeDamageInfo &info )
{
	return TryContinue();
}


//-----------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotTacticalMonitor::OnNavAreaChanged( CFFBot *me, CNavArea *newArea, CNavArea *oldArea )
{
	// does the area we are entering have a prerequisite?
	if ( newArea && newArea->HasPrerequisite( me ) && !me->HasAttribute( CFFBot::AGGRESSIVE ) )
	{
		const CUtlVector< CHandle< CFuncNavPrerequisite > > &prereqVector = newArea->GetPrerequisiteVector();

		for( int i=0; i<prereqVector.Count(); ++i )
		{
			const CFuncNavPrerequisite *prereq = prereqVector[i];
			if ( prereq && prereq->IsEnabled() && const_cast< CFuncNavPrerequisite * >( prereq )->PassesTriggerFilters( me ) )
			{
				// this prerequisite applies to me
				if ( prereq->IsTask( CFuncNavPrerequisite::TASK_WAIT ) )
				{
					return TrySuspendFor( new CFFBotNavEntWait( prereq ), RESULT_IMPORTANT, "Prerequisite commands me to wait" );
				}
				else if ( prereq->IsTask( CFuncNavPrerequisite::TASK_MOVE_TO_ENTITY ) )
				{
					return TrySuspendFor( new CFFBotNavEntMoveTo( prereq ), RESULT_IMPORTANT, "Prerequisite commands me to move to an entity" );
				}
			}
		}
	}


	return TryContinue();
}

//-----------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotTacticalMonitor::OnCommandString( CFFBot *me, const char *command )
{
	if ( FStrEq( command, "goto action point" ) )
	{
		return TrySuspendFor( new CTFGotoActionPoint(), RESULT_IMPORTANT, "Received command to go to action point" );
	}
	else if ( FStrEq( command, "despawn" ) )
	{
		return TrySuspendFor( new CTFDespawn(), RESULT_CRITICAL, "Received command to go to de-spawn" );
	}
	else if ( FStrEq( command, "taunt" ) )
	{
		return TrySuspendFor( new CFFBotTaunt(), RESULT_TRY, "Received command to taunt" );
	}
	else if ( FStrEq( command, "build sentry at nearest sentry hint" ) )
	{
		if ( me->IsPlayerClass( CLASS_ENGINEER ) )
		{
			CFFBotHintSentrygun *bestSentryHint = NULL;
			float bestDist2 = FLT_MAX;
			CFFBotHintSentrygun *sentryHint;
			for( sentryHint = static_cast< CFFBotHintSentrygun * >( gEntList.FindEntityByClassname( NULL, "bot_hint_sentrygun" ) );
				 sentryHint;
				 sentryHint = static_cast< CFFBotHintSentrygun * >( gEntList.FindEntityByClassname( sentryHint, "bot_hint_sentrygun" ) ) )
			{
				// clear the previous owner if it is us
				if ( sentryHint->GetPlayerOwner() == me )
				{
					sentryHint->SetPlayerOwner( NULL );
				}
				if ( sentryHint->IsAvailableForSelection( me ) )
				{
					Vector toMe = me->GetAbsOrigin() - sentryHint->GetAbsOrigin();
					float dist2 = toMe.LengthSqr();
					if ( dist2 < bestDist2 )
					{
						bestSentryHint = sentryHint;
						bestDist2 = dist2;
					}
				}
			}
			if ( bestSentryHint )
			{
				bestSentryHint->SetPlayerOwner( me );
				return TrySuspendFor( new CFFBotEngineerBuilding( bestSentryHint ), RESULT_CRITICAL, "Building a Sentry at a hint location" );
			}			
		}
	}
	else if ( FStrEq( command, "attack sentry at next action point" ) )
	{
		return TrySuspendFor( new CTFTrainingAttackSentryActionPoint(), RESULT_CRITICAL, "Received command to attack sentry gun at next action point" );
	}
#ifdef STAGING_ONLY
	// !!! BountyMode prototype evaluation hacks below - this code will most likely be deleted soon
#endif // STAGING_ONLY

	return TryContinue();
}


//-----------------------------------------------------------------------------------------
bool CFFBotTacticalMonitor::ShouldOpportunisticallyTeleport( CFFBot *me ) const
{
	// if I'm an engineer who hasn't placed his teleport entrance yet, don't use friend's teleporter
	if ( me->IsPlayerClass( CLASS_ENGINEER ) )
	{
		CBaseObject *teleporterEntrance = me->GetObjectOfType( OBJ_TELEPORTER, MODE_TELEPORTER_ENTRANCE );

		return ( teleporterEntrance != NULL );
	}

	// Medics don't automatically take teleporters unless they actively decide to follow their patient through
	if ( me->IsPlayerClass( CLASS_MEDIC ) )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------
CObjectTeleporter *CFFBotTacticalMonitor::FindNearbyTeleporter( CFFBot *me )
{
	if ( !m_findTeleporterTimer.IsElapsed() )
	{
		return NULL;
	}

	m_findTeleporterTimer.Start( RandomFloat( 1.0f, 2.0f ) );

	CTFNavArea *myArea = (CTFNavArea *)me->GetLastKnownArea();
	if ( myArea == NULL )
	{
		return NULL;
	}

	CUtlVector< CNavArea * > nearbyAreaVector;
	CUtlVector< CBaseObject * > objVector;
	CUtlVector< CObjectTeleporter * > nearbyTeleporterEntranceVector;

	CollectSurroundingAreas( &nearbyAreaVector, myArea, 1000.0f );
	TheTFNavMesh()->CollectBuiltObjects( &objVector, me->GetTeamNumber() );

	for( int j=0; j<objVector.Count(); ++j )
	{
		if ( objVector[j]->GetType() == OBJ_TELEPORTER )
		{
			CObjectTeleporter *teleporter = (CObjectTeleporter *)objVector[j];

			teleporter->UpdateLastKnownArea();

			CNavArea *teleporterArea = teleporter->GetLastKnownArea();

			if ( teleporter->IsEntrance() && teleporter->IsReady() && teleporterArea )
			{
				// we've found a functional teleporter entrance - is it in our nearby area set?
				for( int i=0; i<nearbyAreaVector.Count(); ++i )
				{
					CNavArea *nearbyArea = nearbyAreaVector[i];

					if ( nearbyArea->GetID() == teleporterArea->GetID() )
					{
						// yes, it's nearby
						nearbyTeleporterEntranceVector.AddToTail( teleporter );
						break;
					}
				}
			}
		}
	}

	if ( nearbyTeleporterEntranceVector.Count() > 0 )
	{
		int which = RandomInt( 0, nearbyTeleporterEntranceVector.Count()-1 );

		return nearbyTeleporterEntranceVector[ which ];
	}

	return NULL;
}
