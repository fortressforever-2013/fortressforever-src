//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_location.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 02/20/2006
//	@brief client side Hud Location Info
//
//	REVISIONS
//	---------
//	02/20/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_panel.h"

int g_iHudLocation2Pos = 0;

//=============================================================================
//
//	class CHudLocation
//
//=============================================================================
class CHudLocation : public CHudElement, public vgui::FFPanel
{
private:
	DECLARE_CLASS_SIMPLE( CHudLocation, vgui::FFPanel );

public:
	CHudLocation( const char *pElementName ) : vgui::FFPanel( NULL, "HudLocation" ), CHudElement( pElementName )
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);
	}

	~CHudLocation( void ) {}

	void Init( void );
	void VidInit( void );
	void Paint( void );

	void MsgFunc_SetPlayerLocation( bf_read &msg );

protected:
	wchar_t		m_pText[ 1024 ];	// Unicode text buffer
	int			m_iTeam;

private:

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );

	CPanelAnimationVarAliasType( float, text_xpos, "text_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text_ypos, "text_ypos", "20", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudLocation );
DECLARE_HUD_MESSAGE( CHudLocation, SetPlayerLocation );

void CHudLocation::Init( void )
{
	HOOK_HUD_MESSAGE( CHudLocation, SetPlayerLocation );

	m_pText[ 0 ] = '\0';
}

void CHudLocation::VidInit( void )
{	
	SetPaintBackgroundEnabled( true );
	m_pText[ 0 ] = '\0'; // Bug 0000293: clear location text buffer on map change	
	g_iHudLocation2Pos = 0;
}

void CHudLocation::MsgFunc_SetPlayerLocation( bf_read &msg )
{
	char szString[ 1024 ];
	msg.ReadString( szString, sizeof( szString ) );

	m_iTeam = msg.ReadShort();

	wchar_t *pszTemp = g_pVGuiLocalize->Find( szString );
	if( pszTemp )
		wcscpy( m_pText, pszTemp );
	else
		g_pVGuiLocalize->ConvertANSIToUnicode( szString, m_pText, sizeof( m_pText ) );

	//DevMsg( "[Location] Team: %i, String: %s\n", iTeam, szString );
}

void CHudLocation::Paint( void )
{
	if( m_pText )
	{
		surface()->DrawSetTextFont( m_hTextFont );

		Color cColor;
		SetColorByTeam( m_iTeam, cColor );

		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );
		surface()->DrawSetTextPos( text_xpos, text_ypos );

		for( wchar_t *wch = m_pText; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}

	BaseClass::Paint();
}