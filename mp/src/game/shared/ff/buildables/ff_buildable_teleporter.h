// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildable_teleporter.h
// @author Patrick O'Leary (Mulchman), -_YoYo178_-
// @date 12/6/2007
// @brief Teleporters
//
// REVISIONS
// ---------
// 12/6/2007, Mulchman: 
//		First created
//		Added man cannon stuff
// 
// 4/7/2024, -_YoYo178_-: 
//		Created teleporters based off
//			of mancannon

#ifndef FF_BUILDABLE_TELEPORTER_H
#define FF_BUILDABLE_TELEPORTER_H

#ifdef CLIENT_DLL
	#define CFFTeleporter C_FFTeleporter
#endif

#include "ff_buildabledefs.h"

//=============================================================================
//
//	class CFFTeleporter / C_FFTeleporter
//
//=============================================================================
class CFFTeleporter : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFTeleporter, CFFBuildableObject )
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC()

	// --> shared
	CFFTeleporter( void );
	virtual ~CFFTeleporter( void );

	virtual Class_T Classify( void ) { return CLASS_TELEPORTER; }
	// <-- shared

	TeleporterState_t m_iState;
	TeleporterType_t m_iType;

	TeleporterTeleportState_t m_iTeleportState;

	CNetworkVar(float, m_flLastTeleport);
	CNetworkVar(float, m_flNextTeleport);

#ifdef CLIENT_DLL
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual int DrawModel(int flags);

	// Creates a client side ONLY teleporter - used for the build slot
	static CFFTeleporter *CreateClientSideTeleporter( const Vector& vecOrigin, const QAngle& vecAngles );	

	float m_flLastDamage;
#else
	virtual void Spawn( void );
	virtual void GoLive( void );

	void OnObjectTouch( CBaseEntity *pOther );
	void OnTeleporterThink( void );
	
	// These are for updating the user
	virtual void	PhysicsSimulate();
	float			m_flLastClientUpdate;
	int				m_iLastState;

	CFFPlayer* m_hTouchingPlayer;
	
	TeleporterState_t GetState( void ) { return m_iState; }
	void			SetState( TeleporterState_t iState ) { m_iState = iState; }

	TeleporterType_t GetType( void ) { return m_iType; }
	void			SetType( TeleporterType_t iType ) { m_iType = iType; }

	TeleporterTeleportState_t GetTeleportState( void ) { return m_iTeleportState; }
	void			SetTeleportState( TeleporterTeleportState_t iState ) { m_iTeleportState = iState; }

	bool			IsComplete( void ) { return GetState() == TELEPORTER_COMPLETE; }
	bool			IsInCooldown( void ) { return GetTeleportState() == TELEPORTER_INCOOLDOWN; }

	bool			IsEntrance( void ) { return GetType() == TELEPORTER_ENTRANCE; }
	bool			IsExit( void ) { return GetType() == TELEPORTER_EXIT; }

	CFFTeleporter*	GetEntrance( void );
	CFFTeleporter*	GetExit( void );
	void			SetEntrance( CFFTeleporter* pTeleporter );
	void			SetExit( CFFTeleporter* pTeleporter );

	CFFTeleporter*	m_hEntrance;
	CFFTeleporter*	m_hExit;

	virtual void Detonate( void );
	virtual void DoExplosionDamage( void );

	static CFFTeleporter *Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner = NULL );
#endif
};

#endif // FF_BUILDABLE_TELEPORTER_H