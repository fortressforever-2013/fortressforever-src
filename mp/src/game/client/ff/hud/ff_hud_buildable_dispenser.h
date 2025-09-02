#ifndef FF_HUDBUILDABLEDISPENSER_H
#define FF_HUDBUILDABLEDISPENSER_H

#include "ff_hud_buildable.h"

#include "c_ff_player.h"

using namespace vgui;

class CHudBuildableDispenser
	: public CHudBuildable
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildableDispenser, CHudBuildable);
	CHudBuildableDispenser(const char* pElementName);
	~CHudBuildableDispenser();

	FFQuantityItem* m_qiAmmo;

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