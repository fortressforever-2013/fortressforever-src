#include "cbase.h"

#include "ff_hud_timer_overpressure.h"
#include "c_ff_player.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudTimerOverpressure);

CHudTimerOverpressure::CHudTimerOverpressure(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudTimerOverpressure")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(DEFAULT_HIDDEN_BITS);
	SetHeaderText(L"Overpressure");
	SetHeaderIconChar('g');
	SetUseToggleText(true);
	SetText(m_wsReady);

	m_qiTimer = nullptr;
}

CHudTimerOverpressure::~CHudTimerOverpressure()
{
}

void CHudTimerOverpressure::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Overpressure");
	if (localized)
		SetHeaderText(localized);

	localized = g_pVGuiLocalize->Find("#HudPanel_Ready");
	if (localized)
		m_wsReady = localized;

	localized = g_pVGuiLocalize->Find("#HudPanel_Regenerating");
	if (localized)
		m_wsRegenerating = localized;
}

void CHudTimerOverpressure::AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions)
{
	KeyValues* kvProgressVisibility = new KeyValues("Values");
	kvProgressVisibility->SetString("2", "#HudPanel_VisibilityAlways");
	kvProgressVisibility->SetString("1", "#HudPanel_VisibilityOnCooldown");
	kvProgressVisibility->SetString("0", "#HudPanel_VisibilityNever");

	KeyValues* kvVisibility = new KeyValues("Values");
	kvVisibility->SetString("0", "#HudPanel_VisibilityNever");
	kvVisibility->SetString("1", "#HudPanel_VisibilityOnCooldown");
	kvVisibility->SetString("2", "#HudPanel_VisibilityAlways");

	AddComboOption(
		kvPanelSpecificOptions,
		"Show",
		"#GameUI_Show",
		kvVisibility,
		static_cast<int>(CooldownVisibility::ALWAYS),
		0);

	AddShowTextOption(kvPanelSpecificOptions);

	KeyValues* kvMode = GetCountDirectionOptions();

	AddItemOption(
		kvPanelSpecificOptions,
		"Progress",
		"#FF_ITEM_PROGRESS",
		{
			{
				ITEM_VISIBILITY,
				"ShowProgress",
				kvProgressVisibility,
				static_cast<int>(CooldownVisibility::ALWAYS)
			},
			{
				ITEM_FORMAT,
				"CountDown",
				kvMode,
				false
			}
		});
}

KeyValues* CHudTimerOverpressure::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	KeyValues* kvPanelSpecificValues = new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("Show", static_cast<int>(CooldownVisibility::ALWAYS));
	kvPanelSpecificValues->SetInt("ShowProgress", static_cast<int>(CooldownVisibility::ALWAYS));
	kvPanelSpecificValues->SetBool("ShowText", true);
	kvPanelSpecificValues->SetBool("CountDown", false);
	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudTimerOverpressure::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	BaseClass::ApplyStyleData(kvStyleData, kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);
	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	int iShowProgress = GetInt("ShowProgress", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iShowProgress != -1)
		m_showProgress = static_cast<CooldownVisibility>(iShowProgress);

	int iShow = GetInt("Show", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iShow != -1)
		m_show = static_cast<CooldownVisibility>(iShow);

	int iShowText
		= GetInt(
			"ShowText",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iShowText != -1)
		m_bShowText = iShowText == 1;

	int iCountDown = GetInt("CountDown", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iCountDown != -1)
		m_bCountDown = iCountDown == 1;

	m_qiTimer->SetAmountFormat(m_bCountDown ? FORMAT_RAW : FORMAT_PERCENTAGE);
	m_qiTimer->SetAmountDecimalPlaces(m_bCountDown ? 1 : 0);
	m_qiTimer->SetLabel(m_bCountDown ? "#FF_ITEM_TIMELEFT" : "#FF_ITEM_PROGRESS");

	ApplyVisibilityOptions();
}

void CHudTimerOverpressure::ApplyVisibilityOptions()
{
	UpdateProgressVisibility();
	SetUseToggleText(m_bShowText);

	if (m_show != CooldownVisibility::NEVER)
	{
		if (IsInPreviewMode())
			SetHiddenBits(0);
		else
			SetHiddenBits(DEFAULT_HIDDEN_BITS);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}
}

void CHudTimerOverpressure::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);

	m_qiTimer = AddItem("Cooldown");
	m_qiTimer->SetLabel("#FF_ITEM_PROGRESS");
	m_qiTimer->SetIcon('k');
	m_qiTimer->SetAmountMax(OVERPRESSURE_DELAY);
	m_qiTimer->SetAmountFormat(FORMAT_PERCENTAGE);

	AddPanelToHudOptions(
		"Overpressure",
		"#HudPanel_Overpressure",
		"Cooldown",
		"#HudPanel_Cooldown");
}

void CHudTimerOverpressure::VidInit()
{
	m_qiTimer->SetAmount(m_bCountDown ? 0 : OVERPRESSURE_DELAY);
	SetText(m_wsReady);
	ApplyVisibilityOptions();
}

bool CHudTimerOverpressure::ShouldDraw()
{
	if (m_show == CooldownVisibility::NEVER || !CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (!engine->IsInGame())
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer)
		return false;

	if (m_show == CooldownVisibility::ON_COOLDOWN
		&& pPlayer->m_flNextClassSpecificSkill <= gpGlobals->curtime)
		return false;

	return pPlayer->GetClassSlot() == CLASS_HWGUY
		&& !FF_IsPlayerSpec(pPlayer)
		&& FF_HasPlayerPickedClass(pPlayer);
}

void CHudTimerOverpressure::Paint()
{
	if (!IsInPreviewMode())
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		float flTimeLeft = pPlayer->m_flNextClassSpecificSkill - gpGlobals->curtime;
		if (flTimeLeft > 0)
		{
			m_qiTimer->SetAmount(m_bCountDown ? flTimeLeft : OVERPRESSURE_DELAY - flTimeLeft);
			SetText(m_wsRegenerating);
		}
		else
		{
			m_qiTimer->SetAmount(m_bCountDown ? 0 : OVERPRESSURE_DELAY);
			SetText(m_wsReady);
		}
	}

	BaseClass::Paint();
}

void CHudTimerOverpressure::UpdateProgressVisibility()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	const bool bCooling = IsInPreviewMode()
		|| (pPlayer && pPlayer->m_flNextClassSpecificSkill > gpGlobals->curtime);

	if (m_showProgress == CooldownVisibility::ALWAYS
		|| (m_showProgress == CooldownVisibility::ON_COOLDOWN && bCooling))
		EnableItem(m_qiTimer);
	else
		DisableItem(m_qiTimer);
}

void CHudTimerOverpressure::OnTick()
{
	UpdateProgressVisibility();
	BaseClass::OnTick();
}
