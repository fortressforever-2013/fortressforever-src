#ifndef FF_HUD_TIMER_BUILD_H
#define FF_HUD_TIMER_BUILD_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"


class CHudTimerBuild
	: public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudTimerBuild, vgui::FFQuantityPanel);
	CHudTimerBuild(const char* pElementName);
	~CHudTimerBuild();

	vgui::FFQuantityItem* m_qiTimer;

	int m_iBuildType;
	float m_flStartTime;
	float m_flDuration;

	bool m_bShowPanel;
	bool m_bInvertScale;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	void Init();
	void VidInit();

	void Paint() override;

	void MsgFunc_FF_BuildTimer(bf_read& msg);
	void SetBuildTimer(int iBuildType, float flDuration = 0);
};

#endif