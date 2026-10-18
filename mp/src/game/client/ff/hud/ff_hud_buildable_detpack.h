#ifndef FF_HUD_BUILDABLE_DETPACK_H
#define FF_HUD_BUILDABLE_DETPACK_H

#include "ff_quantitypanel.h"
#include "hudelement.h"

#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"


class CHudBuildableDetpack
	: public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildableDetpack, vgui::FFQuantityPanel);
	CHudBuildableDetpack(const char* pElementName);
	~CHudBuildableDetpack();

	vgui::FFQuantityItem* m_qiDetpackTimeLeft;
	vgui::FFQuantityItem* m_qiBuildProgress;

	float m_flBuildStartTime = 0.0f;
	float m_flBuildDuration = 0.0f;

	float m_flDetonateTime = 0.0f;

	bool m_bCanHaveDetpack = false;
	bool m_bHasDetpack = false;
	bool m_bArmed = false;
	bool m_bArming = false;

	std::wstring m_wsArmed;
	std::wstring m_wsArming;
	std::wstring m_wsAvailable;
	std::wstring m_wsNotAvailable;

	FFQuantityHelper::BuildableVisibility m_showPanel = FFQuantityHelper::ALWAYS;
	bool m_bShowText = true;
	bool m_bDisableBuildTimer = false;
	bool m_bInvertBuildTimer = false;

	static KeyValues* s_kvVisibilityOptions;

	virtual void ApplySchemeSettings(
		vgui::IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;
	void ApplyVisibilityOptions();

	static std::optional<FFQuantityHelper::BuildableVisibility> GetBuildableVisibility(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	void Init();
	void VidInit();

	void OnTick() override;
	void Paint() override;

	void MsgFunc_DetpackAvailabilityMsg(bf_read& msg);
	void UpdateAvailabilityText();

	void MsgFunc_DetpackMsg(bf_read& msg);
	void MsgFunc_FF_BuildTimer(bf_read& msg);
};

#endif