#ifndef FF_HUDBUILDABLE_H
#define FF_HUDBUILDABLE_H

#include "ff_quantitypanel.h"
#include "ff_buildabledefs.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

#include "usermessages.h"
#include <vector>

using namespace vgui;
using namespace FFQuantityHelper;

class CHudBuildable
	: public CHudElement, public FFQuantityPanel
{
public:
	CHudBuildable(
		const char* szElementName,
		const int iBuildType,
		const char* szPanelName,
		const char* szDisplayNameKey);
	~CHudBuildable();

private:
	DECLARE_CLASS_SIMPLE(CHudBuildable, FFQuantityPanel);

	const char* m_szDisplayNameKey;

	int m_iBuildType;

	FFQuantityItem* m_qiHealth;
	FFQuantityItem* m_qiBuildProgress;
	FFQuantityItem* m_qiCells;

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
	enum BuildableDisplayOption
	{
		NEVER = 0,
		ALWAYS,
		ON_BUILD,
		IF_BUILT
	};

	BuildableDisplayOption m_showPanel;
	BuildableDisplayOption m_hideText;
	BuildableDisplayOption m_hideCells;
	bool m_bDisableBuildTimer;

	static KeyValues* s_kvVisibilityDisplayOptions;

	static std::vector<CHudBuildable*> s_Buildables;

	virtual void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	virtual KeyValues* GetDefaultStyleData() override;
	virtual void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	static std::optional<BuildableDisplayOption> GetBuildableDisplayOption(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	virtual void Init();
	virtual void VidInit();

	virtual void OnTick() override;
	virtual void Paint() override;

	void MsgFunc_FF_BuildTimer(bf_read& msg);

protected:
    virtual void ApplySchemeSettings(IScheme* pScheme) override;
	virtual void ApplyDisplayOptions();

	void InitItemHealth();
	void InitItemBuildProgress();
	void InitItemCells();

	virtual int GetMaxCells() = 0;
	virtual CFFBuildableObject* GetBuildable() = 0;
	virtual void OnBuiltChanged(bool bBuilt);
};

#endif