//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_deliver_flag.h
// Take the flag we are holding to its destination
// Michael Booth, May 2011

#ifndef FF_BOT_DELIVER_FLAG_H
#define FF_BOT_DELIVER_FLAG_H

#include "Path/NextBotPathFollow.h"


//-----------------------------------------------------------------------------
class CFFBotDeliverFlag : public Action< CFFBot >
{
public:
	virtual ActionResult< CFFBot >	OnStart( CFFBot *me, Action< CFFBot > *priorAction );
	virtual ActionResult< CFFBot >	Update( CFFBot *me, float interval );
	virtual void					OnEnd( CFFBot *me, Action< CFFBot > *nextAction );

	virtual QueryResultType ShouldAttack( const INextBot *me, const CKnownEntity *them ) const;
	virtual QueryResultType ShouldHurry( const INextBot *me ) const;
	virtual QueryResultType	ShouldRetreat( const INextBot *me ) const;

	virtual EventDesiredResult< CFFBot > OnContact( CFFBot *me, CBaseEntity *other, CGameTrace *result = NULL );

	virtual const char *GetName( void ) const	{ return "DeliverFlag"; };

private:
	PathFollower m_path;
	CountdownTimer m_repathTimer;
	float m_flTotalTravelDistance;

       CountdownTimer m_buffPulseTimer;
};


//-----------------------------------------------------------------------------
class CFFBotPushToCapturePoint : public Action< CFFBot >
{
public:
	CFFBotPushToCapturePoint( Action< CFFBot > *nextAction = NULL );
	virtual ~CFFBotPushToCapturePoint() { }

	virtual ActionResult< CFFBot >	Update( CFFBot *me, float interval );
	virtual EventDesiredResult< CFFBot > OnNavAreaChanged( CFFBot *me, CNavArea *newArea, CNavArea *oldArea );

	virtual const char *GetName( void ) const	{ return "PushToCapturePoint"; };

private:
	PathFollower m_path;
	CountdownTimer m_repathTimer;

	Action< CFFBot > *m_nextAction;
};


#endif // FF_BOT_DELIVER_FLAG_H
