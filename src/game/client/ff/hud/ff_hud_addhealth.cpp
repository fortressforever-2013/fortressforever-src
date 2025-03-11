

//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_scorelatest.cpp
//	@author Michael Parker (AfterShock)
//	@date 30/06/2007
//	@brief Hud Player Health field - with details of your latest score
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
static ConVar hud_addhealth("hud_addhealth", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visibility of added health notices.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerAddHealth : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerAddHealth, vgui::FFPanel );

	CHudPlayerAddHealth( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerAddHealth" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED  );
		
		m_flStartTime = 0.0f;
		m_flDuration = 0.0f;
		m_iHealthToAdd = 0;
	}

	virtual ~CHudPlayerAddHealth( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );

	void MsgFunc_PlayerAddHealth( bf_read &msg );

protected:
	int			m_iHealthToAdd;

private:

	float		m_flStartTime;		// When the message was recevied
	float		m_flDuration;		// Duration of the message

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "text_font", "HudAddHealth" );
};

DECLARE_HUDELEMENT( CHudPlayerAddHealth );
DECLARE_HUD_MESSAGE( CHudPlayerAddHealth, PlayerAddHealth );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerAddHealth, PlayerAddHealth );
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::VidInit( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::Reset( void )
{
	SetPaintBackgroundEnabled( false );

	m_flStartTime = 0.0f;
	m_flDuration = 0.0f;
	m_iHealthToAdd = 0;
}

void CHudPlayerAddHealth::MsgFunc_PlayerAddHealth( bf_read &msg )
{
	// Read int and convert to string
	const int ptVal = msg.ReadShort();
	if(ptVal==0)
		return;

	// play animation (new points value)
	if(ptVal > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewAddHealth" );
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewSubtractHealth" );
	}
	
	m_iHealthToAdd = ptVal;

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = 3.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerAddHealth::Paint() 
{
	if(!hud_addhealth.GetBool())
		return;

	if ( m_flStartTime + m_flDuration < gpGlobals->curtime )
		return;

	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( GetFgColor() );
	surface()->DrawSetTextPos( 0, 0 );

	wchar_t wBuf[20];

	V_swprintf_safe( wBuf, L"%ls%d", m_iHealthToAdd > 0 ? L"+" : L"", m_iHealthToAdd );

	vgui::surface()->DrawPrintText( wBuf, V_wcslen( wBuf ), FONT_DRAW_NONADDITIVE );
}
