#ifndef FF_HUD_RESOURCE_PIPETRAP_H
#define FF_HUD_RESOURCE_PIPETRAP_H

#include "ff_quantitypanel.h"

#include "hudelement.h"
#include "hud_macros.h"

#include "c_ff_player.h"
#include "iclientmode.h"


class CHudResourcePipeTrap
	: public CHudElement, public vgui::FFQuantityPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudResourcePipeTrap, vgui::FFQuantityPanel);
	CHudResourcePipeTrap(const char* pElementName);
	~CHudResourcePipeTrap();

	vgui::FFQuantityItem* m_qiPipeLaid;

	int m_iNumPipes = 0;

	std::wstring m_wsDeployed;
	std::wstring m_wsNeedAmmo;
	std::wstring m_wsNotDeployed;

	enum PanelVisibility
	{
		NEVER = 0,
		ALWAYS = 1,
		IF_DEPLOYED = 2
	};

	PanelVisibility m_showPanel = ALWAYS;
	PanelVisibility m_showPipes = IF_DEPLOYED;
	PanelVisibility m_showText = ALWAYS;
	bool m_bHasPipeLauncher = false;

	virtual void ApplySchemeSettings(
		vgui::IScheme* pScheme) override;

	void AddPanelSpecificOptions(
		KeyValues* kvPanelSpecificOptions) override;
	KeyValues* GetDefaultStyleData() override;
	void ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData) override;

	std::optional<PanelVisibility> GetPanelVisibility(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData);

	void ApplyVisibilityOptions();

	void Init();
	void VidInit();

	void OnTick() override;

	void MsgFunc_PipeMsg(bf_read& msg);
};

#endif