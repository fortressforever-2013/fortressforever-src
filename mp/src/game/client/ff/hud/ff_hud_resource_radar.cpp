#include "cbase.h"

#include "ff_hud_resource_radar.h"
#include "c_ff_player.h"
#include "ff_playerclass_parse.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudResourceRadar);

CHudResourceRadar::CHudResourceRadar(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourceRadar")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	SetHeaderText(L"Radar");
	SetHeaderIconChar('=');
	SetUseToggleText(true);
	SetText(m_wsRegenerating);
}

void CHudResourceRadar::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	const wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Radar");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Full");
	if (localized)
		m_wsFull = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;
}

void CHudResourceRadar::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	AddShowTextOption(kvPanelSpecificOptions);

	AddHideWhenFullOption(kvPanelSpecificOptions);

	AddItemOption(
		kvPanelSpecificOptions,
		"RadarScans",
		"#HudPanel_RadarScans",
		{
			{
				ITEM_FORMAT,
				"ValueFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});
}

KeyValues* CHudResourceRadar::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	kvPreset->SetInt("showHeaderIcon", 1);
	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_BOTTOM);

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("HideWhenFull", false);
	kvPanelSpecificValues->SetBool("ShowText", true);

	kvPanelSpecificValues->SetInt("ValueFormat", FORMAT_OVER_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudResourceRadar::ApplyStyleData(
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

	const int iHideWhenFull
		= GetInt(
			"HideWhenFull",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	if (iHideWhenFull != -1)
		m_bHideWhenFull = iHideWhenFull == 1;

	const int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	const int iShowText
		= GetInt(
			"ShowText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	const std::optional<AmountFormat> valueFormat
		= GetAmountFormat(
			"ValueFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iShowPanel != -1)
		m_bShowPanel = iShowPanel == 1;

	if (iShowText != -1)
		SetUseToggleText(iShowText == 1);

	if (valueFormat.has_value())
		m_qiScans->SetAmountFormat(
			valueFormat.value());

	SetHiddenBits(m_bShowPanel
		? (IsInPreviewMode() ? 0 : DEFAULT_HIDDEN_BITS)
		: HIDEHUD_ALWAYS);
}

void CHudResourceRadar::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);

	m_qiScans = AddItem("HudResourceRadarScans");
	m_qiScans->SetLabel("#HudPanel_RadarScans");
	m_qiScans->SetIcon('+');
	m_qiScans->SetAmountFormat(FORMAT_OVER_MAX);
	m_qiScans->SetAmountMax(10);

	AddPanelToHudOptions(
		"Radar",
		"#HudPanel_Radar",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceRadar::VidInit()
{
	m_qiScans->SetAmount(0);
	SetText(m_wsRegenerating);
}

//-----------------------------------------------------------------------------
// Purpose: Convert Scout cells into available scans and update capacity from the Scout class definition
//-----------------------------------------------------------------------------
void CHudResourceRadar::OnTick()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!IsInPreviewMode() && engine->IsInGame()
		&& pPlayer && pPlayer->GetClassSlot() == CLASS_SCOUT)
	{
		PLAYERCLASS_FILE_INFO_HANDLE hClassInfo = LookupPlayerClassInfoSlot("scout");
		if (hClassInfo != GetInvalidPlayerClassInfoHandle())
		{
			const CFFPlayerClassInfo* pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);
			if (pClassInfo)
				m_qiScans->SetAmountMax(pClassInfo->m_iMaxCells / RADAR_COST);
		}

		const int iCells = Max(0, pPlayer->GetAmmoCount(AMMO_CELLS));
		m_qiScans->SetAmount(iCells / RADAR_COST);

		if (m_qiScans->GetAmountMax() > 0
			&& m_qiScans->GetAmount() >= m_qiScans->GetAmountMax())
			SetText(m_wsFull);
		else
			SetText(m_wsRegenerating);
	}

	BaseClass::OnTick();
}

bool CHudResourceRadar::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (m_bHideWhenFull && m_qiScans->GetAmountMax() > 0
		&& m_qiScans->GetAmount() >= m_qiScans->GetAmountMax())
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	return engine->IsInGame() && pPlayer && pPlayer->GetClassSlot() == CLASS_SCOUT;
}