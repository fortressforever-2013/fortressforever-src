//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_TEAMMENU_H
#define FF_TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_button.h"
#include <vgui_controls/HTML.h>
#include <igameevents.h>
#include <teammenu.h>

class TeamButton;
/*
namespace vgui
{
	class RichText;
	class HTML;
	class FFButton;
}
*/

//-----------------------------------------------------------------------------
// Purpose: Displays the team menu
//-----------------------------------------------------------------------------
class CFFTeamMenu : public CTeamMenu, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CFFTeamMenu, CTeamMenu );

public:
	CFFTeamMenu( IViewPort *pViewPort );
	~CFFTeamMenu();

	void Update();
	void ShowPanel( bool bShow );
	
	// IGameEventListener interface:
	void FireGameEvent( IGameEvent *event );

private:
	
	void ApplySchemeSettings( vgui::IScheme *pScheme );
	void OnKeyCodePressed( vgui::KeyCode code );
	void OnKeyCodeReleased( vgui::KeyCode code );
	void OnCommand( const char *command );

	void UpdateMapDescriptionText();
	void UpdateServerInfo();
	void UpdateTeamButtons();
	void UpdateTeamIcons();

	// ServerInfo elements
	//FFButton	*m_pServerInfoButton;
	//vgui::HTML	*m_pServerInfoHost;

	// ClassSelection elements
	//TeamButton	*m_pTeamButtons[4];

	// Other
	//FFButton	*m_pFlythroughButton;
	//FFButton	*m_pMapScreenshotButton;	// Click to display the map screenshot

	char		m_szServerName[255];
};

#endif // FF_TEAMMENU_H
