//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_body.cpp
// Team Fortress NextBot body interface
// Michael Booth, May 2010

#include "cbase.h"

#include "ff_bot.h"
#include "ff_bot_body.h"


// 
// Return how often we should sample our target's position and 
// velocity to update our aim tracking, to allow realistic slop in tracking
//
float CFFBotBody::GetHeadAimTrackingInterval( void ) const
{
	CFFBot *me = (CFFBot *)GetBot();


	switch( me->GetDifficulty() )
	{
	case CFFBot::EXPERT:
		return 0.05f;

	case CFFBot::HARD:
		return 0.1f;

	case CFFBot::NORMAL:
		return 0.25f;

	case CFFBot::EASY:
		return 1.0f;
	}

	return 0.0f;
}
