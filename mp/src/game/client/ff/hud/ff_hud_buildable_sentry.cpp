#include "cbase.h"

#include "ff_hud_buildable_sentry.h"

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
	m_bHideLevel = IF_BUILT;
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

	/*
	AddBooleanOption(kvPanelSpecificOptions, "DamageAlert", "Damage Alert", true);
	AddBooleanOption(kvPanelSpecificOptions, "HealAlert", "Heal Alert", true);
	*/

	AddBooleanOption(
		kvPanelSpecificOptions,
		"HideLevel",
		"#HudPanel_Option_HideLevel",
		CHudBuildable::s_kvVisibilityDisplayOptions->MakeCopy(),
		0);

	AddComboOption(
		kvPanelSpecificOptions,
		"LevelDisplay",
		"#HudPanel_Option_LevelDisplay",
		CHudBuildable::s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"ShellsDisplay",
		"#HudPanel_Option_ShellsDisplay",
		CHudBuildable::s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
	AddComboOption(
		kvPanelSpecificOptions,
		"RocketsDisplay",
		"#HudPanel_Option_RocketsDisplay",
		CHudBuildable::s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
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

	kvPanelSpecificValues->SetBool("HideLevel", false);

	kvPanelSpecificValues->SetInt("LevelDisplay", DISPLAY_MAX);

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

	std::optional<AmountDisplay> levelDisplay
		= GetAmountDisplay(
			"LevelDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountDisplay> shellsDisplay
		= GetAmountDisplay(
			"ShellsDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);
	std::optional<AmountDisplay> rocketsDisplay
		= GetAmountDisplay(
			"RocketsDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (levelDisplay.has_value())
	{
		m_qiLevel->SetAmountDisplay(
			levelDisplay.value());
	}

	if (shellsDisplay.has_value())
	{
		m_qiShells->SetAmountDisplay(
			shellsDisplay.value());
	}

	if (rocketsDisplay.has_value())
	{
		m_qiRockets->SetAmountDisplay(
			rocketsDisplay.value());
	}

	int iHideLevel
		= GetInt(
			"HideLevel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (iHideLevel != -1
		&& Change(m_bHideLevel, iHideLevel == 1))
	{
		ApplyDisplayOptions();
	}
}

void CHudBuildableSentry::Init()
{
	HOOK_HUD_MESSAGE(CHudBuildableSentry, SentryLevelMsg);
	HOOK_HUD_MESSAGE(CHudBuildableSentry, SentryStatusMsg);

	InitItemHealth();
	InitItemBuildProgress();

	m_qiLevel = AddItem("Level");
	m_qiLevel->SetLabel("#FF_ITEM_LEVEL");
	m_qiLevel->SetIcon('d');
	m_qiLevel->SetAmountMax(3);
	m_qiLevel->SetIntensityLimits(1, 2, 2, 3);
	m_qiLevel->SetIntensityAmountScaled(false);
	m_qiLevel->SetAmountDisplay(DISPLAY_MAX);

	m_qiShells = AddItem("Shells");
	m_qiShells->SetLabel("#FF_ITEM_SHELLS");
	m_qiShells->SetIcon('r');
	m_qiShells->SetAmountMax(100); //TODO confirm this max is correct
	m_qiShells->SetAmountDisplay(DISPLAY_MAX);

	m_qiRockets = AddItem("Rockets");
	m_qiRockets->SetLabel("#FF_ITEM_ROCKETS");
	m_qiRockets->SetIcon('i');
	m_qiRockets->SetAmountMax(20);
	m_qiRockets->SetAmountDisplay(DISPLAY_MAX);

	InitItemCells();
	m_qiCells->SetIntensityLimits(0, (int)(FF_BUILDCOST_SENTRYGUN / 3), (int)(FF_BUILDCOST_SENTRYGUN / 3) * 2, FF_BUILDCOST_SENTRYGUN);
	m_qiCells->SetIntensityAmountScaled(false);

	CHudBuildable::Init();
}

void CHudBuildableSentry::VidInit()
{
	CHudBuildable::VidInit();

	SetHeaderIconChar('1');

	m_qiLevel->SetAmount(0);

	DisableItem(m_qiShells);
	DisableItem(m_qiRockets);
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

	if (bBuilt)
	{
		EnableItem(m_qiShells);
		DisableItem(m_qiRockets);
	}
	else
	{
		DisableItem(m_qiShells);
		DisableItem(m_qiRockets);
		DisableItem(m_qiLevel);
	}
}

void CHudBuildableSentry::MsgFunc_SentryLevelMsg(
	bf_read& msg)
{
	int iLevel = msg.ReadByte();

	// Level 0 would end message here
	if (!msg.IsEnd())
	{
		int iMaxHealth = msg.ReadByte(),
			iHealth = msg.ReadByte(),
			iMaxShells = msg.ReadByte(),
			iShells = msg.ReadByte();

		m_qiHealth->SetAmount(iHealth);
		m_qiHealth->SetAmountMax(iMaxHealth);

		m_qiShells->SetAmountMax(iMaxShells);
		m_qiShells->SetAmount(iShells);
	}

	// Level 1 & 2 would end message here
	if (!msg.IsEnd())
	{
		int iMaxRockets = msg.ReadByte(),
			iRockets = msg.ReadByte();

		m_qiRockets->SetAmountMax(iMaxRockets);
		m_qiRockets->SetAmount(iRockets);

		EnableItem(m_qiRockets);
	}
	else
	{
		DisableItem(m_qiRockets);
	}

	if (m_qiLevel->GetAmountAsInt() == iLevel)
	{
		return;
	}

	m_qiLevel->SetAmount(iLevel);
	ApplyDisplayOptions();

	switch (iLevel)
	{
	default:
	case 0:
		SetHeaderIconChar('1');
		//set new build values
		m_qiCells->SetIntensityLimits(0, (int)(FF_BUILDCOST_SENTRYGUN / 3), (int)(FF_BUILDCOST_SENTRYGUN / 3) * 2, FF_BUILDCOST_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_SENTRYGUN);
		m_iCellRequirement = FF_BUILDCOST_SENTRYGUN;
		break;
	case 1:
		SetHeaderIconChar('1');
		//set upgrade values
		m_qiCells->SetIntensityLimits(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3) * 2, FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iCellRequirement = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 2:
		SetHeaderIconChar('2');
		//set upgrade values
		m_qiCells->SetIntensityLimits(0, (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3), (int)(FF_BUILDCOST_UPGRADE_SENTRYGUN / 3) * 2, FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_qiCells->SetAmountMax(FF_BUILDCOST_UPGRADE_SENTRYGUN);
		m_iCellRequirement = FF_BUILDCOST_UPGRADE_SENTRYGUN;
		break;
	case 3:
		SetHeaderIconChar('3');
		//set new build values
		m_qiCells->SetIntensityLimits(0, (int)(FF_BUILDCOST_SENTRYGUN / 3), (int)(FF_BUILDCOST_SENTRYGUN / 3) * 2, FF_BUILDCOST_SENTRYGUN);
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

	m_qiHealth->SetAmount(iHealth);

	m_qiShells->SetAmount(iShells);

	if (!msg.IsEnd())
	{
		int iRockets = msg.ReadByte();

		m_qiRockets->SetAmount(iRockets);
	}
}

void CHudBuildableSentry::ApplyDisplayOptions()
{
	CHudBuildable::ApplyDisplayOptions();

	bool bShowCells
		= m_bBuilt
		&& m_qiLevel->GetAmountAsInt() == 3;

	if (bShowCells)
		DisableItem(m_qiCells);
	else
		EnableItem(m_qiCells);

	bool bShowLevel
		= !m_bHideLevel
		&& m_bBuilt
		&& m_qiLevel->GetAmountAsInt() < 3;

	if (bShowLevel)
		EnableItem(m_qiLevel);
	else
		DisableItem(m_qiLevel);
}