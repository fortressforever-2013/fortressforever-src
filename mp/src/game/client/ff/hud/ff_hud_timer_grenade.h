#ifndef FF_HUD_TIMER_GRENADE_H
#define FF_HUD_TIMER_GRENADE_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vector>

using namespace FFQuantityHelper;

class CHudGrenadeTimer : public CHudElement, public vgui::FFQuantityPanel
{
public:
	DECLARE_CLASS_SIMPLE(CHudGrenadeTimer, vgui::FFQuantityPanel);
	CHudGrenadeTimer(
		const char* pElementName,
		const char* szPanelName,
		const char* szTitle,
		int iPrimeState);
	~CHudGrenadeTimer();

	void Init();
	void VidInit();
	void Reset();
	void ResetTimer();
	void SetTimer(float flDuration);
	bool ActiveTimer() const;
	int ActiveTimerCount() const;
	bool ShouldDraw() override;
	void OnThink() override;

protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	void AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(KeyValues* kvStyleData, KeyValues* kvDefaultStyleData) override;

private:
	struct Timer
	{
		float m_flStart;
		float m_flDuration;
	};

	std::vector<Timer> m_timers;
	std::vector<vgui::FFQuantityItem*> m_items;
	KeyValues* m_kvItemStyle;
	const char* m_szTitle;
	int m_iPrimeState;
	int m_iHeaderClass = -1;
	bool m_bShowPanel = true;
	bool m_bCountDown = false;
	size_t m_iMaxVisibleTimers = 3;
	bool m_bReverseOrder = false;
	int m_iThrownOpacity = 50;
	vgui::FFQuantityItem::OpacityTarget m_thrownOpacityTarget = vgui::FFQuantityItem::OPACITY_ALL;

	vgui::FFQuantityItem* CreateTimerItem();
	void ApplyTimerMode(vgui::FFQuantityItem* pItem);
	void UpdatePreview();
	void UpdateGrenadeHeader();
	void ApplyVisibilityOptions();
};

#endif
