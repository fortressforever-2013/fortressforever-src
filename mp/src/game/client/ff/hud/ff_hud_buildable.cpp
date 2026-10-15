#include "cbase.h"

#include "ff_hud_buildable.h"

std::vector<CHudBuildable*> CHudBuildable::s_Buildables;
KeyValues* CHudBuildable::s_kvVisibilityDisplayOptions;

// We can't use DECLARE_HUD_MESSAGE macro on this base class.
// This saves us having to declare it on every derived class.
static void __MsgFunc_FF_BuildTimer(bf_read& msg)
{
	for (CHudBuildable* pBuildable : CHudBuildable::s_Buildables)
	{
		if (pBuildable)
		{
			pBuildable->MsgFunc_FF_BuildTimer(msg);
			msg.Reset();
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
		HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER);

	m_showPanel = ALWAYS;
	m_hideText = ALWAYS;
	m_hideCells = IF_BUILT;
	m_bDisableBuildTimer = false;

	SetUseToggleText(true);

	m_qiBuildProgress = nullptr;
	m_qiHealth = nullptr;

	m_iBuildType = iBuildType;
	m_bBuilt = false;
	m_bBuilding = false;
	m_iCellRequirement = 0;

	m_szDisplayNameKey = szDisplayNameKey;

	m_flBuildStartTime = 0.0f;
	m_flBuildDuration = 0.0f;

	if (!s_kvVisibilityDisplayOptions)
	{
		KeyValues* kvVisibilityDisplayOptions
			= new KeyValues("Values");

		kvVisibilityDisplayOptions->SetString(
			std::to_string(NEVER).c_str(),
			"#HudPanel_DisplayNever");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(ALWAYS).c_str(),
			"#HudPanel_DisplayAlways");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(ON_BUILD).c_str(),
			"#HudPanel_DisplayOnBuild");
		kvVisibilityDisplayOptions->SetString(
			std::to_string(IF_BUILT).c_str(),
			"#HudPanel_DisplayIfBuilt");

		s_kvVisibilityDisplayOptions
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
		s_kvVisibilityDisplayOptions->MakeCopy(),
		ALWAYS,
		0);
	AddComboOption(
		kvPanelSpecificOptions,
		"HideText",
		"#HudPanel_Option_HideText",
		s_kvVisibilityDisplayOptions->MakeCopy(),
		NEVER,
		0);
	AddComboOption(
		kvPanelSpecificOptions,
		"HideCells",
		"#HudPanel_Option_HideCells",
		CHudBuildable::s_kvVisibilityDisplayOptions->MakeCopy(),
		IF_BUILT,
		0);
	AddBooleanOption(
		kvPanelSpecificOptions,
		"DisableBuildTimer",
		"#HudPanel_Option_HideBuildTimer",
		false,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"HealthDisplay",
		"#HudPanel_Option_HealthDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"CellsDisplay",
		"#HudPanel_Option_CellsDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
}

KeyValues* CHudBuildable::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("ShowPanel", ALWAYS);
	kvPanelSpecificValues->SetInt("HideText", NEVER);
	kvPanelSpecificValues->SetInt("HideCells", IF_BUILT);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);

	kvPanelSpecificValues->SetInt("HealthDisplay", DISPLAY_MAX);
	kvPanelSpecificValues->SetInt("CellsDisplay", DISPLAY_MAX);

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

	std::optional<AmountDisplay> healthDisplay
		= GetAmountDisplay(
			"HealthDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountDisplay> cellsDisplay
		= GetAmountDisplay(
			"CellsDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (healthDisplay.has_value())
	{
		m_qiHealth->SetAmountDisplay(
			healthDisplay.value());
	}

	if (cellsDisplay.has_value())
	{
		m_qiCells->SetAmountDisplay(
			cellsDisplay.value());
	}

	std::optional<BuildableDisplayOption> showPanel
		= GetBuildableDisplayOption(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<BuildableDisplayOption> hideText
		= GetBuildableDisplayOption(
			"HideText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<BuildableDisplayOption> hideCells
		= GetBuildableDisplayOption(
			"HideCells",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableBuildTimer
		= GetInt(
			"DisableBuildTimer",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues,
			-1);

	bool bApplyDisplayOptions = false;

	if (showPanel.has_value()
		&& Change(m_showPanel, showPanel.value()))
	{
		bApplyDisplayOptions = true;
	}

	if (hideText.has_value()
		&& Change(m_hideText, hideText.value()))
	{
		bApplyDisplayOptions = true;
	}

	if (hideCells.has_value()
		&& Change(m_hideCells, hideCells.value()))
	{
		bApplyDisplayOptions = true;
	}

	if (iDisableBuildTimer != -1
		&& Change(m_bDisableBuildTimer, iDisableBuildTimer == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (bApplyDisplayOptions)
		ApplyDisplayOptions();
}

std::optional<CHudBuildable::BuildableDisplayOption> CHudBuildable::GetBuildableDisplayOption(
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
		? std::optional<BuildableDisplayOption>()
		: static_cast<BuildableDisplayOption>(value);
}

void CHudBuildable::ApplyDisplayOptions()
{
	switch (m_showPanel)
	{
	case ALWAYS:
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER);
		break;
	case IF_BUILT:
		SetHiddenBits(m_bBuilt
			? HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER
			: HIDEHUD_ALWAYS);
		break;
	case ON_BUILD:
		SetHiddenBits(
			m_bBuilding || m_bBuilt
			? HIDEHUD_PLAYERDEAD | HIDEHUD_NOTENGINEER
			: HIDEHUD_ALWAYS);
		break;
	case NEVER:
	default:
		SetHiddenBits(HIDEHUD_ALWAYS);
		break;
	}

	switch (m_hideText)
	{
	case ALWAYS:
		SetUseToggleText(false);
		break;
	case IF_BUILT:
		SetUseToggleText(true);
		SetToggleTextVisible(!m_bBuilt);
		break;
	case ON_BUILD:
		SetUseToggleText(true);
		SetToggleTextVisible(!m_bBuilding && !m_bBuilt);
		break;
	case NEVER:
	default:
		SetUseToggleText(true);
		SetToggleTextVisible(true);
		break;
	}

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
		if (m_bBuilt)
			DisableItem(m_qiCells);
		else
			EnableItem(m_qiCells);
		break;
	case ON_BUILD:
		if (m_bBuilt || m_bBuilding)
			DisableItem(m_qiCells);
		else
			EnableItem(m_qiCells);
		break;
	}

	if (!m_bBuilding)
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
	usermessages->HookMessage("FF_BuildTimer", __MsgFunc_FF_BuildTimer);

	AddPanelToHudOptions(
		FFQuantityPanel::GetName(),
		m_szDisplayNameKey,
		"Buildables",
		"#HudPanel_Buildables");
}

void CHudBuildable::InitItemHealth()
{
	m_qiHealth = AddItem("Health");
	m_qiHealth->SetLabel("#FF_ITEM_HEALTH");
	m_qiHealth->SetIcon('a');
	m_qiHealth->SetAmountDisplay(DISPLAY_PERCENTAGE);
	m_qiHealth->SetAmountMax(100);
}

void CHudBuildable::InitItemBuildProgress()
{
	m_qiBuildProgress = AddItem("BuildProgress");
	m_qiBuildProgress->SetLabel("#FF_ITEM_PROGRESS");
	m_qiBuildProgress->SetIcon('f');
	m_qiBuildProgress->SetAmountDisplay(DISPLAY_PERCENTAGE);
}

void CHudBuildable::InitItemCells()
{
	m_qiCells = AddItem("Cells");
	m_qiCells->SetLabel("#FF_ITEM_CELLS");
	m_qiCells->SetIcon('p');
	m_qiCells->SetAmountDisplay(DISPLAY_MAX);
	m_qiCells->SetAmountMax(GetMaxCells());
}

void CHudBuildable::VidInit()
{
	m_bBuilt = false;
	m_bBuilding = false;

	SetText(m_wsNotBuilt);

	m_qiHealth->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);
	m_qiCells->SetAmount(0);

	HideItem(m_qiHealth);
	DisableItem(m_qiBuildProgress);

	ApplyDisplayOptions();
}

void CHudBuildable::OnTick()
{
	FFQuantityPanel::OnTick();

	if (!engine->IsInGame())
		return;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (pPlayer->GetClassSlot() != CLASS_ENGINEER)
		return;

	CFFBuildableObject* buildable = GetBuildable();
	bool bBuilt = buildable && buildable->IsBuilt();
	bool bBuilding = buildable && !bBuilt;

	// Update building state
	if (bBuilding != m_bBuilding)
	{
		m_bBuilding = bBuilding;

		ApplyDisplayOptions();
	}

	// Update built state
	if (bBuilt != m_bBuilt)
	{
		m_bBuilt = bBuilt;

		OnBuiltChanged(m_bBuilt);

		ApplyDisplayOptions();
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
		m_qiBuildProgress->SetAmount(flTimeElapsed);

	FFQuantityPanel::Paint();
}

void CHudBuildable::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != m_iBuildType)
		return;

	float flBuildDuration = msg.ReadFloat();

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	if (!m_bDisableBuildTimer)
	{
		EnableItem(m_qiBuildProgress);
		DisableItem(m_qiHealth);
	}
}

void CHudBuildable::OnBuiltChanged(
	bool bBuilt)
{
	if (bBuilt)
		ShowItem(m_qiHealth);
	else
		HideItem(m_qiHealth);
}

