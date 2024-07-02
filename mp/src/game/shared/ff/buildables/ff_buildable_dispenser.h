// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildable_detpack.h
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief Dispenser class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	05/10/2005, Mulchman:
//		Added some stuff for regeneration of items inside
//
//	05/11/2005, Mulchman
//		Added some OnTouch stuff to give the player touching
//		us some stuff
//
//	05/13/2005, Mulchman
//		Fixed the OnTouch stuff to work correctly now, heh
//
//	05/17/2005, Mulchman
//		Added some code so the disp is ready
//		to calculate a new explosion magnitude
//		based on how full it is
//
//	08/25/2005, Mulchman
//		Removed physics orienting the weapon and am now doing
//		it manually
//
// 22/01/2006, Mirv:
//		Dispenser now has ground pos & angles pre-stored for when it goes live
//
//	05/04/2006,	Mulchman
//		AddAmmo function. Minor tweaks here and there.
//
//	05/10/2006,	Mulchman:
//		Matched values from TFC (thanks Dospac)
//		Add radio tags to dispenser

#ifndef FF_BUILDABLE_DISPENSER_H
#define FF_BUILDABLE_DISPENSER_H

#ifdef CLIENT_DLL
	#define CFFDispenser C_FFDispenser
#endif

#include "ff_buildabledefs.h"

//=============================================================================
//
//	class CFFDispenser / C_FFDispenser
//
//=============================================================================
class CFFDispenser : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFDispenser, CFFBuildableObject )
	DECLARE_NETWORKCLASS()
	DECLARE_DATADESC()

	// --> shared
	CFFDispenser( void );
	virtual ~CFFDispenser( void );

	virtual Class_T Classify( void ) { return CLASS_DISPENSER; }


public:
	// Network variables
	CNetworkVar( int, m_iCells );
	CNetworkVar( int, m_iShells );
	CNetworkVar( int, m_iNails );
	CNetworkVar( int, m_iRockets );
	CNetworkVar( int, m_iArmor );

	int GetCells( void ) const { return m_iCells; }
	int GetShells( void ) const { return m_iShells; }
	int GetNails ( void ) const { return m_iNails; }
	int GetRockets( void ) const { return m_iRockets; }
	int GetArmor( void ) const { return m_iArmor; }
	
	int NeedsHealth( void ) const { return m_iMaxHealth - m_iHealth; }
	int NeedsArmor( void ) const { return m_iMaxArmor - m_iArmor; }
	int NeedsCells( void ) const { return m_iMaxCells - m_iCells; }
	int NeedsShells( void ) const { return m_iMaxShells - m_iShells; }
	int NeedsNails( void ) const { return m_iMaxNails - m_iNails; }
	int NeedsRockets( void ) const { return m_iMaxRockets - m_iRockets; }

protected:
	int		m_iMaxCells;
	int		m_iGiveCells;
	int		m_iMaxShells;
	int		m_iGiveShells;
	int		m_iMaxNails;
	int		m_iGiveNails;
	int		m_iMaxRockets;
	int		m_iGiveRockets;
	int		m_iMaxArmor;
	int		m_iGiveArmor;
	// <-- shared

public:

#ifdef CLIENT_DLL 
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// Creates a client side ONLY dispenser - used for build slot
	static CFFDispenser *CreateClientSideDispenser( const Vector& vecOrigin, const QAngle& vecAngles );
#else
	virtual void Spawn( void );
	void GoLive( void );

	virtual Vector EyePosition( void ) { return GetAbsOrigin() + Vector( 0, 0, 48.0f ); }

	void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	virtual void SpawnGibs( void );
	virtual void DoExplosionDamage();

	virtual void Sabotage(CFFPlayer *pSaboteur);
	virtual void MaliciouslySabotage(CFFPlayer *pSaboteur);
	virtual void Detonate();

    bool CloseEnoughToDismantle( CFFPlayer *pPlayer);
    void Dismantle( CFFPlayer *pPlayer);

	CNetworkVar( unsigned int, m_iAmmoPercent );

	void AddAmmo( int iArmor, int iCells, int iShells, int iNails, int iRockets );

	// Some functions for the custom dispenser text
	void SetText( const char *szCustomText ) { Q_strcpy( m_szCustomText, szCustomText ); }
	const char *GetText( void ) const { return m_szCustomText; }

	static CFFDispenser *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

	// These are for updating the user
	virtual void	PhysicsSimulate();
	float			m_flLastClientUpdate;
	int				m_iLastState;

protected:
	void SendStatsToBot( void );

	// Custom dispenser text string thing
	char		m_szCustomText[ FF_BUILD_DISP_STRING_LEN ];
	CFFPlayer	*m_pLastTouch;
	float		m_flLastTouch;

	// Actually give a player stuff
	void Dispense( CFFPlayer *pPlayer );

	// Calculates an adjustment to be made to the explosion
	// based on how much stuff is in the dispenser
	void CalcAdjExplosionVal( void );
	float	m_flOrigExplosionMagnitude;

	void UpdateAmmoPercentage( void );
#endif

};

#endif // FF_BUILDABLE_DISPENSER_H