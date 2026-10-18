#ifndef FF_HUD_CROSSHAIRINFO_H
#define FF_HUD_CROSSHAIRINFO_H

#include "ff_quantitypanel.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudCrosshairInfoPanel : public CHudElement, public vgui::FFQuantityPanel
{
public:
	DECLARE_CLASS_SIMPLE(CHudCrosshairInfoPanel, vgui::FFQuantityPanel);
	CHudCrosshairInfoPanel(const char* pElementName, int iTargetType);
	void Init();
	void VidInit();
	void Reset();
	bool ShouldDraw() override;

protected:
	int GetColorTeamNumber() override;
	void AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(KeyValues* kvStyleData, KeyValues* kvDefaultStyleData) override;

private:
	friend class CHudCrosshairInfo;
	int m_iTargetType;
	int m_iColorTeam = TEAM_UNASSIGNED;
	enum Quantity
	{
		HEALTH, AMMO, ARMOR, CELLS, ROCKETS, NAILS, SHELLS, LEVEL, QUANTITY_COUNT
	};
	vgui::FFQuantityItem* m_items[QUANTITY_COUNT];
	int m_values[QUANTITY_COUNT];
	int m_maximums[QUANTITY_COUNT];
	bool m_bShowPanel = true;
	bool m_bHideAmmo = true;
	bool m_bHideArmor = false;
	bool m_bHideCells = false;
	bool m_bHideNails = false;
	bool m_bHideShells = false;
	bool m_bHideRockets = false;
	bool m_bHideLevel = false;
	bool m_bShowOwnBuildable = false;
	bool m_bOwnBuildable = false;
	AmountFormat m_quantityFormats[QUANTITY_COUNT];
	float m_flDrawTime = -2.0f;
	float m_flDrawDuration = 1.0f;

	void SetQuantity(Quantity quantity, int iValue, int iMaximum);
	void UpdateQuantityVisibility();
	void ApplyVisibilityOptions();
	void ShowPreview();
};

class CHudCrosshairInfoPlayer : public CHudCrosshairInfoPanel
{
public:
	CHudCrosshairInfoPlayer(const char* pElementName);
};

class CHudCrosshairInfoSentry : public CHudCrosshairInfoPanel
{
public:
	CHudCrosshairInfoSentry(const char* pElementName);
};

class CHudCrosshairInfoDispenser : public CHudCrosshairInfoPanel
{
public:
	CHudCrosshairInfoDispenser(const char* pElementName);
};

class CHudCrosshairInfoManCannon : public CHudCrosshairInfoPanel
{
public:
	CHudCrosshairInfoManCannon(const char* pElementName);
};

// Resolve the target once, then update the matching custom HUD panel.
class CHudCrosshairInfo : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE(CHudCrosshairInfo, vgui::Panel);
	CHudCrosshairInfo(const char* pElementName);
	void Init();
	void VidInit();
	void Reset();
	void OnTick() override;
	bool ShouldDraw() override { return false; }

private:
	float m_flStartTime = 0.0f;
	float m_flDuration = 0.2f;
	int m_iTeam = 0;
	int m_iClass = 0;
};

#endif
