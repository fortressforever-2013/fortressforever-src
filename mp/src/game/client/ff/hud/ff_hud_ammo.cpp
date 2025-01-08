//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
//#include "hud.h"
#include "hudelement.h"
//#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
//#include "ammodef.h"

//#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
//#include <vgui_controls/AnimationController.h>
//#include <igameresources.h>

#include "c_ff_player.h"
#include "ff_weapon_base.h"
//#include "ff_hud_boxes.h"
#include "ff_utils.h"

//#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudAmmo, CHudNumericDisplay);

public:
	
	CHudAmmo(const char *pElementName);
	
	virtual void Init();
	virtual void VidInit();
	virtual void Reset();
	virtual void OnTick();

protected:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;

	virtual void SetAmmo(int ammo, bool playAnimation);
	virtual int GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon);

	int		m_iAmmo;
};

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level in clip
//-----------------------------------------------------------------------------
class CHudAmmoClip : public CHudAmmo
{
	DECLARE_CLASS_SIMPLE(CHudAmmoClip, CHudAmmo);

public:

	CHudAmmoClip(const char *pElementName) : BaseClass(pElementName)
	{
		// Hopefully not too late to do this
		SetName("HudAmmoClip");
	}

	virtual int GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon)
	{
		if (!pWeapon)
			return -1;

		return pWeapon->Clip1();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Displays current weapon & ammo
//-----------------------------------------------------------------------------
class CHudAmmoInfo : public CHudElement, public FFPanel
{
public:
	DECLARE_CLASS_SIMPLE(CHudAmmoInfo, FFPanel);

	CHudAmmoInfo(const char *pElementName) : CHudElement(pElementName), FFPanel(NULL, "HudAmmoInfo")
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED | HIDEHUD_WEAPONSELECTION);
	}

	virtual ~CHudAmmoInfo(void)
	{
		if (m_pWeaponIcon)
		{
			delete m_pWeaponIcon;
			m_pWeaponIcon = NULL;
		}
		if (m_pAmmoIcon)
		{
			delete m_pAmmoIcon;
			m_pAmmoIcon = NULL;
		}
	}

	virtual void Paint(void);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnTick(void);

protected:
	CHudTexture* m_pWeaponIcon;
	CHudTexture* m_pAmmoIcon;

	C_BaseCombatWeapon* m_pWeapon;

private:
	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hWeaponIconFont, "weapon_icon_font", "WeaponIconsHUD");
	CPanelAnimationVar(vgui::HFont, m_hAmmoIconFont, "ammo_icon_font", "AmmoIconsSmall");

	CPanelAnimationVarAliasType(float, m_flAmmoIconX, "ammo_icon_x", "5", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flAmmoIconY, "ammo_icon_y", "18", "proportional_float");
	CPanelAnimationVar(Color, m_clrAmmoIconColor, "ammo_icon_color", "HudItem.Foreground");

	CPanelAnimationVarAliasType(float, m_flWeaponIconX, "weapon_icon_x", "45", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flWeaponIconY, "weapon_icon_y", "0", "proportional_float");
	CPanelAnimationVar(Color, m_clrWeaponIconColor, "weapon_icon_color", "HudItem.Foreground");
};

DECLARE_HUDELEMENT(CHudAmmo);
DECLARE_HUDELEMENT(CHudAmmoClip);
DECLARE_HUDELEMENT(CHudAmmoInfo);

CHudAmmo::CHudAmmo(const char *pElementName) : BaseClass(NULL, "HudAmmo"), CHudElement(pElementName)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED | HIDEHUD_WEAPONSELECTION);
}

int CHudAmmo::GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon)
{
	if (!pWeapon || !pPlayer)
		return -1;

	return pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
}
void CHudAmmo::Init()
{
	Reset();
	ivgui()->AddTickSignal( GetVPanel(), 100 );
}

void CHudAmmo::VidInit()
{
	Reset();
}

void CHudAmmo::Reset()
{
	m_hCurrentActiveWeapon = NULL;
	SetLabelText(L"");
	m_iAmmo = -1;
}

void CHudAmmo::OnTick()
{
	BaseClass::OnTick();
	if (!m_pFFPlayer)
		return;

	C_BaseCombatWeapon *pWeapon = m_pFFPlayer->GetActiveWeapon();

	int iAmmo = -1;

	if (pWeapon && pWeapon->UsesPrimaryAmmo())
	{
		iAmmo = GetPlayerAmmo(m_pFFPlayer, pWeapon);
	}

	if ( iAmmo < 0 )
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}
	else
	{
		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);
	}

	if (pWeapon == m_hCurrentActiveWeapon)
	{
		// Same weapon, update w/ animations
		SetAmmo(iAmmo, true);
	}
	else
	{
		// Different weapon, update w/o animations
		SetAmmo(iAmmo, false);
		m_hCurrentActiveWeapon = pWeapon;
	}
}

void CHudAmmo::SetAmmo(int iAmmo, bool bPlayAnimation)
{
	if (bPlayAnimation && m_iAmmo != iAmmo)
	{
		//const char *pszAnimation = (iAmmo == 0 ? "AmmoEmpty" : (iAmmo < 0 ? "AmmoDecreased" : "AmmoIncreased"));
		
		// Mulch: Disabling all hud animations except for health/armor
		//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(pszAnimation);
	}

	m_iAmmo = iAmmo;
	SetDisplayValue(iAmmo);
}

//=============================================================================
// CHudAmmoInfo
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoInfo::Init( void )
{
	Reset();
	ivgui()->AddTickSignal( GetVPanel(), 100 );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoInfo::VidInit( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoInfo::Reset( void )
{
	m_pWeaponIcon = new CHudTexture();
	m_pAmmoIcon = new CHudTexture();

	m_pWeapon = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get stuff!
//-----------------------------------------------------------------------------
void CHudAmmoInfo::OnTick()
{
	BaseClass::OnTick();

	if(!m_pFFPlayer)
		return;
	
	C_BaseCombatWeapon *lastWeapon = m_pWeapon;
	m_pWeapon = m_pFFPlayer->GetActiveWeapon();

	if (m_pWeapon == lastWeapon)
		return;

	if (!m_pWeapon)
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
	}
	else
	{
		if (m_pWeapon->GetSpriteInactive())
		{
			*m_pWeaponIcon = *m_pWeapon->GetSpriteInactive();

			// Change the font so it uses 28 size instead of 64
			m_pWeaponIcon->hFont = m_hWeaponIconFont;
			m_pWeaponIcon->bRenderUsingFont = true;
		}

		if (m_pWeapon->GetSpriteAmmo())
		{
			*m_pAmmoIcon = *m_pWeapon->GetSpriteAmmo();

			m_pAmmoIcon->hFont = m_hAmmoIconFont;
			m_pAmmoIcon->bRenderUsingFont = true;
		}

		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudAmmoInfo::Paint()
{ 
	if(m_pWeaponIcon)
	{
		// for widescreen stuff we take width scaled, then subtract the X not scaled (as we dont stretch the hud)
		// then we add the 44 not scaled (GetProportionalScaledValue is scaled due to height but not width)
		m_pWeaponIcon->DrawSelf( m_flWeaponIconX, m_flWeaponIconY, m_clrWeaponIconColor );
	}

	if(m_pAmmoIcon)
	{
		m_pAmmoIcon->DrawSelf( m_flAmmoIconX, m_flAmmoIconY, m_clrAmmoIconColor );
	}
}