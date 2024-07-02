// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildable_sentrygun.h
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief SentryGun class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	05/17/2005, Mulchman:
//		Starting to make it animated and such
//
//	06/01/2005, Mulchman:
//		Noticed I had dates wrong... *cough* and
//		still working on making the SG animated
//		and such.
//
//	06/08/2005, Mulchman:
//		Decided the SG needs to inherit from the
//		AI base class and not the buildable class.
//		Some easy stuff will give it the same base
//		buildable attributes while inheriting all
//		of the AI stuff that it so desperately needs!
//
//	05/10/2006, Mulchman:
//		Cleaned this up A LOT. SG still doesn't factor in radiotagged targets, though.

#ifndef FF_BUILDABLE_SENTRYGUN_H
#define FF_BUILDABLE_SENTRYGUN_H

#ifdef CLIENT_DLL
	#define CFFSentryGun C_FFSentryGun
#endif

#include "ff_buildabledefs.h"

#define SG_BC_YAW			"aim_yaw"
#define SG_BC_PITCH			"aim_pitch"
#define SG_BC_BARREL_ROTATE	"barrel_rotate"
//#define	SG_RANGE			1200
#define	SG_MAX_WAIT			5
#define SG_SHORT_WAIT		2.0		// Used for FAST_RETIRE spawnflag
#define	SG_PING_TIME		10.0f	// LPB!!

#define SG_MAX_PITCH		90.0f
#define SG_MIN_PITCH		-85.0f//-90.0f
#define SG_MIN_ANIMATED_PITCH		-30.0f//-33.0f (caes: changed to -30 so animation doesn't go up again at end)
#define SG_SCAN_HALFWIDTH	30.0f//40.0f

//extern ConVar sg_health_lvl1;
//#define SG_HEALTH_LEVEL1	sg_health_lvl1.GetInt()
//extern ConVar sg_health_lvl2;
//#define SG_HEALTH_LEVEL2	sg_health_lvl2.GetInt()
//extern ConVar sg_health_lvl3;
//#define SG_HEALTH_LEVEL3	sg_health_lvl3.GetInt()

class SmokeTrail;

//=============================================================================
//
//	class CFFSentryGun / C_FFSentryGun
//
//=============================================================================
class CFFSentryGun : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFSentryGun, CFFBuildableObject )
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC()

	// --> shared
	CFFSentryGun( void );
	virtual ~CFFSentryGun( void );

	int GetRockets( void ) const  { return m_iRockets; };
	int GetShells( void ) const  { return m_iShells; };
	int GetHealth( void ) const { return m_iHealth; };

	int GetMaxRockets( void ) const  { return m_iMaxRockets; };
	int GetMaxShells( void ) const { return m_iMaxShells; };
	int GetMaxHealth( void ) const { return m_iMaxHealth; };

	int GetRocketsPercent( void ) const  { return (int) ((float) GetRockets() / (float) GetMaxRockets()) * 100.0f; };
	int GetShellsPercent( void ) const  { return (int) ((float) GetShells() / (float) GetMaxShells()) * 100.0f; };
	int GetHealthPercent( void ) const  { return (int) ((float) GetHealth() / (float) GetMaxHealth()) * 100.0f; };

	int NeedsRockets( void ) const { return GetMaxRockets() - GetRockets(); }
	int NeedsShells( void ) const { return GetMaxShells() - GetShells(); }
	int NeedsHealth( void ) const { return GetMaxHealth() - GetHealth(); }

	int GetLevel( void ) const { return m_iLevel; }
	bool Upgrade();

#ifdef GAME_DLL
	void Repair( int iCells = 0 );
	void AddAmmo( int iShells = 0, int iRockets = 0 );

	void SetLevel( int iLevel, bool bEmitSounds=true );
	void SetRockets(int iRockets) { m_iRockets = clamp( iRockets, 0, GetMaxRockets() ); RecalculateAmmoPercent(); }
	void SetShells(int iShells) { m_iShells = clamp( iShells, 0, GetMaxShells() ); RecalculateAmmoPercent(); }
	void SetHealth(int iHealth) { m_iHealth = clamp( iHealth, 0, GetMaxHealth() ); RecalculateAmmoPercent(); }
	void RecalculateAmmoPercent();
#endif

	virtual Class_T Classify( void ) { return CLASS_SENTRYGUN; }
	virtual bool	IsNPC( void ) { return true; }

public:
	// Network variables
	//CNetworkVar( float, m_flRange );
	CNetworkVar( int, m_iLevel );
	CNetworkVar( int, m_iShells );
	CNetworkVar( int, m_iRockets );

	CNetworkVar( int, m_iMaxShells );
	CNetworkVar( int, m_iMaxRockets );
	// <-- shared

#ifdef CLIENT_DLL 
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// Creates a client side ONLY sentrygun - used for build slot
	static CFFSentryGun *CreateClientSideSentryGun( const Vector& vecOrigin, const QAngle& vecAngles );

	int	m_iLocalHallucinationIndex;

	virtual int DrawModel(int flags);

	// Mulch: now this is in buildableobject to extend to disp & sg
	// Mirv: Just going to store the ammo percentage here, with the msb
	// holding the rocket state
	//unsigned int m_iAmmoPercent;
#else
	virtual void Precache( void );
	virtual void Spawn( void );
	virtual void UpdateOnRemove( void );
	void GoLive( void );

	int TakeEmp( void );

	void SetFocusPoint(Vector &origin);

	void OnObjectThink( void ); // NOTE: Not an actual think function but called during every think function
	void OnSearchThink( void );
	void OnActiveThink( void );

	CBaseEntity *HackFindEnemy( void );
	
	Vector GetVecAiming() 
	{
		Vector vecAiming;
		AngleVectors(m_angAiming, &vecAiming);
		return vecAiming;
	}
	Vector GetVecGoal()
	{
		Vector vecGoal;
		AngleVectors( m_angGoal, &vecGoal );
		return vecGoal;
	}

	float MaxYawSpeed( void ) const;
	float MaxPitchSpeed( void ) const;

	virtual void DoExplosionDamage();
	virtual void SpawnGibs( void );
	// These are for updating the user
	virtual void	PhysicsSimulate();
	float			m_flLastClientUpdate;
	int				m_iLastState;


private:
	bool IsTargetInAimingEllipse( const Vector& vecTarget ) const;
	bool IsTargetVisible( CBaseEntity *pTarget, int iSightDistance );
	bool IsTargetClassTValid( Class_T cT ) const;

public:
	CBaseEntity *GetEnemy( void	) const { return m_hEnemy; }
	void SetEnemy( CBaseEntity *hEnemy );
private:
	CHandle< CBaseEntity >	m_hEnemy;

public:
	void Shoot();
	void ShootRocket();

protected:
	void Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	void ShootRocket( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	void Ping( void );	
	void SpinUp( void );
	void SpinDown( void );
	bool UpdateFacing( void );

	void SendStatsToBot( void );

public:
	virtual void Event_Killed( const CTakeDamageInfo &info );

	const char *GetTracerType( void ) { return "AR2Tracer"; }

	virtual Vector EyePosition( void );
	Vector MuzzlePosition( void );
	Vector RocketPosition( void );
	Vector EyeOffset( Activity nActivity ) { return Vector( 0, 0, 64 ); }

	bool				m_bSendNailGrenHint;	// Only send the "kill sgs with nail grens" hint once per sg
	float				m_flNextSparkTime;
	SmokeTrail			*m_pSmokeTrail;

	float	m_flLastCloakSonarSound;
	float	m_flCloakDistance;

	virtual void Sabotage(CFFPlayer *pSaboteur);
	virtual void MaliciouslySabotage(CFFPlayer *pSaboteur);
	virtual void Detonate();

	static CFFSentryGun *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

	void DoMuzzleFlash( int iAttachment, const Vector& vecOrigin, const QAngle& vecAngles );
	void DoRocketMuzzleFlash( int iAttachment, const Vector& vecOrigin, const QAngle& vecAngles );

public:
	CNetworkVar(unsigned int, m_iAmmoPercent);


	int		m_iShellDamage;

	float	m_flRocketCycleTime;
	float	m_flShellCycleTime;

	float	m_flTurnSpeed;
	float	m_flPitchSpeed;
	float	m_flLockTime;

	float m_flBarrelRotationDelta;
	float m_flBarrelRotationValue;
	float m_flSpinDownStartTime;
	float m_flSpinUpStartTime;

	// Ammo definition for shells
	int		m_iAmmoType;
	float	m_flNextShell;
	float	m_flNextRocket;
	float	m_flShotAccumulator;

	float	m_flLastSight;
	float	m_flPingTime;
	float	m_flNextActivateSoundTime;
	float	m_flAcknowledgeSabotageTime;
	float	m_flStartLockTime;
	float	m_flEndLockTime;

	int		m_iEyeAttachment;
	int		m_iMuzzleAttachment;
	int		m_iRocketLAttachment;
	int		m_iRocketRAttachment;

	int m_iPitchPoseParameter;
	int m_iYawPoseParameter;

	//
	// Level 3 only stuff
	//
	// Which barrel to fire from (when level 3)
	bool m_bLeftBarrel;
	bool m_bRocketLeftBarrel;
	int m_iLBarrelAttachment;
	int m_iRBarrelAttachment;

	// Aiming
	QAngle	m_angGoal;
	QAngle	m_angAimBase;
	QAngle	m_angAiming;

	// caes: store angular speeds of SG
	float m_angSpeed_yaw;
	float m_angSpeed_pitch;
	// caes
#endif
};

#endif // FF_BUILDABLE_SENTRYGUN_H