//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FFVIEWPORT_H
#define FFVIEWPORT_H


#include "ff_shareddefs.h"
#include "baseviewport.h"


//using namespace vgui;

namespace vgui 
{
	class Panel;
}

class FFViewport : public CBaseViewport
{

private:
	DECLARE_CLASS_SIMPLE( FFViewport, CBaseViewport );

public:

	IViewPortPanel* CreatePanelByName(const char *szPanelName);
	void CreateDefaultPanels( void );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		
	int GetDeathMessageStartHeight( void );

	virtual void PostMessageToPanel(IViewPortPanel* pPanel, KeyValues* pKeyValues);
	virtual void PostMessageToPanel(const char* pName, KeyValues* pKeyValues);
};


#endif // FFViewport_H
