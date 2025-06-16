//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_scenario_monitor.h
// Behavior layer that interrupts for scenario rules (picked up flag, drop what you're doing and capture, etc)
// Michael Booth, May 2011

#include "cbase.h"
#include "fmtstr.h"

#include "ff_gamerules.h"
#include "ff_weapon_pipebomblauncher.h"
#include "NextBot/NavMeshEntities/func_nav_prerequisite.h"

#include "bot/ff_bot.h"
#include "bot/ff_bot_manager.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_destroy_entity.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_move_to.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_wait.h"
#include "bot/behavior/ff_bot_tactical_monitor.h"
#include "bot/behavior/ff_bot_retreat_to_cover.h"
#include "bot/behavior/ff_bot_get_health.h"
#include "bot/behavior/ff_bot_get_ammo.h"
#include "bot/behavior/sniper/ff_bot_sniper_lurk.h"
#include "bot/behavior/scenario/capture_point/ff_bot_capture_point.h"
#include "bot/behavior/scenario/capture_point/ff_bot_defend_point.h"
#include "bot/behavior/ff_bot_use_teleporter.h"
#include "bot/behavior/training/ff_bot_training.h"
#include "bot/behavior/ff_bot_destroy_enemy_sentry.h"
#include "bot/behavior/engineer/ff_bot_engineer_building.h"
#include "bot/behavior/spy/ff_bot_spy_infiltrate.h"
#include "bot/behavior/spy/ff_bot_spy_leave_spawn_room.h"
#include "bot/behavior/medic/ff_bot_medic_heal.h"
#include "bot/behavior/engineer/ff_bot_engineer_build.h"
#include "bot/behavior/missions/ff_bot_mission_destroy_sentries.h"
#include "bot/map_entities/ff_bot_hint_sentrygun.h"


#include "bot/behavior/ff_bot_attack.h"
#include "bot/behavior/ff_bot_seek_and_destroy.h"
#include "bot/behavior/ff_bot_taunt.h"
#include "bot/behavior/ff_bot_escort.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_fetch_flag.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_deliver_flag.h"

#include "bot/behavior/squad/ff_bot_escort_squad_leader.h"

#include "bot/behavior/ff_bot_scenario_monitor.h"


extern ConVar ff_bot_health_ok_ratio;
extern ConVar ff_bot_health_critical_ratio;


//-----------------------------------------------------------------------------------------
// Returns the initial Action we will run concurrently as a child to us
Action< CFFBot > *CFFBotScenarioMonitor::InitialContainedAction( CFFBot *me )
{
	if ( me->IsInASquad() )
	{
		if ( me->GetSquad()->IsLeader( me ) )
		{
			// I'm the leader of this Squad, so I can do what I want and the other Squaddies will support me
			return DesiredScenarioAndClassAction( me );
		}

		// Medics are the exception - they always heal, and have special squad logic in their heal logic
		if ( me->IsPlayerClass( CLASS_MEDIC ) )
		{
			return new CFFBotMedicHeal;
		}

		// I'm in a Squad but not the leader, do "escort and support" Squad behavior
		// until the Squad disbands, and then do my normal thing
		return new CFFBotEscortSquadLeader( DesiredScenarioAndClassAction( me ) );
	}

	return DesiredScenarioAndClassAction( me );
}


//-----------------------------------------------------------------------------------------
// Returns Action specific to the scenario and my class
Action< CFFBot > *CFFBotScenarioMonitor::DesiredScenarioAndClassAction( CFFBot *me )
{
	switch( me->GetMission() )
	{
	case CFFBot::MISSION_SEEK_AND_DESTROY:
		break;

       case CFFBot::MISSION_DESTROY_SENTRIES:
               return new CFFBotMissionDestroySentries;

	case CFFBot::MISSION_SNIPER:
		return new CFFBotSniperLurk;

	}




	if ( me->IsPlayerClass( CLASS_SPY ) )
	{
		return new CFFBotSpyInfiltrate;
	}

	if ( !TheTFBots().IsMeleeOnly() )
	{
		if ( me->IsPlayerClass( CLASS_SNIPER ) )
		{
			return new CFFBotSniperLurk;
		}

		if ( me->IsPlayerClass( CLASS_MEDIC ) )
		{
			return new CFFBotMedicHeal;
		}

		if ( me->IsPlayerClass( CLASS_ENGINEER ) )
		{
			return new CFFBotEngineerBuild;
		}
	}

	if ( me->GetFlagToFetch() )
	{
		// capture the flag
		return new CFFBotFetchFlag;
	}
       else if ( FFGameRules()->GetGameType() == TF_GAMETYPE_CP )
       {
               // if we have a point we can capture - do it
		CUtlVector< CTeamControlPoint * > captureVector;
		FFGameRules()->CollectCapturePoints( me, &captureVector );

		if ( captureVector.Count() > 0 )
		{
			return new CFFBotCapturePoint;
		}

		// otherwise, defend our point(s) from capture
		CUtlVector< CTeamControlPoint * > defendVector;
		FFGameRules()->CollectDefendPoints( me, &defendVector );

		if ( defendVector.Count() > 0 )
		{
			return new CFFBotDefendPoint;
		}

		// likely KotH mode and/or all points are locked - assume capture
		DevMsg( "%3.2f: %s: Gametype is CP, but I can't find a point to capture or defend!\n", gpGlobals->curtime, me->GetDebugIdentifier() );
		return new CFFBotCapturePoint;
	}
	else
	{
		// scenario not implemented yet - just fight
		return new CFFBotSeekAndDestroy;
	}

	return NULL;
}


//-----------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotScenarioMonitor::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_ignoreLostFlagTimer.Start( 20.0f );
	m_lostFlagTimer.Invalidate();
	return Continue();
}


ConVar ff_bot_fetch_lost_flag_time( "ff_bot_fetch_lost_flag_time", "10", FCVAR_CHEAT, "How long busy TFBots will ignore the dropped flag before they give up what they are doing and go after it" );
ConVar ff_bot_flag_kill_on_touch( "ff_bot_flag_kill_on_touch", "0", FCVAR_CHEAT, "If nonzero, any bot that picks up the flag dies. For testing." );


//-----------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotScenarioMonitor::Update( CFFBot *me, float interval )
{
	// CTF Scenario
	if ( me->HasTheFlag() )
	{
		if ( ff_bot_flag_kill_on_touch.GetBool() )
		{
			me->CommitSuicide( false, true );
			return Done( "Flag kill" );
		}

		// we just picked up the flag - drop what we're doing and take it in
		return SuspendFor( new CFFBotDeliverFlag, "I've picked up the flag! Running it in..." );
	}

	if ( me->HasMission( CFFBot::NO_MISSION ) && m_ignoreLostFlagTimer.IsElapsed() && me->IsAllowedToPickUpFlag() )
	{
		CCaptureFlag *flag = me->GetFlagToFetch();

		if ( flag )
		{
			CFFPlayer *carrier = ToFFPlayer( flag->GetOwnerEntity() );
			if ( carrier )
			{
				m_lostFlagTimer.Invalidate();
			}
			else
			{
				// flag is loose
				if ( !m_lostFlagTimer.HasStarted() )
				{
					m_lostFlagTimer.Start( ff_bot_fetch_lost_flag_time.GetFloat() );
				}
				else if ( m_lostFlagTimer.IsElapsed() )
				{
					m_lostFlagTimer.Invalidate();

					// if we're a Medic an actively healing someone, don't interrupt
					if ( !me->MedicGetHealTarget() )
					{
						// we better go get the flag
						return SuspendFor( new CFFBotFetchFlag( TEMPORARY_FLAG_FETCH ), "Fetching lost flag..." );
					}
				}
			}
		}
	}

	return Continue();
}

