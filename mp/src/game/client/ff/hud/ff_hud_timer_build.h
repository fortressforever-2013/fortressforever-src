#ifndef FF_HUDBUILDSTATEDETPACK_H
#define FF_HUDBUILDSTATEDETPACK_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace vgui;

class CHudTimerBuild
	: public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudTimerBuild, FFQuantityPanel);
	CHudTimerBuild(const char* pElementName);
	~CHudTimerBuild();

	FFQuantityItem* m_qiTimer;

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