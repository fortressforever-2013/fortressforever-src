//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_deliver_flag.cpp
// Take the flag we are holding to its destination
// Michael Booth, May 2011

#include "cbase.h"

#include "ff_player_shared.h"

#include "bot/ff_bot.h"
#include "bot/behavior/scenario/capture_the_flag/ff_bot_deliver_flag.h"
#include "bot/behavior/ff_bot_taunt.h"

#include "tf_objective_resource.h"
#include "tf_gamestats.h"

#include "bot/behavior/nav_entities/ff_bot_nav_ent_move_to.h"
#include "bot/behavior/nav_entities/ff_bot_nav_ent_wait.h"

#include "particle_parse.h"



//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotDeliverFlag::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_flTotalTravelDistance = -1.0f;

	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );



	return Continue();
}


//---------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
CFFBotPushToCapturePoint::CFFBotPushToCapturePoint( Action< CFFBot > *nextAction )
{
	m_nextAction = nextAction;
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot > CFFBotPushToCapturePoint::Update( CFFBot *me, float interval )
{
	// flag collection and delivery is handled by our parent behavior, ScenarioMonitor

	CCaptureZone *zone = me->GetFlagCaptureZone();

	if ( !zone )
	{
		if ( m_nextAction )
		{
			return ChangeTo( m_nextAction, "No flag capture zone exists!" );
		}

		return Done( "No flag capture zone exists!" );
	}

	Vector toZone = zone->WorldSpaceCenter() - me->GetAbsOrigin();
	if ( toZone.AsVector2D().IsLengthLessThan( 50.0f ) )
	{
		if ( m_nextAction )
		{
			return ChangeTo( m_nextAction, "At destination" );
		}

		return Done( "At destination" );
	}

	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat && threat->IsVisibleRecently() )
	{
		// prepare to fight
		me->EquipBestWeaponForThreat( threat );
	}

	if ( m_repathTimer.IsElapsed() )
	{
		CFFBotPathCost cost( me, FASTEST_ROUTE );
		m_path.Compute( me, zone->WorldSpaceCenter(), cost );

		m_repathTimer.Start( RandomFloat( 1.0f, 2.0f ) );
	}

	m_path.Update( me );

	return Continue();
}

//-----------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotPushToCapturePoint::OnNavAreaChanged( CFFBot *me, CNavArea *newArea, CNavArea *oldArea )
{
	// does the area we are entering have a prerequisite?
	if ( newArea && newArea->HasPrerequisite( me ) )
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
