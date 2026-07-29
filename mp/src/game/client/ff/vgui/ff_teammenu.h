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

#include <teammenu.h>

//-----------------------------------------------------------------------------
// Purpose: Displays the team menu
//-----------------------------------------------------------------------------
class CFFTeamMenu : public CTeamMenu
{
private:
	DECLARE_CLASS_SIMPLE( CFFTeamMenu, CTeamMenu );

public:
	CFFTeamMenu(IViewPort *pViewPort);
	~CFFTeamMenu();

	void Reset();
	void Update();
	void ShowPanel( bool bShow );

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual void FireGameEvent( IGameEvent *event );

	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeReleased(vgui::KeyCode code);
	
protected:
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