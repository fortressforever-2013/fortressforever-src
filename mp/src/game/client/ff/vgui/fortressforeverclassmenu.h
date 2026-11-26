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

#include "classmenu.h"

class FFClassMenuButton;
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
class CFFClassMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE( CFFClassMenu, CClassMenu );

public:
	CFFClassMenu(IViewPort *pViewPort);

	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return gpGlobals->curtime > m_flNextUpdate; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeReleased(vgui::KeyCode code);

	MESSAGE_FUNC_PARAMS(OnMouseOverMessage, "MouseOverEvent", data);

protected:

	void UpdateClassInfo(const char* pszClassName);
	void SetClassInfoVisible(bool state);

	// command callbacks
	void OnCommand( const char *command );

	vgui::RichText* m_pClassInfo;

	float			m_flNextUpdate;

	FFClassMenuButton* m_pClassButtons[10];
	vgui::FFButton* m_pCancelButton;
	FFClassMenuButton* m_pRandomButton;

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

};


#endif // FF_CLASSMENU_H
