#include "cbase.h"

#include "ff_hud_resource_medpacks.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudResourceMedpacks);
DECLARE_HUD_MESSAGE(CHudResourceMedpacks, MedpacksMsg);

CHudResourceMedpacks::CHudResourceMedpacks(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourceMedpacks"),
	m_wsFull(L"Full"),
	m_wsRegenerating(L"Regenerating...")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS | HIDEHUD_NOTMEDIC);

	SetHeaderText(L"Medpacks");
	SetHeaderIconChar('c');
	SetUseToggleText(true);
	SetText(m_wsRegenerating);

	m_qiMedpacks = nullptr;
}

CHudResourceMedpacks::~CHudResourceMedpacks()
{
}

void CHudResourceMedpacks::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Medpacks");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Full");
	if (localized)
		m_wsFull = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;
}

void CHudResourceMedpacks::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	AddShowTextOption(kvPanelSpecificOptions);

	AddHideWhenFullOption(kvPanelSpecificOptions);

	AddItemOption(
		kvPanelSpecificOptions,
		"Medpacks",
		"#FF_ITEM_MEDPACKS",
		{
			{
				ITEM_FORMAT,
				"ValueFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});
}

KeyValues* CHudResourceMedpacks::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("HideWhenFull", false);
	kvPanelSpecificValues->SetBool("ShowText", true);

	kvPanelSpecificValues->SetInt("ValueFormat", FORMAT_OVER_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudResourceMedpacks::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	FFQuantityPanel::ApplyStyleData(
		kvStyleData,
		kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	int iHideWhenFull = GetInt("HideWhenFull", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideWhenFull != -1)
		m_bHideWhenFull = iHideWhenFull == 1;

	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iShowText
		= GetInt(
			"ShowText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountFormat> valueFormat
		= GetAmountFormat(
			"ValueFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iShowPanel != -1)
	{
		m_bShowPanel = (iShowPanel == 1);

		if (m_bShowPanel)
			SetHiddenBits(DEFAULT_HIDDEN_BITS | HIDEHUD_NOTMEDIC);
		else
			SetHiddenBits(HIDEHUD_ALWAYS);
	}

	if (iShowText != -1)
		SetUseToggleText(iShowText == 1);

	if (valueFormat.has_value())
		m_qiMedpacks->SetAmountFormat(
			valueFormat.value());

	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
		{
			SetHiddenBits(0);
		}
		else
		{
			SetHiddenBits(DEFAULT_HIDDEN_BITS | HIDEHUD_NOTMEDIC);
		}
	}
}

void CHudResourceMedpacks::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);
	HOOK_HUD_MESSAGE(CHudResourceMedpacks, MedpacksMsg);

	m_qiMedpacks = AddItem("HudResourceMedpacks");
	m_qiMedpacks->SetLabel("#FF_ITEM_MEDPACKS");
	m_qiMedpacks->SetIcon('c');
	m_qiMedpacks->SetAmountFormat(FORMAT_OVER_MAX);
	m_qiMedpacks->SetAmountMax(5);

	AddPanelToHudOptions(
		"Medpacks",
		"#HudPanel_Medpacks",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceMedpacks::VidInit()
{
	m_qiMedpacks->SetAmount(0);
	SetText(m_wsRegenerating);
}

void CHudResourceMedpacks::MsgFunc_MedpacksMsg(
	bf_read& msg)
{
	int iMaxMedpacks = static_cast<int>(msg.ReadByte());
	int iNumMedpacks = static_cast<int>(msg.ReadByte());

	SetMedpacks(iNumMedpacks, iMaxMedpacks);
}

void CHudResourceMedpacks::SetMedpacks(int iNumMedpacks, int iMaxMedpacks)
{
	m_qiMedpacks->SetAmount(iNumMedpacks);
	m_qiMedpacks->SetAmountMax(iMaxMedpacks);

	if (iMaxMedpacks > 0 && iNumMedpacks >= iMaxMedpacks)
		SetText(m_wsFull);
	else
		SetText(m_wsRegenerating);
}

bool CHudResourceMedpacks::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (m_bHideWhenFull && m_qiMedpacks->GetAmountMax() > 0
		&& m_qiMedpacks->GetAmount() >= m_qiMedpacks->GetAmountMax())
		return false;

	return true;
}