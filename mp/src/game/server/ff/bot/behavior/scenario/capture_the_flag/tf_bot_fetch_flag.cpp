//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_fetch_flag.cpp
// Go get the flag!
// Michael Booth, May 2011

#include "cbase.h"

#include "bot/ff_bot.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_fetch_flag.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_escort_flag_carrier.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_attack_flag_defenders.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_deliver_flag.h"


//---------------------------------------------------------------------------------------------
CFFBotFetchFlag::CFFBotFetchFlag( bool isTemporary )
{
	m_isTemporary = isTemporary;
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotFetchFlag::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot > CFFBotFetchFlag::Update( CFFBot *me, float interval )
{
	CCaptureFlag *flag = me->GetFlagToFetch();

       if ( !flag )
       {
               return Done( "No flag" );
       }





	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat )
	{
		me->EquipBestWeaponForThreat( threat );
	}

	CFFPlayer *carrier = ToFFPlayer( flag->GetOwnerEntity() );
	if ( carrier )
	{
		if ( m_isTemporary )
		{
			return Done( "Someone else picked up the flag" );
		}

		// NOTE: if I've picked up the flag, the ScenarioMonitor will handle it
		return SuspendFor( new CFFBotAttackFlagDefenders, "Someone has the flag - attacking the enemy defenders" );
	}

	// go pick up the flag
	if ( m_repathTimer.IsElapsed() )
	{
		CFFBotPathCost cost( me, DEFAULT_ROUTE );
               float maxPathLength = 0.0f;
               if ( m_path.Compute( me, flag->WorldSpaceCenter(), cost, maxPathLength ) == false )
		{
			if ( flag->IsDropped() )
			{
				// flag is unreachable - attack for awhile and hope someone else can dislodge it
				return SuspendFor( new CFFBotAttackFlagDefenders( RandomFloat( 5.0f, 10.0f ) ), "Flag unreachable - Attacking" );

				// just give it to me
				// flag->PickUp( me, true );
			}
		}

		m_repathTimer.Start( RandomFloat( 1.0f, 2.0f ) );
	}

	m_path.Update( me );

	return Continue();
}


//---------------------------------------------------------------------------------------------
// are we in a hurry?
QueryResultType CFFBotFetchFlag::ShouldHurry( const INextBot *me ) const
{
	return ANSWER_YES;
}


//---------------------------------------------------------------------------------------------
// is it time to retreat?
QueryResultType	CFFBotFetchFlag::ShouldRetreat( const INextBot *me ) const
{
	return ANSWER_NO;
}
