#ifndef FF_HUD_RESOURCE_RADAR_H
#define FF_HUD_RESOURCE_RADAR_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudResourceRadar : public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourceRadar, vgui::FFQuantityPanel);
	CHudResourceRadar(const char* pElementName);

	vgui::FFQuantityItem* m_qiScans = nullptr;
	std::wstring m_wsFull = L"Full";
	std::wstring m_wsRegenerating = L"Regenerating...";

	bool m_bShowPanel = true;
	bool m_bHideWhenFull = false;

	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	void AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	bool ShouldDraw() override;
	void Init() override;
	void VidInit() override;
	void OnTick() override;
};

#endif
