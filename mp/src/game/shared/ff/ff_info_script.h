// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_info_script.h
// @author Patrick O'Leary (Mulchman)
// @date 07/13/2006
// @brief info_ff_script, the C++ class
//
// REVISIONS
// ---------
// 07/13/2006, Mulchman: 
//		This file First created
//		Rewriting "ff_item_flag"
//		CFF_InfoScript name will change once complete

#ifndef FF_INFO_SCRIPT_H
#define FF_INFO_SCRIPT_H

#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
	#define CFFInfoScript C_FFInfoScript
	#include "c_baseanimating.h"
#elif GAME_DLL
	#include "baseanimating.h"
#endif

#define FLAG_MODEL "models/items/healthkit.mdl"

// An info_ff_script has 3 position states
enum FF_ScriptPosState_e
{
	PS_RETURNED = 0,
	PS_CARRIED = 1,
	PS_DROPPED = 2,
	PS_REMOVED = 3
};

// An info_ff_script has 3 goal states
enum FF_ScriptGoalState_e
{
	GS_ACTIVE = 0,
	GS_INACTIVE = 1,
	GS_REMOVED = 2
};

#ifdef GAME_DLL
	// for lua
	enum FF_AllowFlags
	{
		kAllowOnlyPlayers = 1 << 0,
		kAllowBlueTeam = 1 << 1,
		kAllowRedTeam = 1 << 2,
		kAllowYellowTeam = 1 << 3,
		kAllowGreenTeam = 1 << 4,
		kAllowScout = 1 << 5,
		kAllowSniper = 1 << 6,
		kAllowSoldier = 1 << 7,
		kAllowDemoman = 1 << 8,
		kAllowMedic = 1 << 9,
		kAllowHwguy = 1 << 10,
		kAllowPyro = 1 << 11,
		kAllowSpy = 1 << 12,
		kAllowEngineer = 1 << 13,
		kAllowCivilian = 1 << 14,
	};

	// Forward declaration
	class CFFInfoScriptAnimator;

	namespace luabridge
	{
		class LuaRef;
	}
#endif // GAME_DLL

//=============================================================================
//
// Class CFFInfoScript
//
//=============================================================================
class CFFInfoScript : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFInfoScript, CBaseAnimating );
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC();
	DECLARE_PREDICTABLE();

	// --> Shared
	CFFInfoScript( void );
	~CFFInfoScript( void );

	virtual void	Precache( void );
	virtual void	Spawn( void );

	virtual void	UpdateOnRemove( void );

	virtual Vector	EyePosition( void ) { return WorldSpaceCenter(); } // squeek: for spectating info scripts

	virtual Class_T	Classify( void ) { return CLASS_INFOSCRIPT; }

	virtual bool	IsPlayer( void ) { return false; }
	virtual bool	BlocksLOS( void ) { return false; }
	virtual bool	IsAlive( void ) { return false; }
	
	// An info_ff_script's position state
	virtual bool	IsCarried( void ) { return m_iPosState == PS_CARRIED; }
	virtual bool	IsDropped( void ) { return m_iPosState == PS_DROPPED; }
	virtual bool	IsReturned( void ) { return m_iPosState == PS_DROPPED; }

	// An info_ff_script's goal state
	virtual bool	IsActive( void ) { return m_iGoalState == GS_ACTIVE; }
	virtual bool	IsInactive( void ) { return m_iGoalState == GS_INACTIVE; }
#ifdef CLIENT_DLL
	virtual bool	IsRemoved( void ) { return ( m_iGoalState == GS_REMOVED ) || ( m_iPosState == PS_REMOVED ); }
#elif GAME_DLL
	virtual bool	IsRemoved( void ) { return ( m_iGoalState.Get() == GS_REMOVED ) || ( m_iPosState.Get() == PS_REMOVED ); }
#endif
	// <-- Shared

#ifdef CLIENT_DLL
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual bool	ShouldDraw( void );
	virtual int		DrawModel( int flags );
	virtual void	ClientThink( void );
	virtual ShadowType_t ShadowCastType( void );

	//virtual int		ShouldTransmit( const CCheckTransmitInfo *pInfo );
#elif GAME_DLL
	virtual int		ShouldTransmit(const CCheckTransmitInfo* pInfo);
	virtual int		UpdateTransmitState(void);

	CBaseEntity		*GetCarrier( void );
	CBaseEntity		*GetDropper( void );

	void			Pickup( CBaseEntity *pEntity );
	void			Drop( float delay, Vector pos, Vector velocity );
	void			Drop( float delay, float speed = 0.0f );
	void			Respawn( float delay );
	void			Return( void );
	void			ForceReturn( void );

	void			RemoveThink( void );

	void			SetSpawnFlags( int flags );

	bool			HasAnimations( void ) const { return m_bHasAnims; }

	// events
	void			OnTouch( CBaseEntity *pEntity );
	void			OnOwnerDied( CBaseEntity *pEntity );
	void			OnOwnerForceRespawn( CBaseEntity *pEntity );
	void			OnThink( void );
	void			OnRespawn( void );

	// Methods explicitly made for LUA
	void	LUA_Remove(void);
	void	LUA_Restore(void);

	// Phsyics can be enabled on this object which
	// makes certain functions not work!
	Vector			LUA_GetOrigin( void ) const;
	void			LUA_SetOrigin( const Vector& vecOrigin );
	QAngle			LUA_GetAngles( void ) const;
	void			LUA_SetAngles( const QAngle& vecAngles );

	void			LUA_SetModel( const char *szModel );
	const char *	LUA_GetModel();

	void			LUA_SetStartOrigin(const Vector& vecOrigin);
	Vector			LUA_GetStartOrigin() const;
	void			LUA_SetStartAngles(const QAngle& vecAngles);
	QAngle			LUA_GetStartAngles() const;

	
	// setters and getters for the criteria necessary for another entity to "touch" this entity
	void			SetTouchFlags( const luabridge::LuaRef& table );
	int				GetTouchFlags( void ) const { return m_allowTouchFlags; }

	// setters and getters for the criteria necessary for another entity to NOT "touch" this entity
	void			SetDisallowTouchFlags( const luabridge::LuaRef& table );
	int				GetDisallowTouchFlags( void ) const { return m_disallowTouchFlags; }

	// returns true if a specified is allowed to touch this entity
	bool			CanEntityTouch(CBaseEntity* pEntity);

	// bot info accessors
	int				GetBotTeamFlags() const { return m_BotTeamFlags; }
	int				GetBotGoalType() const { return m_BotGoalType; }

	void			SetBotGoalInfo(int _type);
	void			SpawnBot(const char* _name, int _team, int _class);

	virtual void	ResolveFlyCollisionCustom(trace_t& trace, Vector& vecVelocity);
	virtual void	PhysicsSimulate(void);
#endif // CLIENT_DLL

protected:
#ifdef GAME_DLL
	// Do not expose these to LUA!
	virtual void	SetActive( void );
	virtual void	SetInactive( void );
	virtual void	SetRemoved( void );
	virtual void	SetCarried( void );
	virtual void	SetReturned( void );
	virtual void	SetDropped( void );

	void MakeTouchable();
	bool CreateItemVPhysicsObject( void );

	void PlayDroppedAnim( void );
	void PlayCarriedAnim( void );
	void PlayReturnedAnim( void	);
	void InternalPlayAnim( Activity hActivity );

	bool m_atStart;

	bool m_bUsePhysics;

	bool m_bHasAnims;
	CFFInfoScriptAnimator* m_pAnimator;

	Vector m_vStartOrigin;
	QAngle m_vStartAngles;
	Vector	m_vecMins;
	Vector	m_vecMaxs;
	Vector	m_vecPhysicsMins;
	Vector	m_vecPhysicsMaxs;
	Vector	m_vecTouchMins;
	Vector	m_vecTouchMaxs;
	float	m_flHitboxBloat;

	CBaseEntity* m_pLastOwner;

	float m_flSpawnTime;
	float m_flReturnTime;
	bool m_bFloatActive;

	// indicates some criteria limiting what will
	// be allowed to "touch" this entity
	int		m_allowTouchFlags;
	int		m_disallowTouchFlags;

	// cached information for bot use
	int		m_BotTeamFlags;
	int		m_BotGoalType;
#endif // GAME_DLL

public:
#ifdef GAME_DLL
	CNetworkVar( float, m_flThrowTime );

	CNetworkVector( m_vecOffset );
	
	CNetworkVar( unsigned int, m_iHasModel );
	CNetworkVar( unsigned int, m_iShadow );

	CNetworkVar( int, m_iGoalState );
	CNetworkVar( int, m_iPosState );
#elif CLIENT_DLL
	float m_flThrowTime;

	Vector m_vecOffset;

	unsigned int m_iHasModel;
	unsigned int m_iShadow;

	int m_iGoalState;
	int m_iPosState;
#endif
};

#ifdef GAME_DLL
/////////////////////////////////////////////////////////////////////////////
// This is a cheap hack. Basically, this just calls
// StudioFrameAdvance() on m_pFFScript.
class CFFInfoScriptAnimator : public CBaseAnimating
{
public:
	DECLARE_CLASS(CFFInfoScriptAnimator, CBaseAnimating );
	DECLARE_DATADESC();

	CFFInfoScriptAnimator( void ) : m_pFFScript(NULL) {}
	~CFFInfoScriptAnimator( void ) {}

	virtual void Spawn(void)
	{
		SetThink( &CFFInfoScriptAnimator::OnThink );
		SetNextThink( gpGlobals->curtime );
	};

	void OnThink( void )
	{
		if( m_pFFScript )
		{
			if( m_pFFScript->HasAnimations() )
			{
				m_pFFScript->StudioFrameAdvance();
			}
		}

		SetNextThink( gpGlobals->curtime );
	};

	CFFInfoScript* m_pFFScript;
};
#endif // GAME_DLL

#endif // FF_INFO_SCRIPT_H
