#include "cbase.h"

#include "ff_hud_buildable_mancannon.h"

using namespace vgui;

KeyValues* CHudBuildableManCannon::s_kvVisibilityOptions;

DECLARE_HUDELEMENT(CHudBuildableManCannon);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, ManCannonMsg);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, ManCannonAvailabilityMsg);
DECLARE_HUD_MESSAGE(CHudBuildableManCannon, FF_BuildTimer);

CHudBuildableManCannon::CHudBuildableManCannon(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudBuildableManCannon"),
	m_wsDeployed(L"Deployed"),
	m_wsDeploying(L"Deploying..."),
	m_wsNotAvailable(L"Not Available"),
	m_wsNotDeployed(L"Not Deployed"),
	m_showPanel(ALWAYS),
	m_bShowText(true),
	m_bDisableBuildTimer(false)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);

	SetHeaderText(L"ManCannon");
	SetHeaderIconChar('6');
	SetUseToggleText(true);
	SetText(m_wsNotAvailable);

	m_qiBuildProgress = nullptr;
	m_qiHealth = nullptr;

	if (!s_kvVisibilityOptions)
	{
		KeyValues* kvVisibilityDisplayOptions
			= new KeyValues("Values");

		kvVisibilityDisplayOptions->SetString(
			std::to_string(NEVER).c_str(),
			"#HudPanel_VisibilityNever");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(ALWAYS).c_str(),
			"#HudPanel_VisibilityAlways");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(ON_BUILD).c_str(),
			"#HudPanel_VisibilityOnBuild");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(IF_BUILT).c_str(),
			"#HudPanel_VisibilityIfBuilt");

		s_kvVisibilityOptions
			= kvVisibilityDisplayOptions;
	}
}

CHudBuildableManCannon::~CHudBuildableManCannon()
{
}

void CHudBuildableManCannon::ApplySchemeSettings(
	IScheme* pScheme)
{
	FFQuantityPanel::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_ManCannon");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Deployed");
	if (localized)
		m_wsDeployed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Deploying");
	if (localized)
		m_wsDeploying = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotDeployed");
	if (localized)
		m_wsNotDeployed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotAvailable");
	if (localized)
		m_wsNotAvailable = localized;
}

void CHudBuildableManCannon::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	KeyValues* kvShowPanelOptions = s_kvVisibilityOptions->MakeCopy();
	kvShowPanelOptions->SetString(
		std::to_string(IF_AVAILABLE).c_str(), "#HudPanel_VisibilityIfAvailable");

	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvShowPanelOptions,
		ALWAYS,
		0);

	AddShowTextOption(kvPanelSpecificOptions);

	// Aligns with HealthFormat

	AddItemOption(
		kvPanelSpecificOptions,
		"Health",
		"#FF_ITEM_HEALTH",
		{
			{
				ITEM_FORMAT,
				"HealthFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	AddAlertOptions(kvPanelSpecificOptions, "#HudPanel_Option_DamageAlert", 0);

	KeyValues* kvBuildTimerOptions = GetCountDirectionOptions();

	AddItemOption(
		kvPanelSpecificOptions,
		"BuildTimer",
		"#HudPanel_Option_BuildTimer",
		{
			{
				ITEM_VISIBILITY,
				"DisableBuildTimer",
				GetQuantityVisibilityOptions(),
				false
			},
			{
				ITEM_FORMAT,
				"InvertBuildTimer",
				kvBuildTimerOptions,
				false
			}
		});
}

KeyValues* CHudBuildableManCannon::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetBool("ShowText", true);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);
	kvPanelSpecificValues->SetBool("InvertBuildTimer", false);

	kvPanelSpecificValues->SetInt("HealthFormat", FORMAT_OVER_MAX);
	AddAlertDefaults(kvPanelSpecificValues);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudBuildableManCannon::ApplyStyleData(
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

	std::optional<AmountFormat> healthFormat
		= GetAmountFormat(
			"HealthFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (healthFormat.has_value())
	{
		m_qiHealth->SetAmountFormat(
			healthFormat.value());
	}

	std::optional<BuildableVisibility> showPanel
		= GetBuildableVisibility(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iShowText
		= GetInt(
			"ShowText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableBuildTimer
		= GetInt(
			"DisableBuildTimer",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues,
			-1);

	int iInvertBuildTimer
		= GetInt(
			"InvertBuildTimer",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iInvertBuildTimer != -1)
	{
		m_bInvertBuildTimer = iInvertBuildTimer == 1;

		m_qiBuildProgress->SetAmountDecimalPlaces(m_bInvertBuildTimer ? 1 : 0);

		m_qiBuildProgress->SetLabel(
			m_bInvertBuildTimer
			? "#FF_ITEM_TIMELEFT"
			: "#FF_ITEM_PROGRESS");

		m_qiBuildProgress->SetAmountFormat(
			m_bInvertBuildTimer
			? FORMAT_RAW
			: FORMAT_PERCENTAGE);
	}

	bool bApplyVisibilityOptions = false;

	if (showPanel.has_value()
		&& Change(m_showPanel, showPanel.value()))
	{
		bApplyVisibilityOptions = true;
	}

	if (iShowText != -1
		&& Change(m_bShowText, iShowText == 1))
	{
		bApplyVisibilityOptions = true;
	}

	if (iDisableBuildTimer != -1
		&& Change(m_bDisableBuildTimer, iDisableBuildTimer == 1))
	{
		bApplyVisibilityOptions = true;
	}

	if (bApplyVisibilityOptions || kvStyleData->GetInt("previewMode", -1) != -1)
		ApplyVisibilityOptions();
}

std::optional<BuildableVisibility> CHudBuildableManCannon::GetBuildableVisibility(
	const char* keyName,
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	int value
		= GetInt(
			keyName,
			kvStyleData,
			kvDefaultStyleData,
			-1);

	return value == -1
		? std::optional<BuildableVisibility>()
		: static_cast<BuildableVisibility>(value);
}

void CHudBuildableManCannon::ApplyVisibilityOptions()
{
	if (IsInPreviewMode())
	{
		SetHiddenBits(m_showPanel == NEVER ? HIDEHUD_ALWAYS : 0);

		SetText(m_wsDeployed);
		SetUseToggleText(m_bShowText);
		SetToggleTextVisible(m_bShowText);

		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiHealth);
		ShowItem(m_qiHealth);

		return;
	}

	if (m_bDeployed)
		ShowItem(m_qiHealth);
	else
		HideItem(m_qiHealth);

	if (m_bDeploying)
		SetText(m_wsDeploying);
	else if (m_bDeployed)
		SetText(m_wsDeployed);
	else
		UpdateAvailabilityText();

	if (m_bCanHaveManCannon)
	{
		switch (m_showPanel)
		{
		case ALWAYS:
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
			break;
		case IF_AVAILABLE:
			SetHiddenBits((m_bHasManCannon || m_bDeploying || m_bDeployed)
				? DEFAULT_HIDDEN_BITS
				: HIDEHUD_ALWAYS);
			break;
		case IF_BUILT:
			// Show only if deployed (built)
			SetHiddenBits(m_bDeployed
				? DEFAULT_HIDDEN_BITS
				: HIDEHUD_ALWAYS);
			break;
		case ON_BUILD:
			// Show if deploying (building) or deployed (built)
			SetHiddenBits((m_bDeploying || m_bDeployed)
				? DEFAULT_HIDDEN_BITS
				: HIDEHUD_ALWAYS);
			break;
		case NEVER:
		default:
			SetHiddenBits(HIDEHUD_ALWAYS);
			break;
		}
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}

	SetUseToggleText(m_bShowText);
	SetToggleTextVisible(m_bShowText);

	if (m_bDeploying && !m_bDisableBuildTimer)
	{
		EnableItem(m_qiBuildProgress);
		DisableItem(m_qiHealth);
	}
	else
	{
		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiHealth);
	}
}

void CHudBuildableManCannon::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, ManCannonMsg);
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, ManCannonAvailabilityMsg);
	HOOK_HUD_MESSAGE(CHudBuildableManCannon, FF_BuildTimer);

	m_qiHealth = AddItem("Health");
	m_qiHealth->SetLabel("#FF_ITEM_HEALTH");
	m_qiHealth->SetIcon('a');
	m_qiHealth->SetAmountFormat(FORMAT_PERCENTAGE);
	m_qiHealth->SetAmountMax(100);

	m_qiBuildProgress = AddItem("BuildProgress");
	m_qiBuildProgress->SetLabel("#FF_ITEM_PROGRESS");
	m_qiBuildProgress->SetIcon('f');
	m_qiBuildProgress->SetAmountFormat(FORMAT_PERCENTAGE);

	AddPanelToHudOptions(
		"ManCannon",
		"#HudPanel_ManCannon",
		"Buildables",
		"#HudPanel_Buildables");
}

void CHudBuildableManCannon::VidInit()
{
	ClearAlert();
	m_bDeployed = false;
	m_bDeploying = false;

	UpdateAvailabilityText();

	m_qiHealth->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);

	HideItem(m_qiHealth);
	DisableItem(m_qiBuildProgress);

	ApplyVisibilityOptions();
}

void CHudBuildableManCannon::Paint()
{
	if (!IsInPreviewMode() && m_bDeploying)
	{
		float flTimeElapsed
			= gpGlobals->curtime - m_flBuildStartTime;

		if (m_flBuildDuration > flTimeElapsed)
			m_qiBuildProgress->SetAmount(
				m_bInvertBuildTimer ? m_flBuildDuration - flTimeElapsed : flTimeElapsed);
	}

	FFQuantityPanel::Paint();
}

void CHudBuildableManCannon::UpdateAvailabilityText()
{
	if (!m_bDeploying && !m_bDeployed)
	{
		if (m_bHasManCannon)
			SetText(m_wsNotDeployed);
		else
			SetText(m_wsNotAvailable);
	}
}

void CHudBuildableManCannon::MsgFunc_ManCannonAvailabilityMsg(bf_read& msg)
{
	const bool bCanHave = msg.ReadByte() != 0;
	const bool bHas = msg.ReadByte() != 0;
	SetAvailability(bCanHave, bHas);
}

void CHudBuildableManCannon::SetAvailability(bool bCanHave, bool bHas)
{
	m_bCanHaveManCannon = bCanHave;
	m_bHasManCannon = bHas;
	UpdateAvailabilityText();
	ApplyVisibilityOptions();
}

void CHudBuildableManCannon::MsgFunc_ManCannonMsg(
	bf_read& msg)
{
	BuildState_t state
		= static_cast<BuildState_t>(msg.ReadByte());

	const int iHealth = msg.ReadByte();
	const int iMaxHealth = msg.ReadByte();
	SetBuildState(state, iHealth, iMaxHealth);
}

void CHudBuildableManCannon::SetBuildState(BuildState_t state, int iHealth, int iMaxHealth)
{
	if (state == BUILDSTATE_BUILT && m_bDeployed && iHealth < m_qiHealth->GetAmount())
		TriggerAlert();
	else if (state != BUILDSTATE_BUILT)
		ClearAlert();

	m_qiBuildProgress->SetAmount(0.0f);

	switch (state)
	{
		case BUILDSTATE_NOTBUILT:
		{
			m_bDeployed = false;
			m_bDeploying = false;
			UpdateAvailabilityText();
			HideItem(m_qiHealth);
			DisableItem(m_qiBuildProgress);
			break;
		}
		case BUILDSTATE_BUILDING:
		{
			m_bDeployed = false;
			m_bDeploying = true;
			SetText(m_wsDeploying);
			DisableItem(m_qiHealth);
			EnableItem(m_qiBuildProgress);
			break;
		}
		case BUILDSTATE_BUILT:
		{
			m_qiHealth->SetAmountMax(iMaxHealth);
			m_qiHealth->SetAmount(iHealth);
			m_bDeployed = true;
			m_bDeploying = false;
			SetText(m_wsDeployed);
			ShowItem(m_qiHealth);
			DisableItem(m_qiBuildProgress);
			break;
		}
	}

	UpdateAvailabilityText();
	ApplyVisibilityOptions();
}

void CHudBuildableManCannon::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != FF_BUILD_MANCANNON)
		return;

	const float flBuildDuration = msg.ReadFloat();
	SetBuildTimer(flBuildDuration);
}

void CHudBuildableManCannon::SetBuildTimer(float flBuildDuration)
{
	m_bDeploying = true;
	SetText(m_wsDeploying);

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(m_bInvertBuildTimer ? m_flBuildDuration : 0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	ApplyVisibilityOptions();
}