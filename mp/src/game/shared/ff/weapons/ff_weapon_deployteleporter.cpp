//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_weapon_deploydispenser.cpp
//	@author Gavin "Mirvin_Monkey" Bramhill
//	@date May 10, 2005
//	@brief A test dispenser construction slot
//
//	REVISIONS
//	---------
//	May 10, 2005, Mirv: First created
//
//	05/10/05, Mulchman:
//		Took over this class, thanks Mirv. Teleporter now sees if it can be built
//		during weapon idle times. If it can it draws a box. Pressing the attack
//		button will cause the dispenser to be built(if it can be).
//
//	05/11/05, Mulchman:
//		Fixed a bug where the faux dispenser would still be valid even though
//		we had built the thing already
//
//	05/14/05, Mulchman:
//		Optimized as per Mirv's forum suggestion
//
//	03/14/06, Mulchman:
//		Add some stuff for bug fixing

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "in_buttons.h"


#ifdef CLIENT_DLL 
	#define CFFWeaponDeployTeleporter C_FFWeaponDeployTeleporter
	#define CFFTeleporter C_FFTeleporter

	#include "c_ff_player.h"
	#include "ff_hud_chat.h"
#else
	#include "ff_player.h"
#endif

#include "ff_buildableobject.h"
#include "ff_buildable_teleporter.h"

#include "ff_buildableinfo.h"

//=============================================================================
// CFFWeaponDeployTeleporter
//=============================================================================

class CFFWeaponDeployTeleporter : public CFFWeaponBase
{
public:
	DECLARE_CLASS(CFFWeaponDeployTeleporter, CFFWeaponBase);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponDeployTeleporter( void );
#ifdef CLIENT_DLL 
	~CFFWeaponDeployTeleporter( void ) { Cleanup(); }
#endif

	virtual void PrimaryAttack( void );
	virtual void WeaponIdle( void );
	virtual bool Holster(CBaseCombatWeapon *pSwitchingTo);
	virtual bool CanBeSelected( void );
	virtual bool CanDeploy( void );
	virtual void ItemPostFrame( void );

	virtual FFWeaponID GetWeaponID( void ) const		{ return FF_WEAPON_DEPLOYTELEPORTER; }

private:

	CFFWeaponDeployTeleporter(const CFFWeaponDeployTeleporter &);

protected:
#ifdef CLIENT_DLL
	CFFTeleporter *m_pBuildable;
#endif

	// I've stuck the client dll check in here so it doesnt have to be everywhere else
	void Cleanup() 
	{
#ifdef CLIENT_DLL
		if (m_pBuildable) 
		{
			m_pBuildable->Remove();
			m_pBuildable = NULL;
		}
#endif
	}
};

//=============================================================================
// CFFWeaponDeployTeleporter tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFWeaponDeployTeleporter, DT_FFWeaponDeployTeleporter) 

BEGIN_NETWORK_TABLE(CFFWeaponDeployTeleporter, DT_FFWeaponDeployTeleporter) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA(CFFWeaponDeployTeleporter) 
END_PREDICTION_DATA() 

LINK_ENTITY_TO_CLASS(ff_weapon_deployteleporter, CFFWeaponDeployTeleporter);
PRECACHE_WEAPON_REGISTER(ff_weapon_deployteleporter);

//=============================================================================
// CFFWeaponDeployTeleporter implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponDeployTeleporter::CFFWeaponDeployTeleporter( void ) 
{
#ifdef CLIENT_DLL
	m_pBuildable = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: A modified ItemPostFrame to allow for different cycledecrements
//-----------------------------------------------------------------------------
void CFFWeaponDeployTeleporter::ItemPostFrame()
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// if just released the attack, then reset nextfiretime
	if (pOwner->m_afButtonReleased & IN_ATTACK)
		m_flNextPrimaryAttack = gpGlobals->curtime;

	if ((pOwner->m_nButtons & IN_ATTACK || pOwner->m_afButtonPressed & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
			PrimaryAttack();
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (! ((pOwner->m_nButtons & IN_ATTACK) || /* (pOwner->m_nButtons & IN_ATTACK2) ||*/ (pOwner->m_nButtons & IN_RELOAD))) // |-- Mirv: Removed attack2 so things can continue while in menu
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Handles whatever should be done when they fire(build, aim, etc) 
//----------------------------------------------------------------------------
void CFFWeaponDeployTeleporter::PrimaryAttack( void ) 
{
	if (m_flNextPrimaryAttack < gpGlobals->curtime) 
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

		Cleanup();

#ifdef GAME_DLL
		CFFPlayer *pPlayer = GetPlayerOwner();
		pPlayer->Command_BuildTeleporter();
#endif
	}
}

//----------------------------------------------------------------------------
// Purpose: Checks validity of ground at this point or whatever
//----------------------------------------------------------------------------
void CFFWeaponDeployTeleporter::WeaponIdle( void ) 
{
	if (m_flTimeWeaponIdle < gpGlobals->curtime) 
	{
		//SetWeaponIdleTime(gpGlobals->curtime + 0.1f);

#ifdef CLIENT_DLL 
		C_FFPlayer *pPlayer = GetPlayerOwner();

		// If we've built and we're not building pop out wrench
		/*if( ( pPlayer->GetTeleporter() && !pPlayer->IsBuilding() ) || ( pPlayer->GetAmmoCount( AMMO_CELLS ) < 100 ) )
			pPlayer->SwapToWeapon( FF_WEAPON_SPANNER );*/

		if( !pPlayer->IsStaticBuilding() )
		{
			if (!pPlayer->GetTeleporterEntrance())
			{
				CFFBuildableInfo hBuildInfo(pPlayer, FF_BUILD_TELEPORTER_ENTRANCE);
				if (!m_pBuildable)
				{
					m_pBuildable = CFFTeleporter::CreateClientSideTeleporter(hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles());
				}
				else
				{
					m_pBuildable->SetAbsOrigin(hBuildInfo.GetBuildOrigin());
					m_pBuildable->SetAbsAngles(hBuildInfo.GetBuildAngles());
				}
				m_pBuildable->SetBuildError(hBuildInfo.BuildResult());
			}
			else if (!pPlayer->GetTeleporterExit())
			{
				CFFBuildableInfo hBuildInfo(pPlayer, FF_BUILD_TELEPORTER_EXIT);
				if (!m_pBuildable)
				{
					m_pBuildable = CFFTeleporter::CreateClientSideTeleporter(hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles());
				}
				else
				{
					m_pBuildable->SetAbsOrigin(hBuildInfo.GetBuildOrigin());
					m_pBuildable->SetAbsAngles(hBuildInfo.GetBuildAngles());
				}
				m_pBuildable->SetBuildError(hBuildInfo.BuildResult());
			}
		}
		else
			Cleanup();
#endif
	}
}

bool CFFWeaponDeployTeleporter::Holster(CBaseCombatWeapon *pSwitchingTo) 
{
	Cleanup();

	return BaseClass::Holster( pSwitchingTo );
}

bool CFFWeaponDeployTeleporter::CanDeploy( void )
{
	return BaseClass::CanDeploy();
}

bool CFFWeaponDeployTeleporter::CanBeSelected( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if( !pPlayer )
		return false;

	if( pPlayer->GetTeleporterEntrance() && pPlayer->GetTeleporterExit() )
	{
		return false;
	}

	return BaseClass::CanBeSelected();
}

#ifdef GAME_DLL
	//=============================================================================
	// Commands
	//=============================================================================
	CON_COMMAND(dismantletpen, "Dismantle teleporter entrance")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_TELEPORTER_ENTRANCE ) )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		CFFTeleporter *pTeleporter = pPlayer->GetTeleporterEntrance();

		// can't dismantle what doesn't exist
		if (!pTeleporter)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NO_TPEN_TODISMANTLE");	
			return;
		}

		//Bug fix: dismantling a ghost teleporter 
		//if the teleporter is in transparent form, dont dismantle it -GreenMushy
		if( pTeleporter->IsTransparent() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		if (pTeleporter->CloseEnoughToDismantle(pPlayer))
		{
			pTeleporter->Dismantle(pPlayer);
		}
		else
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_TOOFARAWAY");
		}
	}

	CON_COMMAND(dettpen, "Detonates teleporter entrance")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_TELEPORTER_ENTRANCE ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETMIDBUILD" );
			return;
		}

		CFFTeleporter *pTeleporter = pPlayer->GetTeleporterEntrance();

		// can't detonate what we don't have
		if (!pTeleporter)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NO_TPEN_TODET");
			return;
		}

		pTeleporter->DetonateNextFrame();
	}

	CON_COMMAND(detdismantletpen, "Dismantles or detonate teleporter entrance depending on distance")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		if( !pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEORDETWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_TELEPORTER_ENTRANCE ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD" );
			return;
		}

		CFFTeleporter *pTeleporter = pPlayer->GetTeleporterEntrance();

		// can't do owt to it 'cause it doesn't exist!
		if (!pTeleporter)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NO_TPEN");
			return;
		}

		//Bug fix: dismantling a ghost teleporter 
		//if the teleporter is in transparent form, dont dismantle it -GreenMushy
		if( pTeleporter->IsTransparent() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		//The previous IsBuilt function didnt seem to work so i removed it -GreenMushy
		if (pTeleporter->CloseEnoughToDismantle(pPlayer))
		{
			pTeleporter->Dismantle(pPlayer);
		}
		else
		{
			pTeleporter->DetonateNextFrame();
		}
	}

	CON_COMMAND(dismantletpex, "Dismantle teleporter exit")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_TELEPORTER_EXIT ) )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		CFFTeleporter *pTeleporter = pPlayer->GetTeleporterExit();

		// can't dismantle what doesn't exist
		if (!pTeleporter)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NO_TPEX_TODISMANTLE");	
			return;
		}

		//Bug fix: dismantling a ghost teleporter 
		//if the teleporter is in transparent form, dont dismantle it -GreenMushy
		if( pTeleporter->IsTransparent() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		if (pTeleporter->CloseEnoughToDismantle(pPlayer))
		{
			pTeleporter->Dismantle(pPlayer);
		}
		else
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_TOOFARAWAY");
		}
	}

	CON_COMMAND(dettpex, "Detonates teleporter exit")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		if( ! pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_TELEPORTER_EXIT ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDETMIDBUILD" );
			return;
		}

		CFFTeleporter *pTeleporter = pPlayer->GetTeleporterExit();

		// can't detonate what we don't have
		if (!pTeleporter)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NO_TPEX_TODET");
			return;
		}

		pTeleporter->DetonateNextFrame();
	}

	CON_COMMAND(detdismantletpex, "Dismantles or detonate teleporter exit depending on distance")
	{
		CFFPlayer *pPlayer = ToFFPlayer(UTIL_GetCommandClient());

		if (!pPlayer)
			return;

		if( !pPlayer->IsAlive() )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEORDETWHENDEAD" );
			return;
		}

		// Bug #0000333: Buildable Behavior (non build slot) while building
		if( pPlayer->IsBuilding() && ( pPlayer->GetCurrentBuild() == FF_BUILD_TELEPORTER_EXIT ) )
		{
			ClientPrint( pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD" );
			return;
		}

		CFFTeleporter *pTeleporter = pPlayer->GetTeleporterExit();

		// can't do owt to it 'cause it doesn't exist!
		if (!pTeleporter)
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_NO_TPEX");
			return;
		}

		//Bug fix: dismantling a ghost teleporter 
		//if the teleporter is in transparent form, dont dismantle it -GreenMushy
		if( pTeleporter->IsTransparent() )
		{
			ClientPrint(pPlayer, HUD_PRINTCENTER, "#FF_ENGY_CANTDISMANTLEMIDBUILD");			
			return;
		}

		//The previous IsBuilt function didnt seem to work so i removed it -GreenMushy
		if (pTeleporter->CloseEnoughToDismantle(pPlayer))
		{
			pTeleporter->Dismantle(pPlayer);
		}
		else
		{
			pTeleporter->DetonateNextFrame();
		}
	}
#endif