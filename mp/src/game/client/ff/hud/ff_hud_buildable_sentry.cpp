#include "cbase.h"

using namespace vgui;

#include "ff_hud_buildable_sentry.h"
#include "ff_buildable_sentrygun.h"

#include <vprof.h>

DECLARE_HUDELEMENT(CHudBuildableSentry);
DECLARE_HUD_MESSAGE(CHudBuildableSentry, SentryLevelMsg);
DECLARE_HUD_MESSAGE(CHudBuildableSentry, SentryStatusMsg);

CHudBuildableSentry::CHudBuildableSentry(
	const char* pElementName)
	: CHudBuildable(
		pElementName,
		FF_BUILD_SENTRYGUN,
		"HudBuildableSentry",
		"#HudPanel_SentryGun")
{
	m_hideLevel = IF_BUILT;
	m_iCellRequirement = FF_BUILDCOST_SENTRYGUN;
}

CHudBuildableSentry::~CHudBuildableSentry()
{
}

void CHudBuildableSentry::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	CHudBuildable::AddPanelSpecificOptions(
		kvPanelSpecificOptions);

	AddItemOption(
		kvPanelSpecificOptions,
		"Shells",
		"#FF_ITEM_SHELLS",
		{
			{
				ITEM_VISIBILITY,
				"HideShells",
				GetQuantityVisibilityOptions(),
				false
			},
			{
				ITEM_FORMAT,
				"ShellsFormat",
				CHudBuildable::s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	AddItemOption(
		kvPanelSpecificOptions,
		"Rockets",
		"#FF_ITEM_ROCKETS",
		{
			{
				ITEM_VISIBILITY,
				"HideRockets",
				GetQuantityVisibilityOptions(),
				false
			},
			{
				ITEM_FORMAT,
				"RocketsFormat",
				CHudBuildable::s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	KeyValues* kvLevelVisibility = GetQuantityVisibilityOptions();
	kvLevelVisibility->SetString(
		std::to_string(IF_BUILT).c_str(),
		"#HudPanel_VisibilityIfNotBuilt");

	AddItemOption(
		kvPanelSpecificOptions,
		"Level",
		"#FF_ITEM_LEVEL",
		{
			{
				ITEM_VISIBILITY,
				"HideLevel",
				kvLevelVisibility,
				IF_BUILT
			},
			{
				ITEM_FORMAT,
				"LevelFormat",
				CHudBuildable::s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	// Cells are the final quantity in the sentry panel.
	for (KeyValues* kvItem = kvPanelSpecificOptions->GetFirstTrueSubKey();
		kvItem;
		kvItem = kvItem->GetNextTrueSubKey())
	{
		if (Q_stricmp(kvItem->GetString("item"), "Cells") != 0)
			continue;

		kvPanelSpecificOptions->RemoveSubKey(kvItem);
		kvPanelSpecificOptions->AddSubKey(kvItem);
		break;
	}

	AddAlertOptions(kvPanelSpecificOptions, "#HudPanel_Option_DamageAlert", 0);
}

KeyValues* CHudBuildableSentry::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= CHudBuildable::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_BOTTOM);

	KeyValues* kvPanelSpecificValues
		= kvPreset->FindKey("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("HideLevel", IF_BUILT);
	kvPanelSpecificValues->SetBool("HideShells", false);
	kvPanelSpecificValues->SetBool("HideRockets", false);

	kvPanelSpecificValues->SetInt("LevelFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetInt("ShellsFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetInt("RocketsFormat", FORMAT_OVER_MAX);

	return kvPreset;
}

void CHudBuildableSentry::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	CHudBuildable::ApplyStyleData(
		kvStyleData,
		kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaultStyleData->FindKey("PanelSpecificValues", true);

	std::optional<AmountFormat> levelFormat
		= GetAmountFormat(
			"LevelFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountFormat> shellsFormat
		= GetAmountFormat(
			"ShellsFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountFormat> rocketsFormat
		= GetAmountFormat(
			"RocketsFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (levelFormat.has_value())
	{
		m_qiLevel->SetAmountFormat(
			levelFormat.value());
	}

	if (shellsFormat.has_value())
	{
		m_qiShells->SetAmountFormat(
			shellsFormat.value());
	}

	if (rocketsFormat.has_value())
	{
		m_qiRockets->SetAmountFormat(
			rocketsFormat.value());
	}

	int iHideShells = GetInt("HideShells", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideShells != -1)
		m_bHideShells = iHideShells == 1;

	int iHideRockets = GetInt("HideRockets", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideRockets != -1)
		m_bHideRockets = iHideRockets == 1;

	std::optional<BuildableVisibility> hideLevel
		= GetBuildableVisibility(
			"HideLevel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (hideLevel.has_value())
		m_hideLevel = hideLevel.value();

	ApplyVisibilityOptions();
}

void CHudBuildableSentry::Init()
{
	HOOK_HUD_MESSAGE(CHudBuildableSentry, SentryLevelMsg);
	HOOK_HUD_MESSAGE(CHudBuildableSentry, SentryStatusMsg);

	InitItemHealth();
	InitItemBuildProgress();

	m_qiShells = AddItem("Shells");
	m_qiShells->SetLabel("#FF_ITEM_SHELLS");
	m_qiShells->SetIcon('r');
	m_qiShells->SetAmountMax(SG_MAXSHELLS_LEVEL1);
	m_qiShells->SetAmountFormat(FORMAT_OVER_MAX);

	m_qiRockets = AddItem("Rockets");
	m_qiRockets->SetLabel("#FF_ITEM_ROCKETS");
	m_qiRockets->SetIcon('i');
	m_qiRockets->SetAmountMax(SG_MAXROCKETS_LEVEL3);
	m_qiRockets->SetAmountFormat(FORMAT_OVER_MAX);

	m_qiLevel = AddItem("Level");
	m_qiLevel->SetLabel("#FF_ITEM_LEVEL");
	m_qiLevel->SetIcon('d');
	m_qiLevel->SetAmountMax(3);
	m_qiLevel->SetIntensityLimits(1, 2, 2, 3);
	m_qiLevel->SetIntensityAmountScaled(false);
	m_qiLevel->SetAmountFormat(FORMAT_OVER_MAX);

	InitItemCells();
	m_qiCells->SetIntensityLimits(
		0,
		(int)(FF_BUILDCOST_SENTRYGUN / 3),
		(int)(FF_BUILDCOST_SENTRYGUN / 3) * 2,
		FF_BUILDCOST_SENTRYGUN);
	m_qiCells->SetIntensityAmountScaled(false);

	CHudBuildable::Init();
}

void CHudBuildableSentry::VidInit()
{
	CHudBuildable::VidInit();

	SetHeaderIconChar('1');

	m_qiLevel->SetAmount(0);

	ApplyVisibilityOptions();
}

int CHudBuildableSentry::GetMaxCells()
{
	int iMaxCells = 0;
	switch (m_qiLevel->GetAmountAsInt())
	{
	case 1:
		iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 2:
		iMaxCells = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	default:
		iMaxCells = FF_BUILDCOST_SENTRYGUN;
		break;
	}

	return iMaxCells;
}

CFFBuildableObject* CHudBuildableSentry::GetBuildable()
{
	return C_FFPlayer::GetLocalFFPlayer()
		->GetSentryGun();
}

void CHudBuildableSentry::OnBuiltChanged(
	bool bBuilt)
{
	CHudBuildable::OnBuiltChanged(bBuilt);

	ApplyVisibilityOptions();
}

void CHudBuildableSentry::MsgFunc_SentryLevelMsg(
	bf_read& msg)
{
	int iLevel = msg.ReadByte();

	// Level 0 would end message here
	if (msg.GetNumBitsLeft() > 0)
	{
		int iMaxHealth = msg.ReadByte(),
			iHealth = msg.ReadByte(),
			iMaxShells = msg.ReadByte(),
			iShells = msg.ReadByte();

		SetHealth(iHealth);
		m_qiHealth->SetAmountMax(iMaxHealth);

		m_qiShells->SetAmountMax(iMaxShells);
		m_qiShells->SetAmount(iShells);
	}

	// Level 1 & 2 would end message here
	if (msg.GetNumBitsLeft() > 0)
	{
		int iMaxRockets = msg.ReadByte(),
			iRockets = msg.ReadByte();

		m_qiRockets->SetAmountMax(iMaxRockets);
		m_qiRockets->SetAmount(iRockets);
	}

	if (m_qiLevel->GetAmountAsInt() == iLevel)
	{
		return;
	}

	m_qiLevel->SetAmount(iLevel);
	ApplyVisibilityOptions();

	switch (iLevel)
	{
	default:
	case 0:
		SetHeaderIconChar('1');
		//set new build values
		m_qiCells->SetIntensityLimits(
			0,
			(int)(FF_BUILDCOST_SENTRYGUN / 3),
			(int)(FF_BUILDCOST_SENTRYGUN / 3) * 2,
			FF_BUILDCOST_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iCellRequirement = FF_BUILDCOST_SENTRYGUN;
		break;
	case 1:
		SetHeaderIconChar('1');
		//set upgrade values
		m_qiCells->SetIntensityLimits(
			0,
			(int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3),
			(int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3) * 2,
			FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iCellRequirement = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 2:
		SetHeaderIconChar('2');
		//set upgrade values
		m_qiCells->SetIntensityLimits(
			0,
			(int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3),
			(int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3) * 2,
			FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iCellRequirement = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 3:
		SetHeaderIconChar('3');
		//set new build values
		m_qiCells->SetIntensityLimits(
			0,
			(int)(FF_BUILDCOST_SENTRYGUN / 3),
			(int)(FF_BUILDCOST_SENTRYGUN / 3) * 2,
			FF_BUILDCOST_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iCellRequirement = 0;
		break;
	}
}

void CHudBuildableSentry::MsgFunc_SentryStatusMsg(
	bf_read& msg)
{
	int iHealth = msg.ReadByte(),
		iShells = msg.ReadByte();

	SetHealth(iHealth);

	m_qiShells->SetAmount(iShells);

	if (msg.GetNumBitsLeft() > 0)
	{
		int iRockets = msg.ReadByte();

		m_qiRockets->SetAmount(iRockets);
	}
}

void CHudBuildableSentry::ApplyVisibilityOptions()
{
	CHudBuildable::ApplyVisibilityOptions();

	const bool bPreview = IsInPreviewMode();
	const bool bBuilt = bPreview || m_bBuilt;
	const int iLevel = bPreview ? 3 : m_qiLevel->GetAmountAsInt();

	SetHeaderIconChar('1' + clamp(iLevel, 1, 3) - 1);

	// Cells are still needed for upgrades until the sentry reaches level 3.
	if (m_hideCells == IF_BUILT && (!bBuilt || iLevel < 3))
		EnableItem(m_qiCells);

	if (bBuilt && iLevel == 3)
		DisableItem(m_qiCells);

	// Before Build uses the Cells row first; Health takes its place once built.
	if (m_hideCells == ON_BUILD && !bBuilt)
		DisableItem(m_qiHealth);

	if (bBuilt && (m_hideLevel == NEVER || (m_hideLevel == IF_BUILT && iLevel < 3)))
		EnableItem(m_qiLevel);
	else
		DisableItem(m_qiLevel);

	if (bBuilt && !m_bHideShells)
		EnableItem(m_qiShells);
	else
		DisableItem(m_qiShells);

	if (bBuilt && iLevel == 3 && !m_bHideRockets)
		EnableItem(m_qiRockets);
	else
		DisableItem(m_qiRockets);
}
