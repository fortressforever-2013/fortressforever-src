//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_cellcount.cpp
//
// implementation of CHudCellCount class
//
#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

#include "iclientmode.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

#include "hudelement.h"

#include "convar.h"

#include "c_ff_player.h"
#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <c_playerresource.h>

#define INIT_CELLCOUNT -1

#define CELLCOUNT_BACKGROUND_TEXTURE "hud/CellCountBG"
#define CELLCOUNT_FOREGROUND_TEXTURE "hud/CellCountFG"

//-----------------------------------------------------------------------------
// Purpose: Cell count panel
//-----------------------------------------------------------------------------
class CHudCellCount : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudCellCount, vgui::Panel );

public:
	CHudCellCount( const char *pElementName );
	~CHudCellCount();
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void Paint();
	virtual bool ShouldDraw();
			void UpdateCellCount();
			void CacheTextures();

private:
	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsSmall" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );
	CPanelAnimationVar( Color, m_IconColor, "IconColor", "HUD_Tone_Default" );
	CPanelAnimationVar( Color, m_TextColor, "TextColor", "HUD_Tone_Default" );

	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, image_xpos, "image_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image_ypos, "image_ypos", "4", "proportional_float" );

	int		m_iCellCount;

	CHudTexture* m_pHudCellIcon;

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};	

DECLARE_HUDELEMENT( CHudCellCount );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudCellCount::CHudCellCount( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudCellCount" )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

CHudCellCount::~CHudCellCount()
{
	if( m_pBGTexture )
	{
		delete m_pBGTexture;
		m_pBGTexture = NULL;
	}

	if( m_pFGTexture )
	{
		delete m_pFGTexture;
		m_pFGTexture = NULL;
	}

	if( m_pHudCellIcon )
	{
		delete m_pHudCellIcon;
		m_pHudCellIcon = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::Init()
{
	Reset();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::Reset()
{
	m_iCellCount	= INIT_CELLCOUNT;

	// this shit fucking breaks when moved to CacheTextures() so i'm leaving it here
	// until we replace all font icons
	m_pHudCellIcon = new CHudTexture();
	m_pHudCellIcon->bRenderUsingFont = true;
	m_pHudCellIcon->hFont = m_hIconFont;
	m_pHudCellIcon->cCharacterInFont = '6';
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::VidInit()
{
	Reset();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudCellCount::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(CELLCOUNT_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(CELLCOUNT_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
bool CHudCellCount::ShouldDraw()
{
	if ( !CHudElement::ShouldDraw() )
		return false;
	
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if(pPlayer->GetClassSlot() != CLASS_ENGINEER)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCellCount::UpdateCellCount()
{
	// Fix for ToFFPlayer( NULL ) being called.
	if( !engine->IsInGame() )
		return;

	int newCells = 0;	

	C_FFPlayer *local = C_FFPlayer::GetLocalFFPlayer();
	if (!local)
		return;

	// Never below zero
	newCells = max( local->GetAmmoCount( AMMO_CELLS ), 0);

	// Only update the fade if we've changed cell count
	if ( newCells == m_iCellCount )
	{
		return;
	}

	// Play appropriate animation whether cell count has gone up or down
	if( newCells > m_iCellCount )
	{
		// Cell count went up
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "CellCountIncrease" );
	}
	else
	{
		// Cell count went down
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "CellCountDecrease" );
	}

	m_iCellCount = newCells;
}

void CHudCellCount::Paint()
{
	
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	UpdateCellCount();

	Color cColor = Color(255, 255, 255, 255);
	if ( g_PR )
		cColor = g_PR->GetTeamColor(pPlayer->GetTeamNumber());

	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, CELLCOUNT_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, CELLCOUNT_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	if (m_pHudCellIcon)
	{
		m_pHudCellIcon->DrawSelf( image_xpos, image_ypos, m_IconColor );
		
		// Get the class as a string
		wchar_t unicode[6];
		V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", m_iCellCount);

		// Draw text
		surface()->DrawSetTextFont( m_hTextFont );
		surface()->DrawSetTextColor( m_TextColor );
		surface()->DrawSetTextPos( text_xpos, text_ypos );
		surface()->DrawUnicodeString( unicode );
	}
}
