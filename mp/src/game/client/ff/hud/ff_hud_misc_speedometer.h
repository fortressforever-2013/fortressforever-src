#ifndef FF_HUD_MISC_SPEEDOMETER_H
#define FF_HUD_MISC_SPEEDOMETER_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudMiscSpeedometer
    : public CHudElement, public vgui::FFQuantityPanel
{
private:
    DECLARE_CLASS_SIMPLE(CHudMiscSpeedometer, vgui::FFQuantityPanel);
    CHudMiscSpeedometer(const char* pElementName);
    ~CHudMiscSpeedometer();

    vgui::FFQuantityItem* m_qiInstantaneousSpeed;
    vgui::FFQuantityItem* m_qiAverageSpeed;

    float m_flNextUpdate = 0.0f;
    float m_flAverageSpeed = 0.0f;
    float m_flClassSpeed = 0.0f;
    float m_flAveragePeriod = 1.0f;
    float m_flAverageResponsiveness = 0.8f;

    // HUD customization options
    bool m_bShowPanel;
    enum SpeedItems
    {
        SPEED_ONLY = 0,
        AVERAGE_ONLY,
        BOTH
    };
    SpeedItems m_speedItems;

    virtual void ApplySchemeSettings(vgui::IScheme* pScheme) override;

	virtual void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	virtual KeyValues* GetDefaultStyleData() override;
	virtual void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);
    void ApplyVisibilityOptions();

    void Init();
    void VidInit();

    void OnThink() override;

    float AlphaForTarget(
        float flResponsiveness,
        float flPeriod,
        float flUpdateInterval);
};

#endif