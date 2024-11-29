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
#include <c_playerresource.h>

#define LOCATION_BACKGROUND_TEXTURE "hud/HudLocationBG"
#define LOCATION_FOREGROUND_TEXTURE "hud/HudLocationFG"

//=============================================================================
//
//	class CHudLocation
//
//=============================================================================
class CHudLocation : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudLocation, vgui::Panel );

public:
	CHudLocation( const char *pElementName );
	~CHudLocation( void );

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Paint( void );

	void MsgFunc_SetPlayerLocation( bf_read &msg );
	void CacheTextures( void );

protected:
	wchar_t		m_pText[ 1024 ];	// Unicode text buffer
	int			m_iTeam;

private:

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "20", "proportional_float" );

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};

DECLARE_HUDELEMENT( CHudLocation );
DECLARE_HUD_MESSAGE( CHudLocation, SetPlayerLocation );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudLocation::CHudLocation(const char* pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudLocation")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudLocation::~CHudLocation(void)
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

void CHudLocation::Init( void )
{
	HOOK_HUD_MESSAGE( CHudLocation, SetPlayerLocation );

	m_pText[ 0 ] = '\0';
	CacheTextures();
}

void CHudLocation::VidInit( void )
{	
	SetPaintBackgroundEnabled( true );
	m_pText[ 0 ] = '\0'; // Bug 0000293: clear location text buffer on map change
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudLocation::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(LOCATION_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(LOCATION_FOREGROUND_TEXTURE);
	}
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
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

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
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, LOCATION_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, LOCATION_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	if( m_pText )
	{
		surface()->DrawSetTextFont( m_hTextFont );

		Color cColor;
		SetColorByTeam( m_iTeam, cColor );

		surface()->DrawSetTextColor( cColor.r(), cColor.g(), cColor.b(), 255 );
		surface()->DrawSetTextPos( text1_xpos, text1_ypos );

		for( wchar_t *wch = m_pText; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}

	BaseClass::Paint();
}