#include "cbase.h"

#include "ff_hud_timer_detpack.h"

DECLARE_HUDELEMENT(CHudTimerDetpack);
DECLARE_HUD_MESSAGE(CHudTimerDetpack, FF_BuildTimer);
DECLARE_HUD_MESSAGE(CHudTimerDetpack, DetpackMsg);

CHudTimerDetpack::CHudTimerDetpack(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudTimerDetpack"),
	m_wsArmed(L"Armed"),
	m_wsArming(L"Arming..."),
	m_wsAvailable(L"Available"),
	m_wsNotAvailable(L"Not Available")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NOTDEMOMAN);

	SetHeaderText(L"Detpack");
	SetHeaderIconChar('5');
	SetUseToggleText(true);
	SetText(m_wsNotAvailable);

	m_qiBuildProgress = nullptr;
	m_qiDetpackTimeLeft = nullptr;
}

CHudTimerDetpack::~CHudTimerDetpack()
{
}

void CHudTimerDetpack::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Detpack");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Armed");
	if (localized)
		m_wsArmed = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Arming");
	if (localized)
		m_wsArming = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Available");
	if (localized)
		m_wsAvailable = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_NotAvailable");
	if (localized)
		m_wsNotAvailable = localized;
}

void CHudTimerDetpack::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddBooleanOption(
		kvPanelSpecificOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		true,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideText",
		"#HudPanel_Option_HideText",
		false,
		0);

	AddBooleanOption(
		kvPanelSpecificOptions,
		"DisableBuildTimer",
		"#HudPanel_Option_HideBuildTimer",
		false,
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"ValueDisplay",
		"#HudPanel_Option_ValueDisplay",
		s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
}

KeyValues* CHudTimerDetpack::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
	kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_BOTTOM);

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("HideText", false);
	kvPanelSpecificValues->SetBool("DisableBuildTimer", false);

	kvPanelSpecificValues->SetInt("ValueDisplay", DISPLAY_MAX);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudTimerDetpack::ApplyStyleData(
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

	std::optional<AmountDisplay> valueDisplay
		= GetAmountDisplay(
			"ValueDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (valueDisplay.has_value())
	{
		m_qiDetpackTimeLeft->SetAmountDisplay(
			valueDisplay.value());
	}

	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iHideText
		= GetInt(
			"HideText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iDisableBuildTimer
		= GetInt(
			"DisableBuildTimer",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	bool bApplyDisplayOptions = false;

	if (iShowPanel != -1
		&& Change(m_bShowPanel, iShowPanel == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (iHideText != -1
		&& Change(m_bHideText, iHideText == 1))
	{
		bApplyDisplayOptions = true;
	}

	if (iDisableBuildTimer != -1)
	{
		m_bDisableBuildTimer = iDisableBuildTimer == 1;
	}

	if (bApplyDisplayOptions)
		ApplyDisplayOptions();
}

void CHudTimerDetpack::ApplyDisplayOptions()
{
	if (m_bShowPanel)
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NOTDEMOMAN);
	else
		SetHiddenBits(HIDEHUD_ALWAYS);

	if (m_bHideText)
		SetUseToggleText(false);
	else
		SetUseToggleText(true);

	if (!m_bArming)
	{
		DisableItem(m_qiBuildProgress);
		EnableItem(m_qiDetpackTimeLeft);
	}
	else if (!m_bDisableBuildTimer)
	{
		EnableItem(m_qiBuildProgress);
		DisableItem(m_qiDetpackTimeLeft);
	}
}

void CHudTimerDetpack::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);
	HOOK_HUD_MESSAGE(CHudTimerDetpack, FF_BuildTimer);
	HOOK_HUD_MESSAGE(CHudTimerDetpack, DetpackMsg);

	m_qiBuildProgress = AddItem("BuildProgress");
	m_qiBuildProgress->SetLabel("#FF_ITEM_PROGRESS");
	m_qiBuildProgress->SetIcon('f');
	m_qiBuildProgress->SetAmountDisplay(DISPLAY_PERCENTAGE);

	m_qiDetpackTimeLeft = AddItem("TimeLeft");
	m_qiDetpackTimeLeft->SetLabel("#FF_ITEM_TIMELEFT");
	m_qiDetpackTimeLeft->SetIcon('f');

	AddPanelToHudOptions(
		"Detpack",
		"#HudPanel_Detpack",
		"Timers",
		"#HudPanel_Timers");
}

void CHudTimerDetpack::VidInit()
{
	m_bArmed = false;
	m_bArming = false;

	SetText(m_wsNotAvailable);

	m_qiDetpackTimeLeft->SetAmount(0);
	m_qiBuildProgress->SetAmount(0);

	HideItem(m_qiDetpackTimeLeft);
	DisableItem(m_qiBuildProgress);

	ApplyDisplayOptions();
}

void CHudTimerDetpack::OnTick()
{
	BaseClass::OnTick();

	if (!engine->IsInGame() || !ShouldDraw())
		return;

	C_FFPlayer* pPlayer
		= C_FFPlayer::GetLocalFFPlayer();

	bool bArmed = pPlayer->GetDetpack() && pPlayer->GetDetpack()->IsBuilt();
	bool bArming = pPlayer->GetDetpack() && !bArmed;

	// Update arming state
	if (bArming != m_bArming)
	{
		m_bArming = bArming;

		// The true value is handled in the
		// build timer for responsive updates

		if (!m_bArming)
			// We assume the player cancelled,
			// which is overridden below if armed.
			SetText(m_wsAvailable);

		ApplyDisplayOptions();
	}

	// Update armed state
	if (bArmed != m_bArmed)
	{
		m_bArmed = bArmed;

		if (m_bArmed)
		{
			SetText(m_wsArmed);
			ShowItem(m_qiDetpackTimeLeft);
		}
		else
		{
			// Exploded or defused
			SetText(m_wsAvailable);
			HideItem(m_qiDetpackTimeLeft);
		}

		ApplyDisplayOptions();
	}

	if (!m_bArming && !m_bArmed)
	{
		// Not armed or arming, show if we have one
		if (pPlayer->GetAmmoCount(AMMO_DETPACK) > 0)
			SetText(m_wsAvailable);
		else
			SetText(m_wsNotAvailable);
	}
}

void CHudTimerDetpack::Paint()
{
	if(m_bArming)
	{
		float flTimeElapsed
			= gpGlobals->curtime - m_flBuildStartTime;

		if (m_flBuildDuration > flTimeElapsed)
			m_qiBuildProgress->SetAmount(flTimeElapsed);
	}

	if (m_bArmed)
	{
		float flCurTime = gpGlobals->curtime;

		float flDetpackTimeLeft
			= m_flDetonateTime - flCurTime;

		// Prevent showing negative zero time left
		flDetpackTimeLeft
			= max(flDetpackTimeLeft, 0.0f);

		m_qiDetpackTimeLeft->SetAmount(flDetpackTimeLeft);
	}

	BaseClass::Paint();
}

void CHudTimerDetpack::MsgFunc_DetpackMsg(
	bf_read& msg)
{
	m_flDetonateTime = msg.ReadFloat();
	m_qiDetpackTimeLeft->SetAmountMax(msg.ReadByte());
}

void CHudTimerDetpack::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();

	if (iBuildType != FF_BUILD_DETPACK)
		return;

	m_bArming = true;
	SetText(m_wsArming);

	float flBuildDuration = msg.ReadFloat();

	m_flBuildDuration = flBuildDuration;
	m_flBuildStartTime = gpGlobals->curtime;

	m_qiBuildProgress->SetAmount(0.0f);
	m_qiBuildProgress->SetAmountMax(flBuildDuration);

	ApplyDisplayOptions();
}