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

#define AMMO_BACKGROUND_TEXTURE "hud/AmmoBG"
#define AMMO_FOREGROUND_TEXTURE "hud/AmmoFG"
#define AMMOCLIP_BACKGROUND_TEXTURE "hud/AmmoClipBG"
#define AMMOCLIP_FOREGROUND_TEXTURE "hud/AmmoClipFG"
#define AMMOINFO_BACKGROUND_TEXTURE "hud/AmmoInfoBG"
#define AMMOINFO_FOREGROUND_TEXTURE "hud/AmmoInfoFG"

extern Color GetCustomClientColor(int iPlayerIndex, int iTeamIndex/* = -1*/);

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudAmmo, CHudNumericDisplay);

public:
	
	CHudAmmo( const char *pElementName );
	~CHudAmmo( void ); 
	
	virtual void Init();
	virtual void Reset();
	virtual void VidInit( void );
	virtual void ApplySchemeSettings( IScheme* pScheme );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );
			void CacheTextures( void );

protected:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;

	virtual void SetAmmo(int ammo, bool playAnimation);
	virtual int GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon);

	int		m_iAmmo;

private:
	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo(const char *pElementName) : BaseClass(NULL, "HudAmmo"), CHudElement(pElementName)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED | HIDEHUD_WEAPONSELECTION);

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudAmmo::~CHudAmmo()
{
	if (m_pBGTexture)
	{
		delete m_pBGTexture;
		m_pBGTexture = NULL;
	}

	if (m_pFGTexture)
	{
		delete m_pFGTexture;
		m_pFGTexture = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// blame CHudNumericDisplay
	Panel::SetFgColor( GetSchemeColor( "HUD_Tone_Default", Color( 199, 219, 255, 215 ), pScheme ) );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmo::Init()
{
	m_hCurrentActiveWeapon = NULL;
	m_iAmmo = -1;
	SetLabelText(L"");
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmo::VidInit()
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CHudAmmo::GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon)
{
	if (!pWeapon || !pPlayer)
		return -1;

	return pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmo::Reset()
{
	m_iAmmo = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudAmmo::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(AMMO_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(AMMO_FOREGROUND_TEXTURE);
	}
}

bool CHudAmmo::ShouldDraw()
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer)
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	int iAmmo = -1;

	if (pWeapon && pWeapon->UsesPrimaryAmmo())
	{
		iAmmo = GetPlayerAmmo(pPlayer, pWeapon);
	}

	if ( iAmmo < 0 )
		return false;

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

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmo::Paint()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = GetCustomClientColor( -1, pPlayer->GetTeamNumber() );
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, AMMO_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, AMMO_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// then the numbers
	BaseClass::Paint();
}

DECLARE_HUDELEMENT(CHudAmmo);

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level in clip
//-----------------------------------------------------------------------------
class CHudAmmoClip : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudAmmoClip, CHudNumericDisplay);

public:
	
	CHudAmmoClip( const char *pElementName );
	~CHudAmmoClip( void );
	
	virtual void Init();
	virtual void Reset();
	virtual void VidInit( void );
	virtual void ApplySchemeSettings( IScheme* pScheme );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );
			void CacheTextures( void );

protected:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;

	virtual void SetAmmo(int ammo, bool playAnimation);
	virtual int GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon);

	int		m_iAmmo;

private:
	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmoClip::CHudAmmoClip(const char *pElementName) : BaseClass(NULL, "HudAmmoClip"), CHudElement(pElementName)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED | HIDEHUD_WEAPONSELECTION);

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudAmmoClip::~CHudAmmoClip()
{
	if (m_pBGTexture)
	{
		delete m_pBGTexture;
		m_pBGTexture = NULL;
	}

	if (m_pFGTexture)
	{
		delete m_pFGTexture;
		m_pFGTexture = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmoClip::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// blame CHudNumericDisplay
	Panel::SetFgColor( GetSchemeColor( "HUD_Tone_Default", Color( 199, 219, 255, 215 ), pScheme ) );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoClip::Init()
{
	m_hCurrentActiveWeapon = NULL;
	m_iAmmo = -1;
	SetLabelText(L"");
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoClip::VidInit()
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CHudAmmoClip::GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon)
{
	if (!pWeapon )
		return -1;

	return pWeapon->Clip1();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoClip::Reset()
{
	m_iAmmo = -1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoClip::SetAmmo(int iAmmo, bool bPlayAnimation)
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

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudAmmoClip::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(AMMOCLIP_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(AMMOCLIP_FOREGROUND_TEXTURE);
	}
}

bool CHudAmmoClip::ShouldDraw()
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer)
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();

	int iAmmo = -1;

	if (pWeapon && pWeapon->UsesPrimaryAmmo())
	{
		iAmmo = GetPlayerAmmo(pPlayer, pWeapon);
	}

	if ( iAmmo < 0 )
		return false;

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

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoClip::Paint()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = GetCustomClientColor( -1, pPlayer->GetTeamNumber() );
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, AMMOCLIP_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, AMMOCLIP_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// then the numbers
	BaseClass::Paint();
}

DECLARE_HUDELEMENT(CHudAmmoClip);

//-----------------------------------------------------------------------------
// Purpose: Displays current weapon & ammo
//-----------------------------------------------------------------------------
class CHudAmmoInfo : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE( CHudAmmoInfo, Panel );

public:
	CHudAmmoInfo( const char* pElementName );
	~CHudAmmoInfo( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );
			void CacheTextures( void );

private:
	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;

	CHudTexture* m_pWeaponIcon;
	CHudTexture* m_pAmmoIcon;
	C_BaseCombatWeapon* m_pWeapon;

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsHUD" );
	CPanelAnimationVar( vgui::HFont, m_hAmmoIconFont, "AmmoFont", "WeaponIconsHUD" );

	CPanelAnimationVarAliasType( float, ammo_xpos, "ammo_xpos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, ammo_ypos, "ammo_ypos", "32", "proportional_float" );

	CPanelAnimationVarAliasType( float, weapon_xpos, "weapon_xpos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, weapon_ypos, "weapon_ypos", "32", "proportional_float" );
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmoInfo::CHudAmmoInfo( const char* pElementName ) : CHudElement( pElementName ), Panel( NULL, "HudAmmoInfo" )
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED | HIDEHUD_WEAPONSELECTION);

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;

	m_pWeaponIcon = NULL;
	m_pAmmoIcon = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudAmmoInfo::~CHudAmmoInfo()
{
	if (m_pBGTexture)
	{
		delete m_pBGTexture;
		m_pBGTexture = NULL;
	}

	if (m_pFGTexture)
	{
		delete m_pFGTexture;
		m_pFGTexture = NULL;
	}

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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoInfo::Init()
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoInfo::VidInit()
{
	CacheTextures();
	m_pWeapon = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache textures
//-----------------------------------------------------------------------------
void CHudAmmoInfo::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(AMMOINFO_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(AMMOINFO_FOREGROUND_TEXTURE);
	}

	if (!m_pWeaponIcon)
		m_pWeaponIcon = new CHudTexture();

	if (!m_pAmmoIcon)
		m_pAmmoIcon = new CHudTexture();
}

bool CHudAmmoInfo::ShouldDraw()
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return false;
	
	C_BaseCombatWeapon *lastWeapon = m_pWeapon;
	m_pWeapon = pPlayer->GetActiveWeapon();

	if (!m_pWeapon)
		return false;

	if (m_pWeapon != lastWeapon)
	{
		if (m_pWeapon->GetSpriteInactive())
		{
			*m_pWeaponIcon = *m_pWeapon->GetSpriteInactive();

			// Change the font so it uses 28 size instead of 64
			m_pWeaponIcon->hFont = m_hIconFont;
			m_pWeaponIcon->bRenderUsingFont = true;
		}
		else
			*m_pWeaponIcon = CHudTexture();

		if (m_pWeapon->GetSpriteAmmo())
		{
			*m_pAmmoIcon = *m_pWeapon->GetSpriteAmmo();
			m_pAmmoIcon->hFont = m_hAmmoIconFont;
		}
		else
			*m_pAmmoIcon = CHudTexture();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudAmmoInfo::Paint()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = GetCustomClientColor( -1, pPlayer->GetTeamNumber() );
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, AMMOINFO_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, AMMOINFO_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	if(m_pWeaponIcon)
	{
		m_pWeaponIcon->DrawSelf(weapon_xpos, weapon_ypos, GetFgColor());
	}

	if(m_pAmmoIcon)
	{
		m_pAmmoIcon->DrawSelf(ammo_xpos, ammo_ypos, GetFgColor());
	}
}


DECLARE_HUDELEMENT(CHudAmmoInfo);