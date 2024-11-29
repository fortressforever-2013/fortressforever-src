//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_spydisguise.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 09/01/2006
//	@brief Hud Disguise Indicator
//
//	REVISIONS
//	---------
//	09/01/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

//#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "c_ff_player.h"
#include "ff_utils.h"
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//extern ConVar ffdev_overpressure_delay;
#define OVERPRESSURE_COOLDOWN 16	//ffdev_overpressure_delay.GetFloat()

#define OVERPRESSURE_BAR_BACKGROUND_TEXTURE "hud/OverpressureBarBG"
#define OVERPRESSURE_BAR_FOREGROUND_TEXTURE "hud/OverpressureBarFG"

#define OVERPRESSURE_BAR_PROGRESS_TEXTURE "hud/OverpressureBarProgress"

//-----------------------------------------------------------------------------
// Purpose: Displays overpressure charge on the HUD
//-----------------------------------------------------------------------------
class CHudOverpressure : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudOverpressure, vgui::Panel );

public:
	CHudOverpressure(const char* pElementName);
	virtual ~CHudOverpressure(void);

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Paint(void);
	virtual bool ShouldDraw(void);

	void	CacheTextures(void);

private:
	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;

	CPanelAnimationVarAliasType(float, bar_xpos, "bar_xpos", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_ypos, "bar_ypos", "4", "proportional_float");

	CPanelAnimationVarAliasType(float, bar_width, "bar_width", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_height, "bar_height", "4", "proportional_float");
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudOverpressure::CHudOverpressure(const char* pElementName) : vgui::Panel(NULL, "HudOverpressure"), CHudElement(pElementName)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudOverpressure::~CHudOverpressure()
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
void CHudOverpressure::Init(void)
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudOverpressure::VidInit( void )
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudOverpressure::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(OVERPRESSURE_BAR_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(OVERPRESSURE_BAR_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudOverpressure::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw() )
		return false;

	if( !engine->IsInGame() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return false;

	if( pPlayer->GetClassSlot() != CLASS_HWGUY || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return false;

	if( pPlayer->m_flNextClassSpecificSkill <= gpGlobals->curtime )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudOverpressure::Paint( void )
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = Color(255, 255, 255, 255);
	if ( g_PR )
		cColor = g_PR->GetTeamColor(pPlayer->GetTeamNumber());

	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, OVERPRESSURE_BAR_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, OVERPRESSURE_BAR_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// Let's calculate and draw the disguising progress bar
	//New cloak percent timer -GreenMushy
	float iProgressPercent = ( OVERPRESSURE_COOLDOWN - (pPlayer->m_flNextClassSpecificSkill - gpGlobals->curtime) ) / ( OVERPRESSURE_COOLDOWN );

	// Draw progress bar
	surface()->DrawSetTextureFile( m_pFGTexture->textureId, OVERPRESSURE_BAR_PROGRESS_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( bar_xpos, bar_ypos, bar_width * iProgressPercent, bar_height );
}

DECLARE_HUDELEMENT(CHudOverpressure);
