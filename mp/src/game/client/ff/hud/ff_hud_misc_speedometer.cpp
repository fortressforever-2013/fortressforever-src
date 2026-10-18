#include "cbase.h"

#include "ff_hud_misc_speedometer.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudMiscSpeedometer);

CHudMiscSpeedometer::CHudMiscSpeedometer(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudMiscSpeedometer"),
	m_bShowPanel(true),
	m_speedItems(AVERAGE_ONLY)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);

	SetHeaderIconChar('v');
	SetHeaderText(L"Speedometer");

	m_qiInstantaneousSpeed = nullptr;
	m_qiAverageSpeed = nullptr;
}

CHudMiscSpeedometer::~CHudMiscSpeedometer()
{
}

void CHudMiscSpeedometer::ApplySchemeSettings(
	IScheme* pScheme)
{
	FFQuantityPanel::ApplySchemeSettings(pScheme);

	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Speedometer");
	if (localized)
		SetHeaderText(localized);
}

void CHudMiscSpeedometer::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	KeyValues* kvItems = new KeyValues("Values");
	kvItems->SetString(std::to_string(SPEED_ONLY).c_str(), "#FF_ITEM_SPEED");
	kvItems->SetString(std::to_string(AVERAGE_ONLY).c_str(), "#FF_ITEM_AVERAGE");
	kvItems->SetString(std::to_string(BOTH).c_str(), "#HudPanel_Option_Both");

	AddComboOption(
		kvPanelSpecificOptions,
		"SpeedItems",
		"#HudPanel_Option_Items",
		kvItems,
		AVERAGE_ONLY,
		0);

	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_Average", 1);

	AddNumericOption(
		kvPanelSpecificOptions,
		"AveragePeriod",
		"#HudPanel_Option_AveragePeriod",
		1000,
		100,
		5000,
		1);


	AddNumericOption(
		kvPanelSpecificOptions,
		"AverageResponsiveness",
		"#HudPanel_Option_AverageResponsiveness",
		80,
		1,
		99,
		1);

	AddItemOption(
		kvPanelSpecificOptions,
		"InstantaneousSpeed",
		"#FF_ITEM_SPEED",
		{
			{
				ITEM_FORMAT,
				"InstantaneousSpeedFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_RAW
			}
		});

	AddItemOption(
		kvPanelSpecificOptions,
		"AverageSpeed",
		"#FF_ITEM_AVERAGE",
		{
			{
				ITEM_FORMAT,
				"AverageSpeedFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_RAW
			}
		});
}

KeyValues* CHudMiscSpeedometer::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= FFQuantityPanel::GetDefaultStyleData();

	kvPreset->SetInt("x", 638);
	kvPreset->SetInt("y", 424);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_BOTTOM);
	kvPreset->SetInt("showHeaderText", 0);
	kvPreset->SetInt("showHeaderIcon", 0);
	kvPreset->SetInt("showPanel", 0);
	kvPreset->SetInt("showPanelBorder", 0);
	kvPreset->SetInt("barWidth", 0);
	kvPreset->SetInt("barHeight", 0);

	kvPreset->FindKey("Bar")->SetInt("show", 0);
	kvPreset->FindKey("BarBorder")->SetInt("show", 0);
	kvPreset->FindKey("BarBackground")->SetInt("show", 0);
	kvPreset->FindKey("Label")->SetInt("show", 0);
	kvPreset->FindKey("Icon")->SetInt("show", 0);
	kvPreset->FindKey("Amount")->SetInt("show", 1);
	kvPreset->FindKey("Amount")->SetInt("size", 20);
	kvPreset->FindKey("Amount")->SetInt("colorMode", COLOR_MODE_FADED);

	KeyValues* kvPanelSpecificValues
		= new KeyValues("PanelSpecificValues");

	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetInt("AveragePeriod", 1000);
	kvPanelSpecificValues->SetInt("AverageResponsiveness", 80);
	kvPanelSpecificValues->SetInt("SpeedItems", AVERAGE_ONLY);

	kvPanelSpecificValues->SetInt("InstantaneousSpeedFormat", FORMAT_RAW);
	kvPanelSpecificValues->SetInt("AverageSpeedFormat", FORMAT_RAW);

	kvPreset->AddSubKey(kvPanelSpecificValues);

	return kvPreset;
}

void CHudMiscSpeedometer::ApplyStyleData(
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

	int iPeriod
		= GetInt(
			"AveragePeriod",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iPeriod != -1)
		m_flAveragePeriod = clamp(iPeriod, 100, 5000) / 1000.0f;

	int iResponsiveness
		= GetInt(
			"AverageResponsiveness",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iResponsiveness != -1)
		m_flAverageResponsiveness = clamp(iResponsiveness, 1, 99) / 100.0f;

	std::optional<AmountFormat> instantaneousSpeedFormat
		= GetAmountFormat(
			"InstantaneousSpeedFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<AmountFormat> averageSpeedFormat
		= GetAmountFormat(
			"AverageSpeedFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (instantaneousSpeedFormat.has_value())
	{
		m_qiInstantaneousSpeed->SetAmountFormat(
			instantaneousSpeedFormat.value());
	}

	if (averageSpeedFormat.has_value())
	{
		m_qiAverageSpeed->SetAmountFormat(
			averageSpeedFormat.value());
	}

	int iShowPanel
		= GetInt(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	int iSpeedItems
		= GetInt(
			"SpeedItems",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	bool bApplyVisibilityOptions = false;

	if (iShowPanel != -1
		&& Change(m_bShowPanel, iShowPanel == 1))
	{
		bApplyVisibilityOptions = true;
	}

	if (iSpeedItems != -1
		&& Change(m_speedItems, static_cast<SpeedItems>(iSpeedItems)))
	{
		bApplyVisibilityOptions = true;
	}

	if (bApplyVisibilityOptions || kvStyleData->GetInt("previewMode", -1) != -1)
		ApplyVisibilityOptions();
}


void CHudMiscSpeedometer::ApplyVisibilityOptions()
{
	if (m_bShowPanel)
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

	if (m_speedItems == AVERAGE_ONLY)
		DisableItem(m_qiInstantaneousSpeed);
	else
		EnableItem(m_qiInstantaneousSpeed);

	if (m_speedItems == SPEED_ONLY)
		DisableItem(m_qiAverageSpeed);
	else
		EnableItem(m_qiAverageSpeed);
}

void CHudMiscSpeedometer::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	m_qiInstantaneousSpeed = AddItem("InstantaneousSpeed");
	m_qiInstantaneousSpeed->SetLabel("#FF_ITEM_SPEED");
	m_qiInstantaneousSpeed->SetIcon('a');
	m_qiInstantaneousSpeed->SetAmountFormat(FORMAT_RAW);
	m_qiInstantaneousSpeed->SetAmountMax(600);
	m_qiInstantaneousSpeed->SetAmountOverMax(9999);
	m_qiInstantaneousSpeed->SetIntensityAmountScaled(false);

	m_qiAverageSpeed = AddItem("AverageSpeed");
	m_qiAverageSpeed->SetLabel("#FF_ITEM_AVERAGE");
	m_qiAverageSpeed->SetIcon('f');
	m_qiAverageSpeed->SetAmountFormat(FORMAT_RAW);
	m_qiAverageSpeed->SetAmountMax(600);
	m_qiAverageSpeed->SetAmountOverMax(9999);
	m_qiAverageSpeed->SetIntensityAmountScaled(false);

	AddPanelToHudOptions(
		"Speedometer",
		"#HudPanel_Speedometer",
		"",
		"");
}

void CHudMiscSpeedometer::VidInit()
{
	m_flAverageSpeed = 0.0f;
	m_flNextUpdate = 0.0f;

	m_qiInstantaneousSpeed->SetAmount(0);
	m_qiAverageSpeed->SetAmount(0);

	ApplyVisibilityOptions();
}

void CHudMiscSpeedometer::OnThink()
{
	BaseClass::OnThink();

	if (!engine->IsInGame() || IsInPreviewMode())
		return;

	C_FFPlayer* pPlayer
		= C_FFPlayer::GetLocalFFPlayerOrAnyObserverTarget();

	if (!pPlayer)
		return;

	if (gpGlobals->curtime < m_flNextUpdate) {
		return;
	}

	const float flClassSpeed = pPlayer->MaxSpeed();
	if (flClassSpeed > 0.0f && Change(m_flClassSpeed, flClassSpeed))
	{
		const float flHardCap = flClassSpeed * BHOP_CAP_HARD;
		const int iRunSpeed = RoundFloatToInt(flClassSpeed);
		const int iSoftCap = RoundFloatToInt(flClassSpeed * BHOP_CAP_SOFT);
		const int iHardCap = RoundFloatToInt(flHardCap);
		const int iOrangeSpeed = (iRunSpeed + iSoftCap) / 2;

		m_qiInstantaneousSpeed->SetAmountMax(flHardCap);
		m_qiInstantaneousSpeed->SetIntensityLimits(iRunSpeed, iOrangeSpeed, iSoftCap, iHardCap);

		m_qiAverageSpeed->SetAmountMax(flHardCap);
		m_qiAverageSpeed->SetIntensityLimits(iRunSpeed, iOrangeSpeed, iSoftCap, iHardCap);
	}

	Vector vecVelocity
		= pPlayer->GetAbsVelocity();

	float flSpeed
		= FastSqrt(
			vecVelocity.x * vecVelocity.x
			+ vecVelocity.y * vecVelocity.y);

	m_qiInstantaneousSpeed->SetAmount(round(flSpeed));

	const float flUpdateInterval = 0.1f;
	const float flAlpha
		= AlphaForTarget(
			m_flAverageResponsiveness,
			m_flAveragePeriod,
			flUpdateInterval);

	m_flAverageSpeed
		= flAlpha * flSpeed
		+ (1.0f - flAlpha) * m_flAverageSpeed;

	if (flSpeed < 0.1f && m_flAverageSpeed < 0.5f)
	{
		m_flAverageSpeed = 0.0f;
	}

	m_flNextUpdate = gpGlobals->curtime + flUpdateInterval;

	m_qiAverageSpeed->SetAmount(m_flAverageSpeed);
}

// Close the requested fraction of the speed gap over the selected period.
float CHudMiscSpeedometer::AlphaForTarget(
	float flResponsiveness,
	float flPeriod,
	float flUpdateInterval)
{
	if (flResponsiveness <= 0.0f)
		return 0.0f;

	if (flResponsiveness >= 1.0f || flPeriod <= 0.0f)
		return 1.0f;

	if (flUpdateInterval <= 0.0f)
		return 0.0f;

	return 1.0f - powf(1.0f - flResponsiveness, flUpdateInterval / flPeriod);
}
