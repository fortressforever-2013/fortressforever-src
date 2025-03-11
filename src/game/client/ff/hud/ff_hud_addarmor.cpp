

//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_scorelatest.cpp
//	@author Michael Parker (AfterShock)
//	@date 30/06/2007
//	@brief Hud Player Armor field - with details of your latest score
//
//	REVISIONS
//	---------
//	30/06/2007, AfterShock: 
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
static ConVar hud_addarmor("hud_addarmor", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visibility of added armor notices.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerAddArmor : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerAddArmor, vgui::FFPanel );

	CHudPlayerAddArmor( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerAddArmor" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

		m_flStartTime = 0.0f;
		m_flDuration = 0.0f;
		m_iArmorToAdd = 0;
	}

	virtual ~CHudPlayerAddArmor( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );

	void MsgFunc_PlayerAddArmor( bf_read &msg );

protected:
	int			m_iArmorToAdd;

private:

	float		m_flStartTime;		// When the message was recevied
	float		m_flDuration;		// Duration of the message

	// Stuff we need to know	
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "text_font", "HudAddHealth" );
};

DECLARE_HUDELEMENT( CHudPlayerAddArmor );
DECLARE_HUD_MESSAGE( CHudPlayerAddArmor, PlayerAddArmor );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerAddArmor, PlayerAddArmor );
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::VidInit(void)
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::Reset( void )
{
	SetPaintBackgroundEnabled( false );

	m_flStartTime = 0.0f;
	m_flDuration = 0.0f;
	m_iArmorToAdd = 0;
}

void CHudPlayerAddArmor::MsgFunc_PlayerAddArmor( bf_read &msg )
{
	// Read int and convert to string
	const int ptVal = msg.ReadShort();
	if(ptVal==0)
		return;

	// play animation (new points value)
	if(ptVal > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewAddArmor" );
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewSubtractArmor" );
	}

	m_iArmorToAdd = ptVal;

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = 3.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerAddArmor::Paint() 
{
	if( !hud_addarmor.GetBool() )
		return;

	if ( m_flStartTime + m_flDuration < gpGlobals->curtime )
		return;

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( GetFgColor() );
	surface()->DrawSetTextPos( 0, 0 );

	wchar_t wBuf[20];

	V_swprintf_safe( wBuf, L"%ls%d", m_iArmorToAdd > 0 ? L"+" : L"", m_iArmorToAdd );

	vgui::surface()->DrawPrintText( wBuf, V_wcslen( wBuf ), FONT_DRAW_NONADDITIVE );
}
