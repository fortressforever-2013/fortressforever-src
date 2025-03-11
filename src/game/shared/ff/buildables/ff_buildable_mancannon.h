// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildable_mancannon.h
// @author Patrick O'Leary (Mulchman)
// @date 12/6/2007
// @brief Man cannon thing
//
// REVISIONS
// ---------
// 12/6/2007, Mulchman: 
//		First created
//		Added man cannon stuff

#ifndef FF_BUILDABLE_MANCANNON_H
#define FF_BUILDABLE_MANCANNON_H

#ifdef CLIENT_DLL
	#define CFFManCannon C_FFManCannon
#endif

#include "ff_buildabledefs.h"

//=============================================================================
//
//	class CFFManCannon / C_FFManCannon
//
//=============================================================================
class CFFManCannon : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFManCannon, CFFBuildableObject )
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC()

	// --> shared
	CFFManCannon( void );
	virtual ~CFFManCannon( void );

	virtual Class_T Classify( void ) { return CLASS_MANCANNON; }
	// <-- shared

#ifdef CLIENT_DLL
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual int DrawModel(int flags);

	// Creates a client side ONLY man cannon - used for the build slot
	static CFFManCannon *CreateClientSideManCannon( const Vector& vecOrigin, const QAngle& vecAngles );	

	float m_flLastDamage;
#else
	virtual void Spawn( void );
	virtual void GoLive( void );

	void OnObjectTouch( CBaseEntity *pOther );
	void OnJumpPadThink( void );
	
	// These are for updating the user
	virtual void	PhysicsSimulate();
	float			m_flLastClientUpdate;
	int				m_iLastState;
	JumpPadState_t	m_iCombatState;
	float			m_flLastHeal;
	CNetworkVar( float, m_flLastDamage );

	virtual bool CanSabotage( void ) const { return false; }
	virtual bool IsSabotaged( void ) const { return false; }
	virtual bool IsMaliciouslySabotaged( void ) const { return false; }
	virtual void Sabotage( CFFPlayer *pSaboteur ) {};
	virtual void MaliciouslySabotage( CFFPlayer *pSaboteur ) { m_bMaliciouslySabotaged = true; m_flSabotageTime = gpGlobals->curtime + 8.0f; }

	virtual void Detonate( void );
	virtual void DoExplosionDamage( void );

	static CFFManCannon *Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner = NULL );
#endif
};

#endif // FF_BUILDABLE_MANCANNON_H