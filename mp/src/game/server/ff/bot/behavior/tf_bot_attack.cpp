//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_attack.cpp
// Attack our threat
// Michael Booth, February 2009

#include "cbase.h"
#include "ff_player.h"
#include "ff_gamerules.h"
#include "team_control_point_master.h"
#include "bot/ff_bot.h"
#include "bot/behavior/ff_bot_attack.h"

#include "nav_mesh.h"

extern ConVar ff_bot_path_lookahead_range;
extern ConVar ff_bot_offense_must_push_time;


//---------------------------------------------------------------------------------------------
CFFBotAttack::CFFBotAttack( void ) : m_chasePath( ChasePath::LEAD_SUBJECT )
{
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotAttack::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_path.SetMinLookAheadDistance( me->GetDesiredPathLookAheadRange() );

	return Continue();
}


//---------------------------------------------------------------------------------------------
// head aiming and weapon firing is handled elsewhere - we just need to get into position to fight
ActionResult< CFFBot >	CFFBotAttack::Update( CFFBot *me, float interval )
{
	CFFWeaponBase *myWeapon = me->GetActiveFFWeapon();
	bool isUsingCloseRangeWeapon = ( myWeapon && ( myWeapon->IsWeapon( FF_WEAPON_FLAMETHROWER ) || myWeapon->IsMeleeWeapon() ) );

	const CKnownEntity *threat = me->GetVisionInterface()->GetPrimaryKnownThreat();
	if ( threat == NULL || threat->IsObsolete() || !me->GetIntentionInterface()->ShouldAttack( me, threat ) )
	{
		return Done( "No threat" );
	}

	me->EquipBestWeaponForThreat( threat );

	if ( isUsingCloseRangeWeapon && threat->IsVisibleRecently() && me->IsRangeLessThan( threat->GetLastKnownPosition(), 1.1f * me->GetDesiredAttackRange() ) )
	{
		// circle around our victim
		if ( me->TransientlyConsistentRandomValue( 3.0f ) < 0.5f )
		{
			me->PressLeftButton();
		}
		else
		{
			me->PressRightButton();
		}
	}


	// pursue the threat. if not visible, go to the last known position
	if ( !threat->IsVisibleRecently() || 
		 me->IsRangeGreaterThan( threat->GetEntity()->GetAbsOrigin(), me->GetDesiredAttackRange() ) || 
		 !me->IsLineOfFireClear( threat->GetEntity()->EyePosition() ) )
	{
		if ( threat->IsVisibleRecently() )
		{
			if ( isUsingCloseRangeWeapon )	
			{
				CFFBotPathCost cost( me, SAFEST_ROUTE );
				m_chasePath.Update( me, threat->GetEntity(), cost );
			}
			else
			{
				CFFBotPathCost cost( me, DEFAULT_ROUTE );
				m_chasePath.Update( me, threat->GetEntity(), cost );
			}
		}
		else
		{
			// if we're at the threat's last known position and he's still not visible, we lost him
			m_chasePath.Invalidate();

			if ( me->IsRangeLessThan( threat->GetLastKnownPosition(), 20.0f ) )
			{
				me->GetVisionInterface()->ForgetEntity( threat->GetEntity() );
				return Done( "I lost my target!" );
			}

			// look where we last saw him as we approach
			if ( me->IsRangeLessThan( threat->GetLastKnownPosition(), me->GetMaxAttackRange() ) )
			{
				me->GetBodyInterface()->AimHeadTowards( threat->GetLastKnownPosition() + Vector( 0, 0, HumanEyeHeight ), IBody::IMPORTANT, 0.2f, NULL, "Looking towards where we lost sight of our victim" );
			}

			m_path.Update( me );

			if ( m_repathTimer.IsElapsed() )
			{
				//m_repathTimer.Start( RandomFloat( 0.3f, 0.5f ) );
				m_repathTimer.Start( RandomFloat( 3.0f, 5.0f ) );

				if ( isUsingCloseRangeWeapon )	
				{
					CFFBotPathCost cost( me, SAFEST_ROUTE );
					m_path.Compute( me, threat->GetLastKnownPosition(), cost );
				}
				else
				{
					CFFBotPathCost cost( me, DEFAULT_ROUTE );
					float maxPathLength = 0.0f;
					m_path.Compute( me, threat->GetLastKnownPosition(), cost, maxPathLength );
				}
			}
		}
	}

	return Continue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotAttack::OnStuck( CFFBot *me )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotAttack::OnMoveToSuccess( CFFBot *me, const Path *path )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
EventDesiredResult< CFFBot > CFFBotAttack::OnMoveToFailure( CFFBot *me, const Path *path, MoveToFailureType reason )
{
	return TryContinue();
}


//---------------------------------------------------------------------------------------------
QueryResultType	CFFBotAttack::ShouldRetreat( const INextBot *me ) const
{
	return ANSWER_UNDEFINED;
}


//---------------------------------------------------------------------------------------------
QueryResultType CFFBotAttack::ShouldHurry( const INextBot *me ) const
{
	return ANSWER_UNDEFINED;
}

