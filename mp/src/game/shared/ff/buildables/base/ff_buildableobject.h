// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobject.cpp
// @author Patrick O'Leary (Mulchman)
// @date 12/15/2005
// @brief BuildableObject class
//
// REVISIONS
// ---------
// 12/15/2005, Mulchman: 
//		First created
//
// 12/23-25/2005, Mulchman: 
//		A bunch of modifications (explosions, gibs, fire, building checking)
//
// 12/28/2004, Mulchman:
//		Bunch of mods - shares network values correctly. Officially a base 
//		class for other buildables
//
// 01/20/2004, Mulchman: 
//		Having no sounds (build/explode) won't cause problems
//
// 05/09/2005, Mulchman: 
//		Tons of additions - better checking of build area, lots of 
//		cleanup... basically an overhaul
//
//	06/30/2006, Mulchman:
//		This thing has been through tons of changes and additions.
//		The latest thing is the doorblockers
//
//	05/10/2006, Mulchman:
//		Messing w/ the explode function and dealing better damage

#ifndef FF_BUILDABLEOBJECT_H
#define FF_BUILDABLEOBJECT_H

#ifdef CLIENT_DLL
	#define CFFBuildableObject C_FFBuildableObject
	#define CFFTeam C_FFTeam
	#define CFFPlayer C_FFPlayer

	#define CFFSentryGun C_FFSentryGun
	#define CFFDispenser C_FFDispenser
	#define CFFManCannon C_FFManCannon
	#define CFFDetpack C_FFDetpack

	#include "c_baseanimating.h"
#elif GAME_DLL
	#include "baseanimating.h"
	#include "ff_buildableflickerer.h"
#endif

class CFFTeam;
class CFFPlayer;
class CFFSentryGun;
class CFFDispenser;
class CFFDetpack;
class CFFManCannon;

#include "ff_buildabledefs.h"

//=============================================================================
//
//	class CFFBuildableObject / C_FFBuildableObject
//
//=============================================================================
class CFFBuildableObject : public CBaseAnimating
{
public:
	DECLARE_CLASS( CFFBuildableObject, CBaseAnimating )
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC()

	// --> shared
	CFFBuildableObject();
	virtual ~CFFBuildableObject();
	
	virtual bool IsAlive( void ) { return true; }
	virtual bool IsPlayer( void ) const { return false; }
	virtual bool BlocksLOS( void ) { return true; }
	virtual int	BloodColor( void ) { return BLOOD_COLOR_MECH; } // |-- Mirv: Don't bleed
	virtual int	GetTeamNumber();	// |-- Mirv: Easy team id accessor	
	bool IsBuilt( void	) const { return m_bBuilt; }
 
	CNetworkHandle( CBaseEntity, m_hOwner );

	CFFPlayer *GetOwnerPlayer( void );
	CFFPlayer *GetPlayerOwner( void ) { return GetOwnerPlayer(); } // I always want to type it this way instead of the one that already exists
	CFFTeam *GetOwnerTeam( void );
	int GetOwnerTeamId( void );

	int GetHealthPercent( void ) const;
	unsigned int GetAmmoPercent( void ) const { return m_iAmmoPercent; }

protected:
	CNetworkVarForDerived( unsigned int, m_iAmmoPercent );
	// <-- shared

#ifdef CLIENT_DLL
public:
	virtual RenderGroup_t GetRenderGroup();
	virtual int  DrawModel( int flags );
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void ClientThink( void );

	virtual int	GetHealth( void ) const { return m_iHealth; }
	virtual int	GetMaxHealth( void ) const { return m_iMaxHealth; }

	bool CheckForOwner( void ) { return ( m_hOwner.Get() ); }		

	// Stuff for the "can't build" type glyphs
	virtual void SetClientSideOnly( bool bValue ) { m_bClientSideOnly = bValue; }
	virtual void SetBuildError( BuildInfoResult_t hValue ) { m_hBuildError = hValue; }
protected:
	bool				m_bClientSideOnly;
	BuildInfoResult_t	m_hBuildError;
	bool				m_bBuilt;
	float				m_flSabotageTime;
	int					m_iSaboteurTeamNumber;

#else
public:
	virtual void Spawn( void ); 
	virtual void Precache( void );
	
	virtual Vector BodyTarget( const Vector &posSrc, bool bNoisy = false ) { return WorldSpaceCenter(); }
	
	virtual void GoLive( void );
	virtual void Detonate( void );
	virtual void UpdateOnRemove( void );
	virtual void RemoveSaboteur( bool bSuppressNotification = false );
	virtual void RemoveQuietly( void );

	static CFFBuildableObject *AttackerInflictorBuildable(CBaseEntity *pAttacker, CBaseEntity *pInflictor);

	CHandle<CFFPlayer>	m_hSaboteur;
	CNetworkVar(float, m_flSabotageTime);
	bool				m_bMaliciouslySabotaged;
	CNetworkVar(int, m_iSaboteurTeamNumber);

	virtual bool CanSabotage() const;
	virtual bool IsSabotaged() const;
	virtual bool IsMaliciouslySabotaged() const;
	virtual void Sabotage( CFFPlayer *pSaboteur ) {};
	virtual void MaliciouslySabotage( CFFPlayer *pSaboteur ) { m_bMaliciouslySabotaged = true; m_flSabotageTime = gpGlobals->curtime + 8.0f; }
	
	virtual void Cancel( void ) 
	{
		// Stop the build sound
		StopSound( m_ppszSounds[ 0 ] );
		RemoveQuietly(); 
	}
	
	bool CheckForOwner( void )
	{
		if( !m_hOwner.Get() )
		{
			RemoveQuietly();
			return false;
		}

		return true;
	}

	virtual bool HasMalfunctioned( void ) const;

	// NOTE: Super class handles touch function
	// void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );

	virtual int OnTakeDamage( const CTakeDamageInfo &info );

	virtual void Event_Killed( const CTakeDamageInfo &info );

	virtual bool ShouldSavePhysics( void ) { return false; }
	virtual int TakeEmp( void ) { return 0; }

	// Mirv: Store in advance the ground position
	virtual void SetGroundAngles(const QAngle &ang) { m_angGroundAngles = ang; }
	virtual void SetGroundOrigin(const Vector &vec) { m_vecGroundOrigin = vec; }

	void SetLocation(const char *_loc);
	const char *GetLocation() const { return m_BuildableLocation; }
private:
	// NOTE: Don't call the CFFBuildableObject::Create function
	static CFFBuildableObject *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

public:
	// So weapons (like the railgun) don't effect building
	virtual int VPhysicsTakeDamage( const CTakeDamageInfo &info );

protected:
	void Explode( void );
	void SpawnGib( const char *szGibModel, bool bFlame = true, bool bDieGroundTouch = false );	
	virtual void SpawnGibs( void );
	void DoExplosion( void );
	virtual void DoExplosionDamage() { AssertMsg(0, "No DoExplosionDamage()"); }

	virtual void SendStatsToBot() {};
protected:

	// Flickerer - flickers to indicate us taking damage
	CFFBuildableFlickerer *m_pFlickerer;

	// Mirv: Store in advance the ground position
	QAngle m_angGroundAngles;
	Vector m_vecGroundOrigin;

	// Pointer to array of char *'s of model names
	const char **m_ppszModels;
	// Pointer to array of char *'s of gib model names
	const char **m_ppszGibModels;
	// Pointer to array of char *'s of sounds
	const char **m_ppszSounds;

	// For the explosion function

	// Explosion magnitude (int)
	int		m_iExplosionMagnitude;
	// Explosion magnitude (float)
	float	m_flExplosionMagnitude;
	// Explosion radius (float -> 3.5*magnitude)
	float	m_flExplosionRadius;
	// Explosion radius (int -> 3.5*magnitude)
	int		m_iExplosionRadius;
	// Explosion force
	float	m_flExplosionForce;
	// Explosion damage (for radius damage - same as flExplosion force)
	float	m_flExplosionDamage;
	// Explosion duration (duration of screen shaking)
	float	m_flExplosionDuration;
	// Explosion fireball scale
	int		m_iExplosionFireballScale;

	// Time (+ gpGlobals->curtime) that we will think (update network vars)
	float	m_flThinkTime;// = 0.2f;

	// Jiggles: Time between sending the "take damage" hints -- to avoid spamming
	float m_flOnTakeDamageHintTime;

	// Shockwave texture
	int		m_iShockwaveExplosionTexture;
	// Draw shockwaves
	bool	m_bShockWave;

	// Object is live and in color (not being built)
	CNetworkVar( bool, m_bBuilt );
	// Object takes damage once it is built
	bool	m_bTakesDamage;

	// Object has sounds associated with it
	bool	m_bHasSounds;

	// Whether or not the model is translucent
	// while building
	bool	m_bTranslucent;

	// If true we should be using physics
	bool	m_bUsePhysics;

	char	m_BuildableLocation[1024];
public:
		virtual void DetonateThink();
		virtual void DetonateNextFrame() 
		{ 
			if( m_bMarkedForDetonation )
				return;

			m_bMarkedForDetonation = true;
			SetContextThink(&CFFBuildableObject::DetonateThink, gpGlobals->curtime + gpGlobals->interval_per_tick, "DetonateThink");
		}
		bool	m_bMarkedForDetonation;
#endif
};

// Utils
bool FF_IsBuildableObject( CBaseEntity *pEntity );
bool FF_IsDispenser( CBaseEntity *pEntity );
bool FF_IsSentrygun( CBaseEntity *pEntity );
bool FF_IsDetpack( CBaseEntity *pEntity );
bool FF_IsManCannon( CBaseEntity *pEntity );

CFFBuildableObject *FF_ToBuildableObject( CBaseEntity *pEntity );
CFFDispenser *FF_ToDispenser( CBaseEntity *pEntity );
CFFSentryGun *FF_ToSentrygun( CBaseEntity *pEntity );
CFFDetpack *FF_ToDetpack( CBaseEntity *pEntity );
CFFManCannon* FF_ToManCannon( CBaseEntity* pEntity );

#endif // FF_BUILDABLEOBJECT_H