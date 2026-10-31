/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file classmenu.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New class selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#ifndef FF_CLASSMENU_H
#define FF_CLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <classmenu.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <utlvector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include <game/client/iviewport.h>

#include "mouseoverpanelbutton.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Draws the class menu
//-----------------------------------------------------------------------------
class CFFClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CFFClassMenu, CClassMenu );

public:
	CFFClassMenu( IViewPort *pViewPort );

	void Update();
	void ShowPanel( bool bShow );

	void OnKeyCodePressed( vgui::KeyCode code );
	void OnKeyCodeReleased( vgui::KeyCode code );

protected:

	void UpdateClassInfo(const char* pszClassName);
	void SetClassInfoVisible(bool state);

	// command callbacks
	void OnCommand( const char *command );

	IViewPort	*m_pViewPort;
	ButtonCode_t m_iScoreBoardKey;
	int			m_iTeam;
	vgui::EditablePanel *m_pPanel;

	vgui::RichText* m_pClassInfo;

	float			m_flNextUpdate;
/*
	MouseOverButton* m_pClassButtons[10];
	vgui::FFButton* m_pCancelButton;
	MouseOverButton* m_pRandomButton;

	LoadoutLabel* m_pPrimaryGren;
	LoadoutLabel* m_pSecondaryGren;

	LoadoutLabel* m_WepSlots[8];

	ClassPropertiesLabel* m_pSpeed;
	ClassPropertiesLabel* m_pFirepower;
	ClassPropertiesLabel* m_pHealth;

	vgui::PlayerModelPanel* m_pModelView;

	vgui::Label* m_pClassRole;

	vgui::Section* m_pGrenadesSection;
	vgui::Section* m_pWeaponsSection;
	vgui::Section* m_pClassInfoSection;
	vgui::Section* m_pClassRoleSection;
*/
	//virtual vgui::Panel* CreateControlByName(const char* controlName);
	//virtual MouseOverPanelButton* CreateNewMouseOverPanelButton(vgui::EditablePanel* panel);
};


#endif // FF_CLASSMENU_H
