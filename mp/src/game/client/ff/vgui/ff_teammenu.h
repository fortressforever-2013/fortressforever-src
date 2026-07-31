/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file teammenu2.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New team selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation
/// NOTE from BreakinBenny: Long before this became ff_teammenu.h

#ifndef FF_TEAMMENU_H
#define FF_TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/HTML.h>

#include <igameevents.h>

#include "ff_button.h"
#include <teammenu.h>

//=============================================================================
// A team button has the following components:
//		A number (this is the button text itself so that hotkeys work automatically)
//		The team insignia image
//		Score
//		Player count
//		Avg ping
//=============================================================================
class TeamButton : public FFButton
{
private:
	DECLARE_CLASS_SIMPLE(TeamButton, FFButton);

public:
	TeamButton( vgui::Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd );
	//, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)

	void ApplySchemeSettings( vgui::IScheme *pScheme );
	void SetTeamID( int iTeamID );
	void UpdateTeamIcon( int iTeamID );
	void OnThink();

private:
	ImagePanel	*m_pTeamInsignia;
	Label		*m_pInfoDescriptions;
	Label		*m_pInfoValues;

	int			m_iTeamID;
};

//-----------------------------------------------------------------------------
// Purpose: Displays the team menu
//-----------------------------------------------------------------------------
class CFFTeamMenu : public CTeamMenu, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CFFTeamMenu, CTeamMenu );

public:
	CFFTeamMenu(IViewPort *pViewPort);
	~CFFTeamMenu();

	void Reset();
	void Update();
	void ShowPanel( bool bShow );

	virtual void FireGameEvent(IGameEvent *event);
	
protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);

	virtual void OnKeyCodeReleased(vgui::KeyCode code);

	void UpdateMapDescriptionText();
	void UpdateServerInfo();
	void UpdateTeamButtons();
	void UpdateTeamIcons();

	// vgui overrides
	virtual void OnCommand(const char *command);

	// ServerInfo elements
	FFButton		*m_pServerInfoButton;
	HTML			*m_pServerInfoHost;

	// MapDescription elements
	Label			*m_pMapDescriptionHead;
	RichText		*m_pMapDescriptionText;

	// ClassSelection elements
	TeamButton		*m_pTeamButtons[4];
	FFButton		*m_pSpectateButton;
	FFButton		*m_pAutoAssignButton;
		
	// Other
	FFButton		*m_pFlythroughButton;

	FFButton		*m_pMapScreenshotButton;	// Click to display the map screenshot
	
	char			m_szServerName[255];
};

#endif // FF_TEAMMENU_H