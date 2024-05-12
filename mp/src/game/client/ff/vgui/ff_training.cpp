/********************************************************************
	created:	2006/09/28
	created:	28:9:2006   13:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gamemodes.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gamemodes
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "KeyValues.h"
#include "ff_training.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

DEFINE_GAMEUI(CFFTrainingUI, CFFTrainingPanel, fftraining);

CON_COMMAND( ff_training, NULL )
{
	//ToggleVisibility(ffirc->GetPanel());
	fftraining->GetPanel()->SetVisible(true);
}


// The main screen where everything IRC related is added to
CFFTrainingPanel::CFFTrainingPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFTrainingPanel" )
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	// Centre this panel on the screen for consistency.
	// This should be in the .res surely? - AfterShock

	m_pOKButton = new Button(this, "OKButton", "", this, "OK");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");

	LoadControlSettings("Resource/UI/FFTraining.res");
	//CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
	
	SetPos((ScreenWidth() - GetWide()) / 2, (ScreenHeight() - GetTall()) / 2);

	//Other useful options
	SetSizeable(false);
	SetMoveable(true);
} 

//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFTrainingPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "Cancel") == 0)
	{
		Close();
		return;
	}

	engine->ClientCmd("sv_lan 1\n");
	engine->ClientCmd("mp_timelimit 0\n");
	engine->ClientCmd("sv_cheats 0\n");
	engine->ClientCmd("progress_enable 1\n");
	engine->ClientCmd("map ff_training\n");

	Close();
}

//-----------------------------------------------------------------------------
// Purpose: Catch this menu coming on screen and load up the info needed
//			
//-----------------------------------------------------------------------------
void CFFTrainingPanel::SetVisible(bool state)
{
	if (state)
	{
		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}