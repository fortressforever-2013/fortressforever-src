// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildable_detpack.h
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief Detpack class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	09/28/2005,	Mulchman:
//		Played with the think time and adding
//		support for whine sound when hit by
//		emp or 5 seconds left to go until
//		explode time

#ifndef FF_BUILDABLE_DETPACK_H
#define FF_BUILDABLE_DETPACK_H

#ifdef CLIENT_DLL
	#define CFFDetpack C_FFDetpack
#endif

#include "ff_buildabledefs.h"

//=============================================================================
//
//	class CFFDetpack / C_FFDetpack
//
//=============================================================================
class CFFDetpack : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFDetpack, CFFBuildableObject )
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC()

	// --> shared
	CFFDetpack( void );
	virtual ~CFFDetpack( void );

	virtual bool	BlocksLOS( void ) const { return false; }
	virtual Class_T Classify( void ) { return CLASS_DETPACK; }
	// <-- shared

#ifdef CLIENT_DLL
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// Creates a client side ONLY detpack - used for the build slot
	static CFFDetpack *CreateClientSideDetpack( const Vector& vecOrigin, const QAngle& vecAngles );	
#else
	virtual void Spawn( void );
	void GoLive( void );

	virtual void Detonate();

	virtual Vector EyePosition( void ) { return GetAbsOrigin() + Vector( 0, 0, 4.0f ); }

	void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );
	virtual int TakeEmp( void );
	virtual void DoExplosionDamage();

	static CFFDetpack *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

	int		m_iFuseTime;
	float	m_flDetonateTime;
	bool	m_bFiveSeconds;

	// accessors for lua
	int	GetFuseTime() { return m_iFuseTime;	}
	float GetDetonateTime() { return m_flDetonateTime; }
	bool LastFiveSeconds() { return m_bFiveSeconds; }

#endif

};

#endif // FF_BUILDABLE_DETPACK_H