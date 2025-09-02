#ifndef FF_HUDBUILDABLEMANCANNON_H
#define FF_HUDBUILDABLEMANCANNON_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace vgui;
using namespace FFQuantityHelper;

class CHudBuildableManCannon
    : public CHudElement, public FFQuantityPanel
{
private:
    DECLARE_CLASS_SIMPLE(CHudBuildableManCannon, FFQuantityPanel);
    CHudBuildableManCannon(const char* pElementName);
    ~CHudBuildableManCannon();

    FFQuantityItem* m_qiHealth;
    FFQuantityItem* m_qiBuildProgress;

    float m_flBuildStartTime = 0.0f;
    float m_flBuildDuration = 0.0f;

    bool m_bDeployed = false;
    bool m_bDeploying = false;

    std::wstring m_wsDeployed;
    std::wstring m_wsDeploying;
    std::wstring m_wsNotAvailable;
    std::wstring m_wsNotDeployed;

    // HUD customization options
    BuildableDisplayOption m_showPanel;
    BuildableDisplayOption m_hideText;
    bool m_bDisableBuildTimer;

	static KeyValues* s_kvVisibilityDisplayOptions;

    virtual void ApplySchemeSettings(IScheme* pScheme) override;

	virtual void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	virtual KeyValues* GetDefaultStyleData() override;
	virtual void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);
    void ApplyDisplayOptions();

    static std::optional<BuildableDisplayOption> GetBuildableDisplayOption(
        const char* keyName,
        KeyValues* kvStyleData,
        KeyValues* kvDefaultStyleData);

    void Init();
    void VidInit();

    void OnTick() override;
    void Paint() override;

    // Message Handlers
    void MsgFunc_ManCannonMsg(bf_read& msg);
    void MsgFunc_FF_BuildTimer(bf_read& msg);
};

#endif