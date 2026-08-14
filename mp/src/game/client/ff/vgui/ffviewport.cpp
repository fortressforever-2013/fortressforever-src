//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client DLL VGUI2 Viewport
//
// $Workfile:     $
// $Date: 2005/12/29 18:31:52 $
//
//-----------------------------------------------------------------------------
// $Log: ffviewport.cpp,v $
// Revision 1.4  2005/12/29 18:31:52  mirvin_monkey
// no message
//
// Revision 1.3  2005/06/18 10:07:12  mirvin_monkey
// SDK update
//
// Revision 1.2  2005/02/20 19:02:35  billdoor
// Implementing FF update.
//
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#pragma warning( disable : 4800  )  // disable forcing int to bool performance warning

// VGUI panel includes
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui/Cursor.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/VGUI.h>

// client dll/engine defines
#include "hud.h"
#include <voice_status.h>

// viewport definitions
#include <baseviewport.h>
#include "ffviewport.h"
//#include "ff_teammenu.h"
#include "teammenu.h"

#include "vguicenterprint.h"
#include "text_message.h"
//#include "ff_classmenu.h"
#include "classmenu.h"
#include "mapscreen.h"
#include "c_ff_player.h"


void FFViewport::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	gHUD.InitColors( pScheme );

	SetPaintBackgroundEnabled( false );
}


IViewPortPanel* FFViewport::CreatePanelByName(const char *szPanelName)
{
	IViewPortPanel* newpanel = NULL;

// Up here, strcmp against each type of panel we know how to create.
//	else if ( Q_strcmp(PANEL_OVERVIEW, szPanelName) == 0 )
//	{
//		newpanel = new CCSMapOverview( this );
//	}

	// --> Mirv: Pick up new panels
	if (Q_strcmp(PANEL_TEAM, szPanelName) == 0)
	{
		newpanel = new CTeamMenu(this);
	}
	else if (Q_strcmp(PANEL_CLASS, szPanelName) == 0)
	{
		newpanel = new CClassMenu(this);
	}
	else if (Q_strcmp(PANEL_MAP, szPanelName) == 0)
	{
		newpanel = new CMapScreen(this);
	}
	// <--

	else
	{
		// create a generic base panel, don't add twice
		newpanel = BaseClass::CreatePanelByName( szPanelName );
	}

	return newpanel; 
}

void FFViewport::CreateDefaultPanels( void )
{
	// PANEL_OVERVIEW isn't used in baseviewport.cpp CreatePanelByName
	// This is what was giving us the 
	// "CBaseViewport::AddNewPanel: NULL panel."
	// message in the console
	//AddNewPanel( CreatePanelByName( PANEL_OVERVIEW ) );	// |-- Mirv: Overview!
	// --> Mirv: FF panels
	AddNewPanel( CreatePanelByName( PANEL_TEAM ), "PANEL_TEAM" );
	AddNewPanel( CreatePanelByName( PANEL_CLASS ), "PANEL_CLASS" );
	AddNewPanel( CreatePanelByName( PANEL_MAP ), "PANEL_MAP" );

	BaseClass::CreateDefaultPanels();
}

int FFViewport::GetDeathMessageStartHeight( void )
{
	int x = YRES(2);

	/*IViewPortPanel *spectator = gViewPortInterface->FindPanelByName( PANEL_SPECGUI );

	//TODO: Link to actual height of spectator bar
	if ( spectator && spectator->IsVisible() )
	{
		x += YRES(52);
	}*/

	return x;
}

void FFViewport::PostMessageToPanel(IViewPortPanel* pPanel, KeyValues* pKeyValues)
{
	PostMessage(pPanel->GetVPanel(), pKeyValues);
}

void FFViewport::PostMessageToPanel(const char* pName, KeyValues* pKeyValues)
{
	if (Q_strcmp(pName, PANEL_ALL) == 0)
	{
		for (int i = 0; i < m_Panels.Count(); i++)
		{
			PostMessageToPanel(m_Panels[i], pKeyValues);
		}

		return;
	}

	IViewPortPanel* panel = NULL;

	if (Q_strcmp(pName, PANEL_ACTIVE) == 0)
	{
		panel = m_pActivePanel;
	}
	else
	{
		panel = FindPanelByName(pName);
	}

	if (!panel)
		return;

	PostMessageToPanel(panel, pKeyValues);
}

// BEG: Added by Mulchman for team change & class change
CON_COMMAND( changeteam, "Choose a new team" )
{
	if ( !gViewPortInterface )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( pPlayer )
		gViewPortInterface->ShowPanel( PANEL_TEAM, true );
}

CON_COMMAND( changeclass, "Choose a new class" )
{
	if ( !gViewPortInterface )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	// --> Mirv: Select team first bud
	if (pPlayer && pPlayer->GetTeamNumber() < FIRST_GAME_TEAM)
	{
		engine->ClientCmd("changeteam");
		return;
	}
	// <-- Mirv: Select team first bud

	gViewPortInterface->ShowPanel( PANEL_CLASS, true );
}
// END: Added by Mulchman for team change & class change

