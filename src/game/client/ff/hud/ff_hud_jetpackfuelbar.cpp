#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "ff_panel.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;


//-----------------------------------------------------------------------------
// Purpose: Displays jetpack fuel remaining on the HUD
//-----------------------------------------------------------------------------
class CHudJetpackFuelBar : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudJetpackFuelBar, vgui::FFPanel );

	CHudJetpackFuelBar( const char *pElementName ) : vgui::FFPanel( NULL, "HudJetpackFuelBar" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

		m_pProgressBarTexture = NULL;
	}

	virtual ~CHudJetpackFuelBar( void )
	{
	}

	virtual void ApplySettings( KeyValues *inResourceData )
	{
		const char* pszTexture = inResourceData->GetString( "bar_texture", NULL );
		m_pProgressBarTexture = ( pszTexture ? gHUD.GetIcon( pszTexture ) : NULL );

		BaseClass::ApplySettings( inResourceData );
	}

	virtual void Paint( void );
	virtual bool ShouldDraw( void );

private:
	CPanelAnimationVarAliasType( float, bar_x_offset, "bar_x_offset", "3", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_y_offset, "bar_y_offset", "3", "proportional_float" );

	CPanelAnimationVar( Color, bar_color, "bar_color", "HUD_Tone_Default" );

	CHudTexture* m_pProgressBarTexture;
};

//-----------------------------------------------------------------------------
// Purpose: Decides when to draw and when to not
//-----------------------------------------------------------------------------
bool CHudJetpackFuelBar::ShouldDraw( void )
{
	if( !engine->IsInGame() )
		return false;

	if( !CHudElement::ShouldDraw() )
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

	if( !pPlayer )
		return;

	float iProgressPercent = pPlayer->m_iJetpackFuel / 200.0f;

	int offsetX = ( GetWide() - bar_x_offset * 2 );
	int offsetY = ( GetTall() - bar_y_offset * 2 );

	if ( m_pProgressBarTexture )
		m_pProgressBarTexture->DrawSelf( bar_x_offset, bar_y_offset, offsetX * iProgressPercent, offsetY, bar_color );
}

DECLARE_HUDELEMENT(CHudJetpackFuelBar);
