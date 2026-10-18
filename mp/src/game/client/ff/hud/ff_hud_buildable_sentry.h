#ifndef FF_HUD_BUILDABLE_SENTRY_H
#define FF_HUD_BUILDABLE_SENTRY_H

#include "ff_hud_buildable.h"

#include "c_ff_player.h"


class CHudBuildableSentry
	: public CHudBuildable
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildableSentry, CHudBuildable);
	CHudBuildableSentry(const char* pElementName);
	~CHudBuildableSentry();

	vgui::FFQuantityItem* m_qiLevel;
	vgui::FFQuantityItem* m_qiShells;
	vgui::FFQuantityItem* m_qiRockets;

	BuildableVisibility m_hideLevel;
	bool m_bHideShells = false;
	bool m_bHideRockets = false;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	void ApplyVisibilityOptions() override;

	int GetMaxCells() override;
	CFFBuildableObject* GetBuildable();
	void OnBuiltChanged(bool bBuilt) override;

	void Init() override;
	void VidInit() override;

	void MsgFunc_SentryLevelMsg(bf_read& msg);
	void MsgFunc_SentryStatusMsg(bf_read& msg);
};

#endif