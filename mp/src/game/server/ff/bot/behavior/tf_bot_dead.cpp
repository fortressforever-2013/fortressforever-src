//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_dead.cpp
// Push up daisies
// Michael Booth, May 2009

#include "cbase.h"
#include "ff_player.h"
#include "ff_gamerules.h"
#include "bot/ff_bot.h"
#include "bot/behavior/ff_bot_dead.h"
#include "bot/behavior/ff_bot_behavior.h"

#include "nav_mesh.h"


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotDead::OnStart( CFFBot *me, Action< CFFBot > *priorAction )
{
	m_deadTimer.Start();

	return Continue();
}


//---------------------------------------------------------------------------------------------
ActionResult< CFFBot >	CFFBotDead::Update( CFFBot *me, float interval )
{
	if ( me->IsAlive() )
	{
		// how did this happen?
		return ChangeTo( new CFFBotMainAction, "This should not happen!" );
	}

	if ( m_deadTimer.IsGreaterThen( 5.0f ) )
	{
		if ( me->HasAttribute( CFFBot::REMOVE_ON_DEATH ) )
		{
			// remove dead bots
			engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", me->GetUserID() ) );
		}
		else if ( me->HasAttribute( CFFBot::BECOME_SPECTATOR_ON_DEATH ) )
		{
			me->ChangeTeam( TEAM_SPECTATOR, false, true );
			return Done();
		}
	}


	return Continue();
}

