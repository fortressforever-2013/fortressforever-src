//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_armor.cpp
//
// implementation of CHudHealth class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"

#include "convar.h"

#include "c_ff_player.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <c_playerresource.h>

#define INIT_ARMOR -1

#define ARMOR_BACKGROUND_TEXTURE "hud/ArmorBG"
#define ARMOR_FOREGROUND_TEXTURE "hud/ArmorFG"

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudArmor : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudArmor, CHudNumericDisplay );

public:
	CHudArmor( const char *pElementName );
	~CHudArmor( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme* pScheme );
			void MsgFunc_Damage( bf_read &msg );
			void MsgFunc_PlayerAddArmor( bf_read &msg );
			void UpdateDisplay( void );
			void CacheTextures( void );

private:
	// old variables
	int		m_iArmor;

	int		m_bitsDamage;

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};	

DECLARE_HUDELEMENT( CHudArmor );
DECLARE_HUD_MESSAGE( CHudArmor, Damage );
DECLARE_HUD_MESSAGE( CHudArmor, PlayerAddArmor );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudArmor::CHudArmor( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay(NULL, "HudArmor")
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudArmor::~CHudArmor()
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
void CHudArmor::Init()
{
	HOOK_HUD_MESSAGE( CHudArmor, Damage );
	HOOK_HUD_MESSAGE( CHudArmor, PlayerAddArmor );
	Reset();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// blame CHudNumericDisplay
	Panel::SetFgColor( GetSchemeColor( "HUD_Tone_Default", Color( 199, 219, 255, 215 ), pScheme ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::Reset()
{
	BaseClass::Reset();
	m_iArmor		= INIT_ARMOR;
	m_bitsDamage	= 0;

	SetShouldDisplayValue(false);
	SetDisplayValue(m_iArmor);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::VidInit()
{
	CacheTextures();
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudArmor::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(ARMOR_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(ARMOR_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: paint maybe?
//-----------------------------------------------------------------------------
void CHudArmor::Paint()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	UpdateDisplay();

	ConVarRef cl_teamcolourhud("cl_teamcolourhud");
	Color cColor = Color(255, 255, 255, 255);
	if( cl_teamcolourhud.GetBool() )
	{
		if ( g_PR )
			cColor = g_PR->GetTeamColor( pPlayer->GetTeamNumber() );
	}
	else
	{
		cColor = COLOR_GREY;
	}
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, ARMOR_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, ARMOR_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// then the numbers
	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::UpdateDisplay()
{
	int iArmor = 0;
	int iMaxArmor = 0;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	
	if (!pPlayer)
		return;

	// Never below zero
	iArmor = max(pPlayer->GetArmor(), 0 );
	iMaxArmor = pPlayer->GetMaxArmor();

	// Hullucination
	if (pPlayer->m_iHallucinationIndex)
	{
		iArmor = pPlayer->m_iHallucinationIndex * 4;
	}

	// Only update the fade if we've changed armor
	if ( iArmor == m_iArmor )
		return;

	// Get a health percentage
	float flArmorPercent = ( float )iArmor / ( float )iMaxArmor;

	// Play appropriate animation whether armor has gone up or down
	if( iArmor > m_iArmor )
	// Armor went up
	{
		if( flArmorPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorIncreaseBelow25" );
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorIncrease" );
		}		
	}
	else
	// Armor went down or didn't change
	{
		if( flArmorPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorBelow25" );
		}
	}

	m_iArmor = iArmor;

	SetDisplayValue( m_iArmor );
	SetShouldDisplayValue(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::MsgFunc_Damage( bf_read &msg )
{
	int iArmorTaken = msg.ReadByte();	// armor

	// Actually took damage?
	if ( iArmorTaken > 0 )
	{
		// start the animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "ArmorDamageTaken" );

		//make the display update instantly when we take armour damage
		UpdateDisplay();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudArmor::MsgFunc_PlayerAddArmor( bf_read &msg )
{
	int iArmorAdded = msg.ReadByte();	// armor

	// Actually took damage?
	if ( iArmorAdded > 0 )
	{
		//make the display update instantly when we get armour
		UpdateDisplay();
	}
}
