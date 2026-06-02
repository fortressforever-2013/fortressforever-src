#ifndef FF_HUDResourcePipeTrap_H
#define FF_HUDResourcePipeTrap_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"

using namespace vgui;
using namespace FFQuantityHelper;

class CHudResourceMedpacks
	: public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourceMedpacks, FFQuantityPanel);
	CHudResourceMedpacks(const char* pElementName);
	~CHudResourceMedpacks();

	FFQuantityItem* m_qiMedpacks;

	std::wstring m_wsFull;
	std::wstring m_wsRegenerating; 

	bool m_bShowPanel = false;

	virtual void ApplySchemeSettings(
		IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	void Init();
	void VidInit();

	void MsgFunc_MedpacksMsg(bf_read& msg);
};

#endif