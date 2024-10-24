//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_playerscore.cpp
//	@author Michael Parker (AfterShock)
//	@date 27/05/2007
//	@brief Hud Player Score field - with details of your latest score
//
//	REVISIONS
//	---------
//	15/06/2007, AfterShock: 
//		First created (from ff_hud_spydisguise.cpp)

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

//#include <KeyValues.h>
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
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerScore : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerScore, vgui::FFPanel );

	CHudPlayerScore( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerScore" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudPlayerScore( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );
	void MsgFunc_SetPlayerLatestFortPoints( bf_read &msg );
	
protected:
	wchar_t		m_pTextScore[ 1024 ];	// Unicode text buffer
	wchar_t		m_pTextDesc[ 1024 ];	// Unicode text buffer
	int			m_iLatestFortPoints;

private:
	// Stuff we need to know	
		CPanelAnimationVar( vgui::HFont, m_hScoreFont, "ScoreFont", "Default" );
		CPanelAnimationVar( vgui::HFont, m_hDescFont, "DescFont", "Default" );

	CPanelAnimationVarAliasType( float, ScoreFont_xpos, "ScoreFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, ScoreFont_ypos, "ScoreFont_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, DescFont_xpos, "DescFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, DescFont_ypos, "DescFont_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudPlayerScore );
DECLARE_HUD_MESSAGE( CHudPlayerScore, SetPlayerLatestFortPoints );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerScore::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerScore, SetPlayerLatestFortPoints );

	m_pTextScore[ 0 ] = '\0';
	m_pTextDesc[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerScore::VidInit( void )
{
	SetPaintBackgroundEnabled( true );
	m_pTextScore[ 0 ] = '\0'; // Bug 0000293: clear location text buffer on map change	
	m_pTextDesc[ 0 ] = '\0';
	
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
bool CHudPlayerScore::ShouldDraw() 
{ 
   if( !engine->IsInGame() ) 
      return false; 

   C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 

   if( !pPlayer ) 
      return false; 

   if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) ) 
      return false; 

   return true; 
} 

void CHudPlayerScore::MsgFunc_SetPlayerLatestFortPoints( bf_read &msg )
{
	// Read description string
	char szString[ 1024 ];
	msg.ReadString( szString, sizeof( szString ) );

	// Read int and convert to string
	char szString2[ 1024 ];
	Q_snprintf( szString2, sizeof(szString2), "+%i", msg.ReadShort() );
	//itoa(msg.ReadShort(), szString2, 10);

	// Convert string to unicode
	wchar_t *pszTemp = vgui::localize()->Find( szString );
	if( pszTemp )
		wcscpy( m_pTextDesc, pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( szString, m_pTextDesc, sizeof( m_pTextDesc ) );

	// convert int-string to unicode
	vgui::localize()->ConvertANSIToUnicode( szString2, m_pTextScore, sizeof( m_pTextScore ) );

	// play animation (new points value)
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewFortPoints" );
	
	//DevMsg( "[Location] Team: %i, String: %s\n", iTeam, szString );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerScore::Paint() 
{ 
   FFPanel::Paint(); // Draws the background glyphs 

      C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
      if ( !pPlayer ) 
         return; 

		if( m_pTextDesc[ 0 ] != '\0' )
		{
			surface()->DrawSetTextFont( m_hDescFont );
			surface()->DrawSetTextColor( GetFgColor() );
			surface()->DrawSetTextPos( DescFont_xpos, DescFont_ypos );

			for( wchar_t *wch = m_pTextDesc; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );
		}

		if( m_pTextScore[ 0 ] != '\0' )
		{
			surface()->DrawSetTextFont( m_hScoreFont );
			surface()->DrawSetTextColor( GetFgColor() );
			surface()->DrawSetTextPos( ScoreFont_xpos, ScoreFont_xpos );

			for( wchar_t *wch = m_pTextScore; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );
		}
}
