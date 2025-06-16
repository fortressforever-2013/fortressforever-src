//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_attack_flag_defenders.cpp
// Attack enemies that are preventing the flag from reaching its destination
// Michael Booth, May 2011

#include "cbase.h"

#include "bot/ff_bot.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_attack_flag_defenders.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_escort_flag_carrier.h"

ConVar ff_bot_flag_escort_range( "ff_bot_flag_escort_range", "500", FCVAR_CHEAT );

extern ConVar ff_bot_flag_escort_max_count;

extern int GetBotEscortCount( int team );


//---------------------------------------------------------------------------------------------
CFFBotAttackFlagDefenders::CFFBotAttackFlagDefenders( float minDuration )
{
	if ( minDuration > 0.0f )
	{
		m_minDurationTimer.Start( minDuration );
	}
	else
	{
		m_minDurationTimer.Invalidate();
	}
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotAttackFlagDefenders::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	m_chasePlayer = NULL;
	return CFFBotAttack::OnStart( me, priorAction );
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot > CFFBotAttackFlagDefenders::Update( CFFBot *me, float interval )
{
	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat && threat->IsVisibleRecently() )
	{
		// prepare to fight
		me->EquipBestWeaponForThreat( threat );
	}

	if ( m_watchFlagTimer.IsElapsed() && m_minDurationTimer.IsElapsed() )
	{
		m_watchFlagTimer.Start( RandomFloat( 1.0f, 3.0f ) );

		CCaptureFlag *flag = me->GetFlagToFetch();

		if ( !flag )
		{
			return Done( "No flag" );
		}

		// can't reach flag if it is at home
               if ( !flag->IsHome() )
		{
			CFFPlayer *carrier = ToFFPlayer( flag->GetOwnerEntity() );
			if ( !carrier )
			{
				return Done( "Flag was dropped" );
			}

			if ( me->IsSelf( carrier ) )
			{
				return Done( "I picked up the flag!" );
			}

			// escort the flag carrier, unless the carrier is in a squad
			CFFBot *botCarrier = ToTFBot( carrier );
			if ( !botCarrier || !botCarrier->IsInASquad() )
			{
				if ( me->IsRangeLessThan( carrier, ff_bot_flag_escort_range.GetFloat() ) )
				{
					if ( GetBotEscortCount( me->GetTeamNumber() ) < ff_bot_flag_escort_max_count.GetInt() )
					{
						return ChangeTo( new CFFBotEscortFlagCarrier, "Near flag carrier - escorting" );
					}
				}
			}
		}
	}

	ActionResult< CFFBot > result = CFFBotAttack::Update( me, interval );

	if ( result.IsDone() )
	{
		// nothing to attack, move towards a random player

		if ( m_chasePlayer == NULL || !m_chasePlayer->IsAlive() )
		{
			m_chasePlayer = me->SelectRandomReachableEnemy();
		}

		if ( m_chasePlayer == NULL )
		{
			// everyone is dead or hiding in the spawn room - go escort the flag
			return ChangeTo( new CFFBotEscortFlagCarrier, "No reachable victim - escorting flag" );
		}

		// cheat and "see" our victim so we know where to go
		me->GetVisionInterface()->AddKnownEntity( m_chasePlayer );

		if ( m_repathTimer.IsElapsed() )
		{
			m_repathTimer.Start( RandomFloat( 1.0f, 3.0f ) );

                       CFFBotPathCost cost( me, DEFAULT_ROUTE );
                       float maxPathLength = 0.0f;
                       m_path.Compute( me, m_chasePlayer, cost, maxPathLength );
		}

		m_path.Update( me );
	}

	return Continue();
}
