#include "cbase.h"

#include "ff_hud_resource_jetpackfuel.h"
#include "c_ff_player.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"
#include "ff_playerclass_parse.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudResourceJetpackFuel);

CHudResourceJetpackFuel::CHudResourceJetpackFuel(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourceJetpackFuel"),
	m_wsFull(L"Full"),
	m_wsRefueling(L"Refueling...")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	SetHeaderText(L"Jetpack");
	SetHeaderIconChar('h');
	SetUseToggleText(true);
	SetText(m_wsRefueling);

	m_qiJetpackFuel = nullptr;
}

CHudResourceJetpackFuel::~CHudResourceJetpackFuel()
{
}

void CHudResourceJetpackFuel::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	const wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_JetpackFuel");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Full");
	if (localized)
		m_wsFull = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Refueling");
	if (localized)
		m_wsRefueling = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_InUse");
	if (localized)
		m_wsInUse = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Cooling");
	if (localized)
		m_wsCooling = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_CellsNeeded");
	if (localized)
		m_wsCellsNeeded = localized;
}

void CHudResourceJetpackFuel::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	AddShowTextOption(kvPanelSpecificOptions);

	AddHideWhenFullOption(kvPanelSpecificOptions);

	AddItemOption(
		kvPanelSpecificOptions,
		"Cells",
		"#FF_ITEM_CELLS",
		{
			{
				ITEM_VISIBILITY,
				"DisableCells",
				GetQuantityVisibilityOptions(),
				false
			},
			{
				ITEM_FORMAT,
				"CellsFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	AddItemOption(
		kvPanelSpecificOptions,
		"Fuel",
		"#FF_ITEM_FUEL",
		{
			{
				ITEM_FORMAT,
				"ValueFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	// Retain the saved HUD style keys for compatibility with existing layouts.
	AddItemOption(
		kvPanelSpecificOptions,
		"Recovery",
		"#HudPanel_Cooling",
		{
			{
				ITEM_VISIBILITY,
				"DisableRecovery",
				GetQuantityVisibilityOptions(),
				false
			}
		});
}

KeyValues* CHudResourceJetpackFuel::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("HideWhenFull", false);
	kvPanelSpecificValues->SetBool("ShowText", true);

	kvPanelSpecificValues->SetInt("ValueFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetBool("DisableRecovery", false);
	kvPanelSpecificValues->SetBool("DisableCells", false);
	kvPanelSpecificValues->SetInt("CellsFormat", FORMAT_OVER_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudResourceJetpackFuel::ApplyStyleData(
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

	const int iHideWhenFull = GetInt("HideWhenFull", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
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
	{
		m_bShowPanel = (iShowPanel == 1);

		if (m_bShowPanel)
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
		else
			SetHiddenBits(HIDEHUD_ALWAYS);
	}

	if (iShowText != -1)
		SetUseToggleText(iShowText == 1);

	if (valueFormat.has_value())
		m_qiJetpackFuel->SetAmountFormat(
			valueFormat.value());

	const int iDisableCooling
		= GetInt(
			"DisableRecovery",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iDisableCooling != -1)
		m_bShowCooling = iDisableCooling == 0;

	if (m_bShowCooling)
		EnableItem(m_qiCooling);
	else
		DisableItem(m_qiCooling);

	const int iDisableCells = GetInt(
		"DisableCells", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iDisableCells == 1)
		DisableItem(m_qiCells);
	else if (iDisableCells == 0)
		EnableItem(m_qiCells);

	const std::optional<AmountFormat> cellsFormat = GetAmountFormat(
		"CellsFormat", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (cellsFormat.has_value())
		m_qiCells->SetAmountFormat(cellsFormat.value());

	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
		{
			SetHiddenBits(0);
		}
		else
		{
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
		}
	}
}

void CHudResourceJetpackFuel::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiJetpackFuel = AddItem("HudResourceJetpackFuel");
	m_qiJetpackFuel->SetLabel("#FF_ITEM_FUEL");
	m_qiJetpackFuel->SetIcon('m');
	m_qiJetpackFuel->SetAmountFormat(FORMAT_OVER_MAX);
	m_qiJetpackFuel->SetAmountMax(JETPACK_MAXFUEL);

	m_qiCells = AddItem("Cells");
	m_qiCells->SetLabel("#FF_ITEM_CELLS");
	m_qiCells->SetIcon('p');
	m_qiCells->SetAmountFormat(FORMAT_OVER_MAX);
	m_qiCells->SetAmount(100);

	m_qiCooling = AddItem("Recovery");
	m_qiCooling->SetLabel("#HudPanel_Cooling");
	m_qiCooling->SetIcon('f');
	m_qiCooling->SetAmountFormat(FORMAT_RAW);
	m_qiCooling->SetAmountDecimalPlaces(1);
	m_qiCooling->SetAmountMax(ff_jetpack_cooldown.GetFloat());
	m_qiCooling->SetAmount(ff_jetpack_cooldown.GetFloat() * 0.6f);

	AddPanelToHudOptions(
		"JetpackFuel",
		"#HudPanel_JetpackFuel",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceJetpackFuel::VidInit()
{
	m_qiJetpackFuel->SetAmount(0);
	m_qiCells->SetAmount(0);
	m_qiCooling->SetAmount(0);
	SetText(m_wsRefueling);
}

bool CHudResourceJetpackFuel::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (!engine->IsInGame())
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer)
		return false;

	if (m_bHideWhenFull && pPlayer->m_iJetpackFuel >= JETPACK_MAXFUEL)
		return false;

	return pPlayer->GetClassSlot() == CLASS_PYRO
		&& !FF_IsPlayerSpec(pPlayer)
		&& FF_HasPlayerPickedClass(pPlayer)
		&& pPlayer->m_bCanUseJetpack;
}

//-----------------------------------------------------------------------------
// Purpose: Update cell capacity before layout so the fuel panel reserves the correct text width
//-----------------------------------------------------------------------------
void CHudResourceJetpackFuel::OnThink()
{
	// Class records are indexed by script basename, including the prefix.
	// Update before layout so the amount reserves space for the actual capacity.
	const PLAYERCLASS_FILE_INFO_HANDLE hClassInfo = LookupPlayerClassInfoSlot("ff_playerclass_pyro");
	if (hClassInfo != GetInvalidPlayerClassInfoHandle())
	{
		const CFFPlayerClassInfo* pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);
		if (pClassInfo)
			m_qiCells->SetAmountMax(pClassInfo->m_iMaxCells);
	}
	BaseClass::OnThink();
}

// Refresh fuel, cells, cooling time and status while preserving preview samples.
void CHudResourceJetpackFuel::Paint()
{
	if (!IsInPreviewMode())
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		m_qiJetpackFuel->SetAmount(pPlayer->m_iJetpackFuel);
		m_qiCells->SetAmount(pPlayer->GetAmmoCount(AMMO_CELLS));

		const float flCoolingTime = Max(
			0.0f,
			pPlayer->m_flJetpackCooldown - gpGlobals->curtime);

		m_qiCooling->SetAmountMax(ff_jetpack_cooldown.GetFloat());
		m_qiCooling->SetAmount(flCoolingTime);

		if (pPlayer->IsJetpacking())
			SetText(m_wsInUse);
		else if (pPlayer->m_iJetpackFuel >= JETPACK_MAXFUEL)
			SetText(m_wsFull);
		else if (flCoolingTime > 0.0f)
			SetText(m_wsCooling);
		else if (pPlayer->GetAmmoCount(AMMO_CELLS) <= 0)
			SetText(m_wsCellsNeeded);
		else
			SetText(m_wsRefueling);
	}

	BaseClass::Paint();
}
