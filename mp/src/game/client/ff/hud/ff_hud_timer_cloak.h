#ifndef FF_HUD_TIMER_CLOAK_H
#define FF_HUD_TIMER_CLOAK_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudTimerCloak
	: public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudTimerCloak, vgui::FFQuantityPanel);
	CHudTimerCloak(const char* pElementName);
	~CHudTimerCloak();

	vgui::FFQuantityItem* m_qiTimer;
	CooldownVisibility m_show = CooldownVisibility::ALWAYS;
	CooldownVisibility m_showProgress = CooldownVisibility::ALWAYS;
	bool m_bCountDown = false;
	std::wstring m_wsReady = L"Ready";
	std::wstring m_wsRegenerating = L"Regenerating...";

	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	void AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;
	void ApplyVisibilityOptions();
	void UpdateProgressVisibility();
	void OnTick() override;

	void Init();
	void VidInit();
	bool ShouldDraw() override;
	void Paint() override;
};

#endif
