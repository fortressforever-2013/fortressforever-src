#ifndef FF_HUD_RESOURCE_WEAPONCHARGE_H
#define FF_HUD_RESOURCE_WEAPONCHARGE_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "iclientmode.h"

class CHudResourceWeaponCharge : public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourceWeaponCharge, vgui::FFQuantityPanel);

public:
	CHudResourceWeaponCharge(
		const char* pElementName,
		int iWeaponID,
		const char* pszTitle,
		bool bShowPanel);
	void Init();
	void VidInit();
	bool ShouldDraw() override;

protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	void AddPanelSpecificOptions(KeyValues* kvOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(KeyValues* kvStyleData, KeyValues* kvDefaults) override;

private:
	int m_iWeaponID;
	const char* m_pszTitle;
	bool m_bDefaultShowPanel;
	enum PanelVisibility {
		NEVER = 0,
		WHEN_ACTIVE = 1,
		WHILE_EQUIPPED = 2
	};

	PanelVisibility m_showPanel;
	vgui::FFQuantityItem* m_pCharge = nullptr;
	vgui::FFQuantityItem* m_pAnchor = nullptr;
	bool m_bShowAnchor = true;
	bool m_bShowText = true;

	std::optional<PanelVisibility> GetPanelVisibility(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	void UpdateCharge();
	bool IsEquipped();
	float GetCharge();
};

class CHudResourceSniperCharge : public CHudResourceWeaponCharge
{
public:
	CHudResourceSniperCharge(const char* pElementName);
};

class CHudResourceAssaultCannonCharge : public CHudResourceWeaponCharge
{
public:
	CHudResourceAssaultCannonCharge(const char* pElementName);
};

class CHudResourceJumpgunCharge : public CHudResourceWeaponCharge
{
public:
	CHudResourceJumpgunCharge(const char* pElementName);
};

#endif
