#include "cbase.h"

using namespace vgui;

#include "ff_hud_buildable.h"
#include <algorithm>

std::vector<CHudBuildable*> CHudBuildable::s_Buildables;
KeyValues* CHudBuildable::s_kvVisibilityOptions;

// We can't use DECLARE_HUD_MESSAGE macro on this base class.
// This saves us having to declare it on every derived class.
static void __MsgFunc_FF_BuildTimer(bf_read& msg)
{
	for (CHudBuildable* pBuildable : CHudBuildable::s_Buildables)
	{
		if (pBuildable)
		{
			bf_read msgCopy = msg;
			pBuildable->MsgFunc_FF_BuildTimer(msgCopy);
		}
	}
}

CHudBuildable::CHudBuildable(
	const char* szElementName,
	const int iBuildType,
	const char* szPanelName,
	const char* szDisplayNameKey)
	: CHudElement(szElementName),
	FFQuantityPanel(NULL, szPanelName),
	m_wsBuilding(L"Building..."),
	m_wsBuilt(L"Built"),
	m_wsCellsNeeded(L"Cells Needed"),
	m_wsNotBuilt(L"Not Built"),
	m_wsReadyToUpgrade(L"Ready to Upgrade")
{
	s_Buildables.push_back(this);

	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(
		DEFAULT_HIDDEN_BITS | HIDEHUD_NOTENGINEER);

	m_showPanel = ALWAYS;
	m_bShowText = true;
	m_hideCells = IF_BUILT;
	m_bDisableBuildTimer = false;

	SetUseToggleText(true);

	m_qiBuildProgress = nullptr;
	m_qiHealth = nullptr;
	m_qiCells = nullptr;

	m_iBuildType = iBuildType;
	m_bBuilt = false;
	m_bBuilding = false;
	m_iCellRequirement = 0;

	m_szDisplayNameKey = szDisplayNameKey;

	m_flBuildStartTime = 0.0f;
	m_flBuildDuration = 0.0f;

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

CHudBuildable::~CHudBuildable()
{
	auto it
		= std::find(
			s_Buildables.begin(),
			s_Buildables.end(),
			this);

	if (it != s_Buildables.end())
	{
		s_Buildables.erase(it);
	}
}

void CHudBuildable::ApplySchemeSettings(IScheme* pScheme)
{
	FFQuantityPanel::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find(m_szDisplayNameKey);
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_NotBuilt");
	if (localized)
		m_wsNotBuilt = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Building");
	if (localized)
		m_wsBuilding = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Built");
	if (localized)
		m_wsBuilt = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_CellsNeeded");
	if (localized)
		m_wsCellsNeeded = localized;
}

void CHudBuildable::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddComboOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		s_kvVisibilityOptions->MakeCopy(),
		ALWAYS,
		0);

	AddShowTextOption(kvPanelSpecificOptions);

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

	AddItemOption(
		kvPanelSpecificOptions,
		"Cells",
		"#FF_ITEM_CELLS",
		{
			{
				ITEM_VISIBILITY,
				"HideCells",
				GetBuildableItemVisibilityOptions(),
				IF_BUILT
			},
			{
				ITEM_FORMAT,
				"CellsFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

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

KeyValues* CHudBuildable::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetBool("ShowText", true);
	kvPanelSpecificValues->SetInt("HideCells", IF_BUILT);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);
	kvPanelSpecificValues->SetBool("InvertBuildTimer", false);

	kvPanelSpecificValues->SetInt("HealthFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetInt("CellsFormat", FORMAT_OVER_MAX);

	AddAlertDefaults(kvPanelSpecificValues);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudBuildable::ApplyStyleData(
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
	std::optional<AmountFormat> cellsFormat
		= GetAmountFormat(
			"CellsFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (healthFormat.has_value())
	{
		m_qiHealth->SetAmountFormat(
			healthFormat.value());
	}

	if (cellsFormat.has_value())
	{
		m_qiCells->SetAmountFormat(
			cellsFormat.value());
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
	std::optional<BuildableVisibility> hideCells
		= GetBuildableVisibility(
			"HideCells",
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

	if (hideCells.has_value()
		&& Change(m_hideCells, hideCells.value()))
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

std::optional<CHudBuildable::BuildableVisibility> CHudBuildable::GetBuildableVisibility(
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

void CHudBuildable::ApplyVisibilityOptions()
{
	const bool bBuilt = IsInPreviewMode() || m_bBuilt;
	const bool bBuilding = !IsInPreviewMode() && m_bBuilding;

	if (IsInPreviewMode())
		SetText(m_wsBuilt);

	switch (m_showPanel)
	{
	case ALWAYS:
		SetHiddenBits(DEFAULT_HIDDEN_BITS | HIDEHUD_NOTENGINEER);
		break;
	case IF_BUILT:
		SetHiddenBits(bBuilt
			? DEFAULT_HIDDEN_BITS | HIDEHUD_NOTENGINEER
			: HIDEHUD_ALWAYS);
		break;
	case ON_BUILD:
		SetHiddenBits(
			bBuilding || bBuilt
			? DEFAULT_HIDDEN_BITS | HIDEHUD_NOTENGINEER
			: HIDEHUD_ALWAYS);
		break;
	case NEVER:
	default:
		SetHiddenBits(HIDEHUD_ALWAYS);
		break;
	}

	if (IsInPreviewMode() && m_showPanel != NEVER)
		SetHiddenBits(0);

	SetUseToggleText(m_bShowText);
	SetToggleTextVisible(m_bShowText);

	switch (m_hideCells)
	{
	case NEVER:
	default:
		EnableItem(m_qiCells);
		break;
	case ALWAYS:
		DisableItem(m_qiCells);
		break;
	case IF_BUILT:
		if (bBuilt)
			DisableItem(m_qiCells);
		else
			EnableItem(m_qiCells);

		break;
	case ON_BUILD:
		if (bBuilt || bBuilding)
			DisableItem(m_qiCells);
		else
			EnableItem(m_qiCells);

		break;
	}

	if (bBuilt)
		ShowItem(m_qiHealth);
	else
		HideItem(m_qiHealth);

	if (bBuilding && !m_bDisableBuildTimer)
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

void CHudBuildable::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);

	// We can't use HOOK_HUD_MESSAGE macro on this base class.
	// This saves us having to declare it on every derived class.
	static bool bBuildTimerHooked = false;
	if (!bBuildTimerHooked)
	{
		usermessages->HookMessage("FF_BuildTimer", __MsgFunc_FF_BuildTimer);
		bBuildTimerHooked = true;
	}

	AddPanelToHudOptions(
		FFQuantityPanel::GetName(),
		m_szDisplayNameKey,
		"Buildables",
		"#HudPanel_Buildables");
}

void CHudBuildable::SetHealth(int iHealth)
{
	if (m_bBuilt && m_iHealth >= 0 && iHealth < m_iHealth)
		TriggerAlert();

	m_iHealth = iHealth;
	m_qiHealth->SetAmount(iHealth);
}

void CHudBuildable::InitItemHealth()
{
	m_qiHealth = AddItem("Health");
	m_qiHealth->SetLabel("#FF_ITEM_HEALTH");
	m_qiHealth->SetIcon('a');
	m_qiHealth->SetAmountFormat(FORMAT_PERCENTAGE);
	m_qiHealth->SetAmountMax(100);
}

void CHudBuildable::InitItemBuildProgress()
{
	m_qiBuildProgress = AddItem("BuildProgress");
	m_qiBuildProgress->SetLabel("#FF_ITEM_PROGRESS");
	m_qiBuildProgress->SetIcon('f');
	m_qiBuildProgress->SetAmountFormat(FORMAT_PERCENTAGE);
}

void CHudBuildable::InitItemCells()
{
	m_qiCells = AddItem("Cells");
	m_qiCells->SetLabel("#FF_ITEM_CELLS");
	m_qiCells->SetIcon('p');
	m_qiCells->SetAmountFormat(FORMAT_OVER_MAX);
	m_qiCells->SetAmountMax(GetMaxCells());
}

void CHudBuildable::VidInit()
{
	ClearAlert();
	m_iHealth = -1;

	m_bBuilt = false;
	m_bBuilding = false;

	SetText(m_wsNotBuilt);

	m_qiHealth->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);
	m_qiCells->SetAmount(0);

	HideItem(m_qiHealth);
	DisableItem(m_qiBuildProgress);

	ApplyVisibilityOptions();
}

void CHudBuildable::OnTick()
{
	FFQuantityPanel::OnTick();

	if (IsInPreviewMode())
		return;

	if (!engine->IsInGame())
		return;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer || pPlayer->GetClassSlot() != CLASS_ENGINEER)
		return;

	CFFBuildableObject* buildable = GetBuildable();
	bool bBuilt = buildable && buildable->IsBuilt();
	// The build timer can arrive before the buildable entity is available on the client.
	bool bBuilding = !bBuilt && (buildable
		|| (m_bBuilding && gpGlobals->curtime < m_flBuildStartTime + m_flBuildDuration));

	// Update building state
	if (bBuilding != m_bBuilding)
	{
		m_bBuilding = bBuilding;

		ApplyVisibilityOptions();
	}

	// Update built state
	if (bBuilt != m_bBuilt)
	{
		m_bBuilt = bBuilt;

		OnBuiltChanged(m_bBuilt);

		ApplyVisibilityOptions();
	}

	if (m_bBuilding)
		SetText(m_wsBuilding);
	else if (pPlayer && pPlayer->GetAmmoCount(AMMO_CELLS) < m_iCellRequirement)
		SetText(m_wsCellsNeeded);
	else if (m_bBuilt)
		SetText(
			m_iCellRequirement > 0
			? m_wsReadyToUpgrade
			: m_wsBuilt);
	else
		SetText(m_wsNotBuilt);

	if (m_hideCells != ALWAYS)
	{
		int iCells
			= C_FFPlayer::GetLocalFFPlayer()
			->GetAmmoCount(AMMO_CELLS);

		iCells = min(iCells, GetMaxCells());
		m_qiCells->SetAmount(iCells);
	}
}

void CHudBuildable::Paint()
{
	float flTimeElapsed
		= gpGlobals->curtime - m_flBuildStartTime;

	if (m_flBuildDuration > flTimeElapsed)
		m_qiBuildProgress->SetAmount(
			m_bInvertBuildTimer ? m_flBuildDuration - flTimeElapsed : flTimeElapsed);

	FFQuantityPanel::Paint();
}

void CHudBuildable::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != m_iBuildType && iBuildType != FF_BUILD_NONE)
		return;

	float flBuildDuration = msg.ReadFloat();
	if (flBuildDuration <= 0)
	{
		m_flBuildDuration = 0.0f;
		m_bBuilding = false;
		ApplyVisibilityOptions();
		return;
	}

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(m_bInvertBuildTimer ? m_flBuildDuration : 0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	m_bBuilding = true;
	ApplyVisibilityOptions();
}

void CHudBuildable::OnBuiltChanged(
	bool bBuilt)
{
	if (bBuilt)
		ShowItem(m_qiHealth);
	else
	{
		ClearAlert();
		m_iHealth = -1;

		HideItem(m_qiHealth);
	}
}
