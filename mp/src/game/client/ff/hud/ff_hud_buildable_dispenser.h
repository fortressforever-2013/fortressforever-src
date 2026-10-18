#ifndef FF_HUD_BUILDABLE_DISPENSER_H
#define FF_HUD_BUILDABLE_DISPENSER_H

#include "ff_hud_buildable.h"

#include "c_ff_player.h"


class CHudBuildableDispenser
	: public CHudBuildable
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildableDispenser, CHudBuildable);
	CHudBuildableDispenser(const char* pElementName);
	~CHudBuildableDispenser();

	vgui::FFQuantityItem* m_qiAmmo;
	vgui::FFQuantityItem* m_storedItems[4];
	bool m_bHideAmmo = false;
	bool m_hideStoredItems[4] = { true, true, true, false };

	void ApplyVisibilityOptions() override;
	void OnTick() override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	int GetMaxCells() override;
	CFFBuildableObject* GetBuildable();
	void OnBuiltChanged(bool bBuilt) override;

	void Init() override;
	void VidInit() override;

	void MsgFunc_DispenserMsg(bf_read& msg);
};

#endif
