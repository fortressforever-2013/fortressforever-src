#ifndef FF_HUD_BUILDTIMER_H
#define FF_HUD_BUILDTIMER_H

#include "cbase.h"
#include "ff_panel.h"

using namespace vgui;

class CHudBuildTimer : public CHudElement, public FFPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildTimer, FFPanel);

	int		m_iBuildType;
	int		m_iPlayerTeam;
	bool	m_fVisible;
	float	m_flStartTime;
	float	m_flDuration;

	CHudTexture *m_pIconTexture; // the one being used

	CHudTexture *m_pDispenserIconTexture;
	CHudTexture *m_pSentrygunIconTexture;
	CHudTexture *m_pDetpackIconTexture;
	CHudTexture *m_pMancannonIconTexture;
	
	CPanelAnimationVarAliasType(float, bar_xpos, "bar_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_ypos, "bar_ypos", "0", "proportional_float");

	CPanelAnimationVarAliasType(float, icon_xpos, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_ypos, "icon_ypos", "0", "proportional_float");

	CPanelAnimationVarAliasType(float, icon_width, "icon_width", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_height, "icon_height", "0", "proportional_float");

public:
	CHudBuildTimer(const char *pElementName);
	~CHudBuildTimer();

	virtual void	Init();
	virtual void	VidInit();
	virtual void	Paint();
	virtual void	PaintBackground();
	virtual void	OnTick();
	virtual void	Reset();

	void	SetBuildTimer(int iBuildType, float flDuration);

	// Callback functions for setting
	void	MsgFunc_FF_BuildTimer(bf_read &msg);
};

#endif