#ifndef FF_HUDBUILDSTATEDETPACK_H
#define FF_HUDBUILDSTATEDETPACK_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace vgui;

class CHudTimerDetpack
	: public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudTimerDetpack, FFQuantityPanel);
	CHudTimerDetpack(const char* pElementName);
	~CHudTimerDetpack();

	FFQuantityItem* m_qiDetpackTimeLeft;
	FFQuantityItem* m_qiBuildProgress;

	float m_flBuildStartTime = 0.0f;
	float m_flBuildDuration = 0.0f;

	float m_flDetonateTime = 0.0f;

	bool m_bArmed = false;
	bool m_bArming = false;

	std::wstring m_wsArmed;
	std::wstring m_wsArming;
	std::wstring m_wsAvailable;
	std::wstring m_wsNotAvailable;

	bool m_bShowPanel;
	bool m_bHideText;
	bool m_bDisableBuildTimer;

	virtual void ApplySchemeSettings(
		IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;
	void ApplyDisplayOptions();

	void Init();
	void VidInit();

	void OnTick() override;
	void Paint() override;

	void MsgFunc_DetpackMsg(bf_read& msg);
	void MsgFunc_FF_BuildTimer(bf_read& msg);
};

#endif