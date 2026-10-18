#ifndef FF_HUD_BUILDABLE_H
#define FF_HUD_BUILDABLE_H

#include "ff_quantitypanel.h"
#include "ff_buildabledefs.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

#include "usermessages.h"
#include <vector>

using namespace FFQuantityHelper;

class CHudBuildable
	: public CHudElement, public vgui::FFQuantityPanel
{
public:
	CHudBuildable(
		const char* szElementName,
		const int iBuildType,
		const char* szPanelName,
		const char* szDisplayNameKey);
	~CHudBuildable();

private:
	DECLARE_CLASS_SIMPLE(CHudBuildable, vgui::FFQuantityPanel);

	const char* m_szDisplayNameKey;

	int m_iBuildType;

	vgui::FFQuantityItem* m_qiHealth;
	vgui::FFQuantityItem* m_qiBuildProgress;
	vgui::FFQuantityItem* m_qiCells;

	int m_iHealth = -1;

	float m_flBuildStartTime;
	float m_flBuildDuration;

	bool m_bBuilt;
	bool m_bBuilding;

	int m_iCellRequirement;

	std::wstring m_wsBuilding;
	std::wstring m_wsBuilt;
	std::wstring m_wsCellsNeeded;
	std::wstring m_wsNotBuilt;
	std::wstring m_wsReadyToUpgrade;

    // HUD customization options
	enum BuildableVisibility
	{
		NEVER = 0,
		ALWAYS,
		ON_BUILD,
		IF_BUILT
	};

	BuildableVisibility m_showPanel;
	bool m_bShowText;
	BuildableVisibility m_hideCells;
	bool m_bDisableBuildTimer;
	bool m_bInvertBuildTimer = false;

	static KeyValues* s_kvVisibilityOptions;

	static std::vector<CHudBuildable*> s_Buildables;

	virtual void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	virtual KeyValues* GetDefaultStyleData() override;
	virtual void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	static std::optional<BuildableVisibility> GetBuildableVisibility(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	virtual void Init();
	virtual void VidInit();

	virtual void OnTick() override;
	virtual void Paint() override;

	void MsgFunc_FF_BuildTimer(bf_read& msg);

protected:
    virtual void ApplySchemeSettings(vgui::IScheme* pScheme) override;
	virtual void ApplyVisibilityOptions();

	void SetHealth(int iHealth);

	void InitItemHealth();
	void InitItemBuildProgress();
	void InitItemCells();

	virtual int GetMaxCells() = 0;
	virtual CFFBuildableObject* GetBuildable() = 0;
	virtual void OnBuiltChanged(bool bBuilt);
};

#endif
