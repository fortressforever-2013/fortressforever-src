#ifndef FF_HUD_BUILDABLE_MANCANNON_H
#define FF_HUD_BUILDABLE_MANCANNON_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudBuildableManCannon
    : public CHudElement, public vgui::FFQuantityPanel
{
private:
    DECLARE_CLASS_SIMPLE(CHudBuildableManCannon, vgui::FFQuantityPanel);
    CHudBuildableManCannon(const char* pElementName);
    ~CHudBuildableManCannon();

    vgui::FFQuantityItem* m_qiHealth;
    vgui::FFQuantityItem* m_qiBuildProgress;

    float m_flBuildStartTime = 0.0f;
    float m_flBuildDuration = 0.0f;

    bool m_bCanHaveManCannon = false;
    bool m_bHasManCannon = false;

    bool m_bDeployed = false;
    bool m_bDeploying = false;

    std::wstring m_wsDeployed;
    std::wstring m_wsDeploying;
    std::wstring m_wsNotAvailable;
    std::wstring m_wsNotDeployed;

    // HUD customization options
    BuildableVisibility m_showPanel;
    bool m_bShowText;
    bool m_bDisableBuildTimer;
    bool m_bInvertBuildTimer = false;

	static KeyValues* s_kvVisibilityOptions;

    virtual void ApplySchemeSettings(vgui::IScheme* pScheme) override;

	virtual void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	virtual KeyValues* GetDefaultStyleData() override;
	virtual void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);
    void ApplyVisibilityOptions();

    static std::optional<BuildableVisibility> GetBuildableVisibility(
        const char* keyName,
        KeyValues* kvStyleData,
        KeyValues* kvDefaultStyleData);

    void Init();
    void VidInit();

    void Paint() override;

    void SetAvailability(bool bCanHave, bool bHas);
    void SetBuildState(BuildState_t state, int iHealth, int iMaxHealth);
    void SetBuildTimer(float flDuration);

    // Message Handlers
    void MsgFunc_ManCannonMsg(bf_read& msg);
    void MsgFunc_ManCannonAvailabilityMsg(bf_read& msg);
    void UpdateAvailabilityText();
    void MsgFunc_FF_BuildTimer(bf_read& msg);
};

#endif