//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_destroy_enemy_sentry.h
// Destroy an enemy sentry gun
// Michael Booth, June 2010

#ifndef FF_BOT_DESTROY_ENEMY_SENTRY_H
#define FF_BOT_DESTROY_ENEMY_SENTRY_H

#include "Path/NextBotChasePath.h"

//---------------------------------------------------------------------------------
class CFFBotDestroyEnemySentry : public Action< CFFBot >
{
public:
	static bool IsPossible( CFFBot *me );			// return true if this Action has what it needs to perform right now

	virtual ActionResult< CFFBot >	OnStart( CFFBot *me, Action< CFFBot > *priorAction );
	virtual ActionResult< CFFBot >	Update( CFFBot *me, float interval );

	virtual ActionResult< CFFBot >	OnResume( CFFBot *me, Action< CFFBot > *interruptingAction );

	virtual QueryResultType ShouldHurry( const INextBot *me ) const;					// are we in a hurry?
	virtual QueryResultType	ShouldRetreat( const INextBot *me ) const;					// is it time to retreat?
	virtual QueryResultType	ShouldAttack( const INextBot *me, const CKnownEntity *them ) const;	// should we attack "them"?

	virtual const char *GetName( void ) const	{ return "DestroyEnemySentry"; };

private:
	PathFollower m_path;
	CountdownTimer m_repathTimer;

	bool m_canMove;


	Vector m_safeAttackSpot;
	bool m_hasSafeAttackSpot;
	void ComputeSafeAttackSpot( CFFBot *me );
	void ComputeCornerAttackSpot( CFFBot *me );

	bool m_isAttackingSentry;
	bool m_wasUber;

	ActionResult< CFFBot > EquipLongRangeWeapon( CFFBot *me );

	CHandle< CObjectSentrygun > m_targetSentry;
};


//---------------------------------------------------------------------------------
class CFFBotUberAttackEnemySentry : public Action< CFFBot >
{
public:
	CFFBotUberAttackEnemySentry( CObjectSentrygun *sentryTarget );
	virtual ~CFFBotUberAttackEnemySentry() { }

	virtual ActionResult< CFFBot >	OnStart( CFFBot *me, Action< CFFBot > *priorAction );
	virtual ActionResult< CFFBot >	Update( CFFBot *me, float interval );
	virtual void					OnEnd( CFFBot *me, Action< CFFBot > *nextAction );

	virtual QueryResultType ShouldHurry( const INextBot *me ) const;					// are we in a hurry?
	virtual QueryResultType	ShouldRetreat( const INextBot *me ) const;					// is it time to retreat?
	virtual QueryResultType	ShouldAttack( const INextBot *me, const CKnownEntity *them ) const;	// should we attack "them"?

	virtual const char *GetName( void ) const	{ return "UberAttackEnemySentry"; };

private:
	bool m_wasIgnoringEnemies;

	PathFollower m_path;
	CountdownTimer m_repathTimer;

	CHandle< CObjectSentrygun > m_targetSentry;
};


#endif // FF_BOT_DESTROY_ENEMY_SENTRY_H
