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

#ifndef CLASSMENU_H
#define CLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <utlvector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include <game/client/iviewport.h>

#include "mouseoverpanelbutton.h"

class MouseOverButton;
class LoadoutLabel;
class ClassPropertiesLabel;

namespace vgui
{
	class TextEntry;
	class PlayerModelPanel;
	class FFButton;
	class ProgressBar;
	class Section;
	class RichText;
}

//-----------------------------------------------------------------------------
// Purpose: Draws the class menu
//-----------------------------------------------------------------------------
class CClassMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CClassMenu, vgui::Frame );

public:
	CClassMenu(IViewPort *pViewPort);
	virtual ~CClassMenu();

	virtual const char *GetName( void ) { return PANEL_CLASS; }
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return gpGlobals->curtime > m_flNextUpdate; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
	virtual bool IsVisible() { return BaseClass::IsVisible(); }
	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeReleased(vgui::KeyCode code);

	MESSAGE_FUNC_PARAMS(OnMouseOverMessage, "MouseOverEvent", data);

protected:

	void UpdateClassInfo(const char* pszClassName);
	void SetClassInfoVisible(bool state);

	// helper functions
	void SetLabelText(const char *textEntryName, const char *text);
	void SetVisibleButton(const char *textEntryName, bool state);

	// command callbacks
	void OnCommand( const char *command );

	IViewPort	*m_pViewPort;
	ButtonCode_t m_iScoreBoardKey;
	int			m_iTeam;
	vgui::EditablePanel *m_pPanel;

	vgui::RichText* m_pClassInfo;

	float			m_flNextUpdate;

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

	//virtual vgui::Panel* CreateControlByName(const char* controlName);
	//virtual MouseOverPanelButton* CreateNewMouseOverPanelButton(vgui::EditablePanel* panel);
};


#endif // CLASSMENU_H
