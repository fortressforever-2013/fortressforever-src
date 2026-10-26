//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FFTEXTWINDOW_H
#define FFTEXTWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <igameevents.h>
#include "vguitextwindow.h"
#include <game/client/iviewport.h>
#include <vgui/KeyCode.h>
#include "shareddefs.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: displays the MOTD
//-----------------------------------------------------------------------------

class CFFTextWindow : public CTextWindow, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CFFTextWindow, CTextWindow );

public:
	CFFTextWindow( IViewPort *pViewPort );
	virtual ~CFFTextWindow();

	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );
	
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeReleased(vgui::KeyCode code);
	
	virtual void FireGameEvent(IGameEvent *event);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

public:

	virtual void SetData( int type, const char *title, const char *message, const char *message_fallback, int command, bool bUnload );
	virtual void ShowFile( const char *filename );
	virtual void ShowText( const char *text );
	virtual void ShowURL( const char *URL, bool bAllowUserToDisable = true );
	virtual void ShowIndex( const char *entry );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

protected:	
	// vgui overrides
	virtual void OnCommand( const char *command );
};


#endif // VGUITEXTWINDOW_H