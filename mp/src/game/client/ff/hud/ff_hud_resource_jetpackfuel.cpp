#include "cbase.h"

#include "ff_hud_resource_jetpackfuel.h"
#include "c_ff_player.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudResourceJetpackFuel);

CHudResourceJetpackFuel::CHudResourceJetpackFuel(
	const char* pElementName)
	: CHudElement(pElementName),
	BaseClass(NULL, "HudResourceJetpackFuel"),
	m_wsFull(L"Full"),
	m_wsRegenerating(L"Regenerating...")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	SetHeaderText(L"Jetpack");
	SetHeaderIconChar('h');
	SetUseToggleText(true);
	SetText(m_wsRegenerating);

	m_qiJetpackFuel = nullptr;
}

CHudResourceJetpackFuel::~CHudResourceJetpackFuel()
{
}

void CHudResourceJetpackFuel::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_JetpackFuel");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Full");
	if (localized)
		m_wsFull = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_InUse");
	if (localized)
		m_wsInUse = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Recovering");
	if (localized)
		m_wsRecovering = localized;
}

void CHudResourceJetpackFuel::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	AddShowTextOption(kvPanelSpecificOptions);

	AddHideWhenFullOption(kvPanelSpecificOptions);

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

	AddItemOption(
		kvPanelSpecificOptions,
		"Recovery",
		"#HudPanel_Recovering",
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
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
		else
			SetHiddenBits(HIDEHUD_ALWAYS);
	}

	if (iShowText != -1)
		SetUseToggleText(iShowText == 1);

	if (valueFormat.has_value())
		m_qiJetpackFuel->SetAmountFormat(
			valueFormat.value());

	int iDisableRecovery
		= GetInt(
			"DisableRecovery",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iDisableRecovery != -1)
		m_bShowRecovery = iDisableRecovery == 0;

	if (m_bShowRecovery)
		EnableItem(m_qiRecovery);
	else
		DisableItem(m_qiRecovery);

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

	m_qiRecovery = AddItem("Recovery");
	m_qiRecovery->SetLabel("#HudPanel_Recovering");
	m_qiRecovery->SetIcon('f');
	m_qiRecovery->SetAmountFormat(FORMAT_RAW);
	m_qiRecovery->SetAmountDecimalPlaces(1);
	m_qiRecovery->SetAmountMax(JETPACK_RECOVERY_TIME);
	m_qiRecovery->SetAmount(JETPACK_RECOVERY_TIME * 0.6f);

	AddPanelToHudOptions(
		"JetpackFuel",
		"#HudPanel_JetpackFuel",
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceJetpackFuel::VidInit()
{
	m_qiJetpackFuel->SetAmount(0);
	m_qiRecovery->SetAmount(0);
	SetText(m_wsRegenerating);
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

void CHudResourceJetpackFuel::Paint()
{
	if (!IsInPreviewMode())
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		m_qiJetpackFuel->SetAmount(pPlayer->m_iJetpackFuel);

		const float flRecoveryTime = Max(
			0.0f,
			pPlayer->m_flJetpackCooldown - gpGlobals->curtime);

		m_qiRecovery->SetAmount(flRecoveryTime);

		if (pPlayer->IsJetpacking())
			SetText(m_wsInUse);
		else if (pPlayer->m_iJetpackFuel >= JETPACK_MAXFUEL)
			SetText(m_wsFull);
		else if (flRecoveryTime > 0.0f || pPlayer->m_bJetpackDepleted)
			SetText(m_wsRecovering);
		else
			SetText(m_wsRegenerating);
	}

	BaseClass::Paint();
}
