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

#ifndef TEAMMENU_H
#define TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <igameevents.h>

#include <game/client/iviewport.h>
#include <vgui/KeyCode.h>

class TeamButton;

namespace vgui
{
	class RichText;
	class HTML;
	class FFButton;

	//-----------------------------------------------------------------------------
	// Purpose: Displays the team menu
	//-----------------------------------------------------------------------------
	class CTeamMenu : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
	{
	private:
		DECLARE_CLASS_SIMPLE( CTeamMenu, vgui::Frame );

	public:
		CTeamMenu(IViewPort *pViewPort);
		virtual ~CTeamMenu();

		virtual const char *GetName( void ) { return PANEL_TEAM; }
		virtual void SetData(KeyValues* data) {};
		virtual void Reset();
		virtual void Update();
		virtual bool NeedsUpdate( void ) { return false; }
		virtual bool HasInputElements( void ) { return true; }
		virtual void ShowPanel( bool bShow );

		virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

		virtual void FireGameEvent(IGameEvent* event);

		virtual void OnKeyCodePressed(vgui::KeyCode code);
		virtual void OnKeyCodeReleased(vgui::KeyCode code);

		// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
		vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  		virtual bool IsVisible() { return BaseClass::IsVisible(); }
		virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
	
	protected:

		void UpdateMapDescriptionText();
		void UpdateServerInfo();
		void UpdateTeamButtons();
		void UpdateTeamIcons();

	protected:	
			// vgui overrides
			virtual void OnCommand(const char *command);

			IViewPort	*m_pViewPort;

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

			FFButton		*m_pMapScreenshotButton;			// Click to display the map screenshot
		
			char			m_szServerName[255];
	};
}

#endif // TEAMMENU_H
