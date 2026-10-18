#ifndef FF_HUD_RESOURCE_JETPACKFUEL_H
#define FF_HUD_RESOURCE_JETPACKFUEL_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudResourceJetpackFuel
	: public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourceJetpackFuel, vgui::FFQuantityPanel);
	CHudResourceJetpackFuel(const char* pElementName);
	~CHudResourceJetpackFuel();

	vgui::FFQuantityItem* m_qiJetpackFuel;
	vgui::FFQuantityItem* m_qiRecovery = nullptr;

	std::wstring m_wsFull;
	std::wstring m_wsRegenerating;
	std::wstring m_wsInUse = L"In Use";
	std::wstring m_wsRecovering = L"Recovering...";

	bool m_bShowPanel = false;
	bool m_bHideWhenFull = false;
	bool m_bShowRecovery = true;

	virtual void ApplySchemeSettings(
		vgui::IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	void Init();
	void VidInit();

	bool ShouldDraw() override;
	void Paint() override;
};

#endif