#include "cbase.h"

#include "ff_hud_buildable_dispenser.h"

using namespace FFQuantityHelper;

DECLARE_HUDELEMENT(CHudBuildableDispenser);
DECLARE_HUD_MESSAGE(CHudBuildableDispenser, DispenserMsg);

CHudBuildableDispenser::CHudBuildableDispenser(
	const char* pElementName)
	: CHudBuildable(
		pElementName,
		FF_BUILD_DISPENSER,
		"HudBuildableDispenser",
		"#HudPanel_Dispenser")
{
	m_iCellRequirement = FF_BUILDCOST_DISPENSER;
}

CHudBuildableDispenser::~CHudBuildableDispenser()
{
}

void CHudBuildableDispenser::AddPanelSpecificOptions(
	KeyValues* kvPanelSpecificOptions)
{
	CHudBuildable::AddPanelSpecificOptions(
		kvPanelSpecificOptions);

	/*
	AddBooleanOption(kvPanelSpecificOptions, "DamageAlert", "Damage Alert", true);
	AddBooleanOption(kvPanelSpecificOptions, "HealAlert", "Heal Alert", true);
	*/

	AddComboOption(
		kvPanelSpecificOptions,
		"AmmoDisplay",
		"#HudPanel_Option_AmmoDisplay",
		CHudBuildable::s_kvAmountDisplayOptions->MakeCopy(),
		DISPLAY_MAX,
		1);
}

KeyValues* CHudBuildableDispenser::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= CHudBuildable::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= kvPreset->FindKey("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("AmmoDisplay", DISPLAY_MAX);

	return kvPreset;
}

void CHudBuildableDispenser::ApplyStyleData(
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

	std::optional<AmountDisplay> cellsDisplay
		= GetAmountDisplay(
			"CellsDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<AmountDisplay> ammoDisplay
		= GetAmountDisplay(
			"AmmoDisplay",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (cellsDisplay.has_value())
	{
		m_qiCells->SetAmountDisplay(
			cellsDisplay.value());
	}

	if (ammoDisplay.has_value())
	{
		m_qiAmmo->SetAmountDisplay(
			ammoDisplay.value());
	}
}

void CHudBuildableDispenser::Init()
{
	HOOK_HUD_MESSAGE(CHudBuildableDispenser, DispenserMsg);

	InitItemHealth();
	InitItemBuildProgress();

	m_qiAmmo = AddItem("Ammo");
	m_qiAmmo->SetLabel("#FF_ITEM_AMMO");
	m_qiAmmo->SetIcon('r');
	m_qiAmmo->SetAmountDisplay(DISPLAY_PERCENTAGE);
	m_qiAmmo->SetAmountMax(100);

	InitItemCells();
	m_qiCells->SetIntensityLimits(
		0,
		(int)(FF_BUILDCOST_DISPENSER / 3),
		(int)(FF_BUILDCOST_DISPENSER / 3) * 2,
		FF_BUILDCOST_DISPENSER);
	m_qiCells->SetIntensityAmountScaled(false);

	CHudBuildable::Init();
}

void CHudBuildableDispenser::VidInit()
{
	CHudBuildable::VidInit();

	SetHeaderIconChar('4');

	m_qiAmmo->SetAmount(0);

	DisableItem(m_qiAmmo);
}

int CHudBuildableDispenser::GetMaxCells()
{
	return FF_BUILDCOST_DISPENSER;
}

CFFBuildableObject* CHudBuildableDispenser::GetBuildable()
{
	return C_FFPlayer::GetLocalFFPlayer()
		->GetDispenser();
}

void CHudBuildableDispenser::OnBuiltChanged(
	bool bBuilt)
{
	CHudBuildable::OnBuiltChanged(bBuilt);

	if (bBuilt)
	{
		EnableItem(m_qiAmmo);
		DisableItem(m_qiCells);
		m_iCellRequirement = 0;
	}
	else
	{
		DisableItem(m_qiAmmo);
		EnableItem(m_qiCells);
		m_iCellRequirement = FF_BUILDCOST_DISPENSER;
	}
}

void CHudBuildableDispenser::MsgFunc_DispenserMsg(
	bf_read& msg)
{
	int iHealth = (int)msg.ReadByte();
	int iAmmo = (int)msg.ReadByte();

	m_qiHealth->SetAmount(iHealth);
	m_qiAmmo->SetAmount(iAmmo);
}
