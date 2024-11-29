//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_roundinfo.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 11/04/2006
//	@brief Hud Round Info
//
//	REVISIONS
//	---------
//	11/04/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

//#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include <vgui_controls/Panel.h>
#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_gamerules.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <c_playerresource.h>

using namespace vgui;

#define ROUNDINFO_BACKGROUND_TEXTURE "hud/RoundInfoBG"
#define ROUNDINFO_FOREGROUND_TEXTURE "hud/RoundInfoFG"

extern ConVar mp_timelimit;

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudRoundInfo : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudRoundInfo, vgui::Panel );

	CHudRoundInfo( const char* pElementName );
	virtual ~CHudRoundInfo( void );

	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void VidInit( void );
	virtual void Init( void );

	void	CacheTextures(void);

private:
	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hMapNameFont, "MapNameFont", "HUD_TextRoundInfo" );
	CPanelAnimationVar( Color, m_hMapNameColor, "MapNameColor", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( float, m_flMapNameX, "MapNameX", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flMapNameY, "MapNameY", "3", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hTimerFont, "TimerFont", "HUD_TextRoundInfo" );
	CPanelAnimationVar( Color, m_hTimerColor, "TimerColor", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( float, m_flTimerX, "TimerX", "12", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTimerY, "TimerY", "23", "proportional_float" );
	
	wchar_t		m_szMapName[ MAX_PATH ];
	wchar_t		m_szRoundTimer[ 8 ];

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};

DECLARE_HUDELEMENT( CHudRoundInfo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudRoundInfo::CHudRoundInfo(const char* pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudRoundInfo")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_UNASSIGNED);

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CHudRoundInfo::~CHudRoundInfo( void )
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
// Purpose: Initialization
//-----------------------------------------------------------------------------
void CHudRoundInfo::Init( void )
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache textures
//-----------------------------------------------------------------------------
void CHudRoundInfo::CacheTextures(void)
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(ROUNDINFO_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(ROUNDINFO_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudRoundInfo::VidInit( void )
{
	// Set up map name
	g_pVGuiLocalize->ConvertANSIToUnicode( UTIL_GetFormattedMapName(), m_szMapName, sizeof( m_szMapName ) );

	int iMapWide, iMapTall;
	surface()->GetTextSize( m_hMapNameFont, m_szMapName, iMapWide, iMapTall );

	m_flMapNameX = ( GetWide() / 2 ) - ( iMapWide / 2 );

	// Set up round timer
	g_pVGuiLocalize->ConvertANSIToUnicode( "00:00", m_szRoundTimer, sizeof( m_szRoundTimer ) );

	int iTimerWide, iTimerTall;
	surface()->GetTextSize( m_hTimerFont, m_szRoundTimer, iTimerWide, iTimerTall );

	m_flTimerX = ( GetWide() / 2 ) - ( iTimerWide / 2 );

	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: To draw or not to draw
//-----------------------------------------------------------------------------
bool CHudRoundInfo::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw() )
		return false;

	// Convert to minutes
	float flMinutesLeft = mp_timelimit.GetFloat() * 60;

	// Figure out new timer value
	char szTimer[ 8 ];
	int timer = flMinutesLeft - ( gpGlobals->curtime - FFGameRules()->GetRoundStart() );
	if( timer <= 0 )
		Q_snprintf( szTimer, sizeof( szTimer ), "00:00" );
	else
		Q_snprintf( szTimer, sizeof( szTimer ), "%d:%02d", ( timer / 60 ), ( timer % 60 ) );

	g_pVGuiLocalize->ConvertANSIToUnicode( szTimer, m_szRoundTimer, sizeof( m_szRoundTimer ) );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudRoundInfo::Paint( void )
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

	surface()->DrawSetTextureFile( m_pBGTexture->textureId, ROUNDINFO_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, ROUNDINFO_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// Draw map text
	surface()->DrawSetTextFont( m_hMapNameFont );
	surface()->DrawSetTextColor( m_hMapNameColor );
	surface()->DrawSetTextPos( m_flMapNameX, m_flMapNameY );
	surface()->DrawUnicodeString( m_szMapName );

	// Draw round timer text
	surface()->DrawSetTextFont( m_hTimerFont );
	surface()->DrawSetTextColor( m_hTimerColor );
	surface()->DrawSetTextPos( m_flTimerX, m_flTimerY );
	surface()->DrawUnicodeString( m_szRoundTimer );

	BaseClass::Paint();
}
