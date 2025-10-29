#ifndef FF_HUD_RESOURCE_MEDPACKS_H
#define FF_HUD_RESOURCE_MEDPACKS_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"

using namespace FFQuantityHelper;

class CHudResourceMedpacks
	: public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourceMedpacks, vgui::FFQuantityPanel);
	CHudResourceMedpacks(const char* pElementName);
	~CHudResourceMedpacks();

	vgui::FFQuantityItem* m_qiMedpacks;

	std::wstring m_wsFull;
	std::wstring m_wsRegenerating; 

	bool m_bShowPanel = false;
	bool m_bHideWhenFull = false;

	virtual void ApplySchemeSettings(
		vgui::IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	bool ShouldDraw() override;
	void Init();
	void VidInit();

	void SetMedpacks(int iNumMedpacks, int iMaxMedpacks);
	void MsgFunc_MedpacksMsg(bf_read& msg);
};

#endif