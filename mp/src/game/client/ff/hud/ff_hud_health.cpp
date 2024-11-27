//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// Health.cpp
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

#define INIT_HEALTH -1

#define HEALTH_BACKGROUND_TEXTURE "hud/HealthBG"
#define HEALTH_FOREGROUND_TEXTURE "hud/HealthFG"

extern Color GetCustomClientColor(int iPlayerIndex, int iTeamIndex/* = -1*/);

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
class CHudHealth : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE( CHudHealth, CHudNumericDisplay );

public:
	CHudHealth( const char *pElementName );
	~CHudHealth( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme* pScheme );
			void MsgFunc_Damage( bf_read &msg );
			void MsgFunc_PlayerAddHealth( bf_read &msg );
			void UpdateDisplay( void );
			void CacheTextures( void );

private:
	// old variables
	int		m_iHealth;
	int		m_bitsDamage;

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};	

DECLARE_HUDELEMENT( CHudHealth );
DECLARE_HUD_MESSAGE( CHudHealth, Damage );
DECLARE_HUD_MESSAGE( CHudHealth, PlayerAddHealth );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudHealth::CHudHealth( const char *pElementName ) : CHudElement( pElementName ), CHudNumericDisplay( NULL, "HudHealth" )
{
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudHealth::~CHudHealth()
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
void CHudHealth::Init()
{
	HOOK_HUD_MESSAGE( CHudHealth, Damage );
	HOOK_HUD_MESSAGE( CHudHealth, PlayerAddHealth );
	Reset();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// blame CHudNumericDisplay
	Panel::SetFgColor( GetSchemeColor( "HUD_Tone_Default", Color( 199, 219, 255, 215 ), pScheme ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::Reset()
{
	BaseClass::Reset();

	m_iHealth		= INIT_HEALTH;
	m_bitsDamage	= 0;

	SetShouldDisplayValue(false);
	SetDisplayValue( m_iHealth );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::VidInit()
{
	CacheTextures();
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudHealth::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(HEALTH_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(HEALTH_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: paint maybe?
//-----------------------------------------------------------------------------
void CHudHealth::Paint()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	UpdateDisplay();

	Color cColor = GetCustomClientColor( -1, pPlayer->GetTeamNumber() );
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, HEALTH_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, HEALTH_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// then the numbers
	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::UpdateDisplay()
{
	int iHealth = 0;
	int iMaxHealth = 0;	

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	// Never below zero
	iHealth = max( pPlayer->GetHealth(), 0 );
	iMaxHealth = pPlayer->GetMaxHealth();

	// Hullucination
	if (pPlayer->m_iHallucinationIndex)
	{
		iHealth = pPlayer->m_iHallucinationIndex * 4;
	}

	// Only update the fade if we've changed health
	if ( iHealth == m_iHealth )
		return;

	// Get a health percentage
	float flHealthPercent = ( float )iHealth / ( float )iMaxHealth;

	// Play appropriate animation whether health has gone up or down
	if( iHealth > m_iHealth )
	// Health went up
	{
		if( flHealthPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthIncreaseBelow25" );
		}
		else
		{
			if( flHealthPercent >= 1.0f )
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthIncreaseAbove100" );
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthIncrease" );
		}		
	}
	else
	// Health went down or didn't change
	{
		if( flHealthPercent < 0.25f )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthBelow25" );
		}
	}

	m_iHealth = iHealth;

	SetDisplayValue( m_iHealth );
	SetShouldDisplayValue(true);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_Damage( bf_read &msg )
{
	msg.ReadByte(); //waste the armour msg
	int iHealthTaken = msg.ReadByte();

	// Actually took damage?
	if ( iHealthTaken > 0 )
	{
		// start the animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "HealthDamageTaken" );

		//make the display update instantly when we take health damage
		UpdateDisplay();
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudHealth::MsgFunc_PlayerAddHealth( bf_read &msg )
{
	int iHealthAdded = msg.ReadByte();

	// Actually took damage?
	if ( iHealthAdded > 0 )
	{
		//make the display update instantly when we get health
		UpdateDisplay();
	}
}
