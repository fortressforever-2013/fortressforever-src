//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_mission_destroy_sentries.cpp
// Seek and destroy enemy sentries and ignore everything else
// Michael Booth, June 2011

#include "cbase.h"
#include "team.h"
#include "bot/ff_bot.h"
#include "bot/behavior/missions/ff_bot_mission_destroy_sentries.h"
#include "bot/behavior/spy/ff_bot_spy_sap.h"
#include "bot/behavior/ff_bot_destroy_enemy_sentry.h"
#include "bot/behavior/medic/ff_bot_medic_heal.h"
#include "ff_obj_sentrygun.h"

//
// NOTE: This behavior is deprecated and unused for now.
// The only sentry destroying mission is the Sentry Buster right now (suicide bomber).
//

//---------------------------------------------------------------------------------------------
CFFBotMissionDestroySentries::CFFBotMissionDestroySentries( CObjectSentrygun *goalSentry )
{
	m_goalSentry = goalSentry;
}


//---------------------------------------------------------------------------------------------
CObjectSentrygun *CFFBotMissionDestroySentries::SelectSentryTarget( CFFBot *me )
{
	
	return NULL;
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotMissionDestroySentries::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	if ( me->IsPlayerClass( CLASS_MEDIC ) )
	{
		return ChangeTo( new CFFBotMedicHeal, "My job is to heal/uber the others in the mission" );
	}

	// focus only on the mission
	me->SetAttribute( CFFBot::IGNORE_ENEMIES );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot > CFFBotMissionDestroySentries::Update( CFFBot *me, float interval )
{
	if ( m_goalSentry == NULL )
	{
		// first destroy the sentry we were assigned to, or any sentry we discovered or that is attacking us
		m_goalSentry = me->GetEnemySentry();

		if ( m_goalSentry == NULL )
		{
			// next destroy the most dangerous sentry
 			int iTeam = ( me->GetTeamNumber() == FF_TEAM_RED ) ? FF_TEAM_BLUE : FF_TEAM_RED;

			if ( FFGameRules() && FFGameRules()->IsPVEModeActive() )
			{
				iTeam = FF_TEAM_PVE_DEFENDERS;
			}

			m_goalSentry = FFGameRules()->FindSentryGunWithMostKills( iTeam );
		}
	}



	if ( m_goalSentry == NULL )
	{
		// no sentries left to destroy - our mission is complete
		me->SetMission( CFFBot::NO_MISSION, MISSION_DOESNT_RESET_BEHAVIOR_SYSTEM );
		return ChangeTo( GetParentAction()->InitialContainedAction( me ), "Mission complete - reverting to normal behavior" );
	}

	if ( m_goalSentry != me->GetEnemySentry() )
	{
		me->RememberEnemySentry( m_goalSentry, m_goalSentry->WorldSpaceCenter() );
	}

	if ( me->IsPlayerClass( CLASS_SPY ) )
	{
		return SuspendFor( new CFFBotSpySap( m_goalSentry ), "On a mission to sap a sentry" );
	}

	return SuspendFor( new CFFBotDestroyEnemySentry, "On a mission to destroy a sentry" );
}


//---------------------------------------------------------------------------------------------
void CFFBotMissionDestroySentries::OnEnd( CFFBot *me, Action< CFFBot > *nextAction )
{
	me->ClearAttribute( CFFBot::IGNORE_ENEMIES );
}
