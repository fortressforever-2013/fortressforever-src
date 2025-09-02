#ifndef FF_HUDBUILDABLESENTRY_H
#define FF_HUDBUILDABLESENTRY_H

#include "ff_hud_buildable.h"

#include "c_ff_player.h"

using namespace vgui;

class CHudBuildableSentry
	: public CHudBuildable
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildableSentry, CHudBuildable);
	CHudBuildableSentry(const char* pElementName);
	~CHudBuildableSentry();

	FFQuantityItem* m_qiLevel;
	FFQuantityItem* m_qiShells;
	FFQuantityItem* m_qiRockets;

	bool m_bHideLevel;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	void ApplyDisplayOptions() override;

	int GetMaxCells() override;
	CFFBuildableObject* GetBuildable();
	void OnBuiltChanged(bool bBuilt) override;

	void Init() override;
	void VidInit() override;

	void MsgFunc_SentryLevelMsg(bf_read& msg);
	void MsgFunc_SentryStatusMsg(bf_read& msg);
};

#endif