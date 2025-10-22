#include "cbase.h"

#include "ff_hud_timer_build.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudTimerBuild);
DECLARE_HUD_MESSAGE(CHudTimerBuild, FF_BuildTimer);

CHudTimerBuild::CHudTimerBuild(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudTimerBuild")
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(HIDEHUD_ALWAYS);
	SetHeaderIconChar('f');

	m_flDuration = 0.0f;
	m_flStartTime = 0.0f;
	m_bInvertScale = false;
	m_bShowPanel = true;

	m_iBuildType = FF_BUILD_NONE;

	m_qiTimer = nullptr;
}

CHudTimerBuild::~CHudTimerBuild()
{
}

void CHudTimerBuild::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	KeyValues* kvInvertScaleOptions = GetCountDirectionOptions();

	AddItemOption(
		kvPanelSpecificOptions,
		"Progress",
		"#FF_ITEM_PROGRESS",
		{
			{
				ITEM_FORMAT,
				"InvertScale",
				kvInvertScaleOptions,
				0
			}
		});
}

KeyValues* CHudTimerBuild::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= BaseClass::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("InvertScale", false);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	kvPreset->SetInt("showHeaderIcon", 0);
	kvPreset->SetInt("showHeaderText", 0);

	kvPreset->SetInt("barWidth", 100);
	kvPreset->SetInt("barHeight", 13);

	KeyValues* kvIcon = kvPreset->FindKey("Icon", true);
	kvIcon->SetInt("show", 1);
	kvIcon->SetInt("size", 5);
	kvIcon->SetInt("shadow", 0);
	kvIcon->SetInt("anchorPosition", ANCHORPOS_MIDDLELEFT);
	kvIcon->SetInt("alignH", ALIGN_RIGHT);
	kvIcon->SetInt("alignV", ALIGN_MIDDLE);
	kvIcon->SetInt("offsetX", -2);
	kvIcon->SetInt("offsetY", 0);

	kvPreset->SetInt("x", 320);
	kvPreset->SetInt("y", 400);
	kvPreset->SetInt("alignH", ALIGN_CENTER);
	kvPreset->SetInt("alignV", ALIGN_TOP);

	return kvPreset;
}

void CHudTimerBuild::ApplyStyleData(
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


	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	int iInvertScale
		= GetInt(
			"InvertScale",
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

	if (iInvertScale != -1)
	{
		m_bInvertScale = (iInvertScale == 1);

		m_qiTimer->SetLabel(
			m_bInvertScale
			? "#FF_ITEM_TIMELEFT"
			: "#FF_ITEM_PROGRESS");


		m_qiTimer->SetAmountFormat(
			m_bInvertScale
			? FORMAT_RAW
			: FORMAT_PERCENTAGE);
	}

	if (IsInPreviewMode())
	{
		SetHiddenBits(0);
	}
	else if (m_bShowPanel)
	{
		SetHiddenBits(DEFAULT_HIDDEN_BITS);
	}
}

void CHudTimerBuild::VidInit()
{
	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Building");
	SetHeaderText(localized ? localized : L"Building...");
}

void CHudTimerBuild::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);
	HOOK_HUD_MESSAGE(CHudTimerBuild, FF_BuildTimer);

	m_qiTimer = AddItem("HudTimerBuildTimeLeft");
	m_qiTimer->SetIcon('f');
	m_qiTimer->SetAmount(0);
	m_qiTimer->SetAmountDecimalPlaces(1);
	m_qiTimer->SetLabel("#FF_ITEM_PROGRESS");

	// This is the correct value but gets reset on build below
	m_qiTimer->SetAmountMax(3.5f);

	AddPanelToHudOptions(
		"Build",
		"#HudPanel_Build",
		"Timers",
		"#HudPanel_Timers");
}

void CHudTimerBuild::Paint()
{
	float flTimeElapsed
		= gpGlobals->curtime - m_flStartTime;

	if (m_flDuration > flTimeElapsed)
	{
		m_qiTimer->SetAmount(
			m_bInvertScale
			? (m_flDuration - flTimeElapsed)
			: flTimeElapsed);
	}
	else if (!IsInPreviewMode())
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}

	BaseClass::Paint();
}

void CHudTimerBuild::MsgFunc_FF_BuildTimer(
	bf_read& msg)
{
	int iBuildType = msg.ReadShort();
	float flDuration = msg.ReadFloat();

	SetBuildTimer(iBuildType, flDuration);
}

void CHudTimerBuild::SetBuildTimer(
	int iBuildType,
	float flDuration)
{
	if (m_iBuildType != iBuildType)
	{
		m_iBuildType = iBuildType;
		switch (iBuildType)
		{
		case FF_BUILD_NONE:
			if (!IsInPreviewMode())
				SetHiddenBits(HIDEHUD_ALWAYS);

			return;
		case FF_BUILD_DISPENSER:
			m_qiTimer->SetIcon('4');
			break;
		case FF_BUILD_SENTRYGUN:
			m_qiTimer->SetIcon('1');
			break;
		case FF_BUILD_DETPACK:
			m_qiTimer->SetIcon('5');
			break;
		case FF_BUILD_MANCANNON:
			m_qiTimer->SetIcon('6');
			break;
		default:
			m_qiTimer->SetIcon('f');
			break;
		}
	}

	if (!IsInPreviewMode())
	{
		SetHiddenBits(DEFAULT_HIDDEN_BITS);
	}

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = flDuration;

	m_qiTimer->SetAmountMax(m_flDuration);
	m_qiTimer->SetAmount(
		m_bInvertScale
		? 0
		: m_flDuration);
}
