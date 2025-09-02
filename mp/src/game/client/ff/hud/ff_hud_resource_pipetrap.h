#ifndef FF_HUDResourcePipeTrap_H
#define FF_HUDResourcePipeTrap_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"

using namespace vgui;

class CHudResourcePipeTrap
	: public CHudElement, public FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourcePipeTrap, FFQuantityPanel);
	CHudResourcePipeTrap(const char* pElementName);
	~CHudResourcePipeTrap();

	FFQuantityItem* m_qiPipeLaid;

	int m_iNumPipes = 0;

	std::wstring m_wsDeployed;
	std::wstring m_wsNeedAmmo;
	std::wstring m_wsNotDeployed;

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

	void OnTick() override;

	void MsgFunc_PipeMsg(bf_read& msg);
};

#endif