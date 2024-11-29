#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "c_ff_player.h"
#include "ff_utils.h"
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define JETPACK_FUEL_BAR_BACKGROUND_TEXTURE "hud/JetpackFuelBarBG"
#define JETPACK_FUEL_BAR_FOREGROUND_TEXTURE "hud/JetpackFuelBarFG"

#define JETPACK_FUEL_BAR_PROGRESS_TEXTURE "hud/JetpackFuelBarProgress"

//-----------------------------------------------------------------------------
// Purpose: Displays jetpack fuel remaining on the HUD
//-----------------------------------------------------------------------------
class CHudJetpackFuelBar : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudJetpackFuelBar, vgui::Panel );

	CHudJetpackFuelBar( const char *pElementName );

	virtual ~CHudJetpackFuelBar( void );

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );

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
CHudJetpackFuelBar::CHudJetpackFuelBar( const char *pElementName ) : vgui::Panel( NULL, "HudJetpackFuelBar" ), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudJetpackFuelBar::~CHudJetpackFuelBar()
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
void CHudJetpackFuelBar::Init( void )
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudJetpackFuelBar::VidInit( void )
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudJetpackFuelBar::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(JETPACK_FUEL_BAR_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(JETPACK_FUEL_BAR_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Defines when to draw and when to not
//-----------------------------------------------------------------------------
bool CHudJetpackFuelBar::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw() )
		return false;

	if( !engine->IsInGame() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return false;

	if( pPlayer->GetClassSlot() != CLASS_PYRO || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) || !pPlayer->m_bCanUseJetpack )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudJetpackFuelBar::Paint( void )
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = Color(255, 255, 255, 255);
	if ( g_PR )
		cColor = g_PR->GetTeamColor(pPlayer->GetTeamNumber());

	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, JETPACK_FUEL_BAR_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, JETPACK_FUEL_BAR_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	float iProgressPercent = pPlayer->m_iJetpackFuel / 200.0f;

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, JETPACK_FUEL_BAR_PROGRESS_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( bar_xpos, bar_ypos, bar_width * iProgressPercent, bar_height );
}

DECLARE_HUDELEMENT(CHudJetpackFuelBar);
