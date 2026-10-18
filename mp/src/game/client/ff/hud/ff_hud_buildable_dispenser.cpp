#include "cbase.h"

using namespace vgui;

#include "ff_hud_buildable_dispenser.h"
#include "ff_buildable_dispenser.h"

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

	KeyValues* kvCells = nullptr;
	for (KeyValues* kvItem = kvPanelSpecificOptions->GetFirstTrueSubKey();
		kvItem;
		kvItem = kvItem->GetNextTrueSubKey())
	{
		if (Q_stricmp(kvItem->GetString("item"), "Cells") != 0)
			continue;

		kvCells = kvItem;
		kvPanelSpecificOptions->RemoveSubKey(kvCells);

		for (KeyValues* kvControl = kvCells->GetFirstTrueSubKey();
			kvControl;
			kvControl = kvControl->GetNextTrueSubKey())
		{
			if (kvControl->GetInt("column") != ITEM_VISIBILITY)
				continue;

			kvControl->SetInt("defaultValue", NEVER);

			KeyValues* kvVisibility = kvControl->FindKey("Values");
			kvVisibility->Clear();
			kvVisibility->SetString(
				std::to_string(IF_BUILT).c_str(),
				"#HudPanel_VisibilityIfBuilt");
			kvVisibility->SetString(
				std::to_string(NEVER).c_str(),
				"#HudPanel_VisibilityAlways");
		}

		break;
	}

	AddItemOption(
		kvPanelSpecificOptions,
		"Ammo",
		"#FF_ITEM_AMMO",
		{
			{
				ITEM_VISIBILITY,
				"HideAmmo",
				GetQuantityVisibilityOptions(),
				false
			},
			{
				ITEM_FORMAT,
				"AmmoFormat",
				CHudBuildable::s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	AddItemOption(
		kvPanelSpecificOptions,
		"Armor",
		"#FF_ITEM_ARMOR",
		{
			{
				ITEM_VISIBILITY,
				"HideArmor",
				GetQuantityVisibilityOptions(),
				true
			},
			{
				ITEM_FORMAT,
				"ArmorFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	if (kvCells)
		kvPanelSpecificOptions->AddSubKey(kvCells);

	AddItemOption(
		kvPanelSpecificOptions,
		"Nails",
		"#FF_ITEM_NAILS",
		{
			{
				ITEM_VISIBILITY,
				"HideNails",
				GetQuantityVisibilityOptions(),
				true
			},
			{
				ITEM_FORMAT,
				"NailsFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	AddItemOption(
		kvPanelSpecificOptions,
		"Shells",
		"#FF_ITEM_SHELLS",
		{
			{
				ITEM_VISIBILITY,
				"HideShells",
				GetQuantityVisibilityOptions(),
				true
			},
			{
				ITEM_FORMAT,
				"ShellsFormat",
				s_kvAmountFormatOptions->MakeCopy(),
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
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_OVER_MAX
			}
		});

	AddAlertOptions(kvPanelSpecificOptions, "#HudPanel_Option_DamageAlert", 0);
}

KeyValues* CHudBuildableDispenser::GetDefaultStyleData()
{
	KeyValues* kvPreset
		= CHudBuildable::GetDefaultStyleData();

	KeyValues* kvPanelSpecificValues
		= kvPreset->FindKey("PanelSpecificValues");

	kvPanelSpecificValues->SetInt("AmmoFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetBool("HideAmmo", false);
	kvPanelSpecificValues->SetInt("HideCells", NEVER);
	kvPanelSpecificValues->SetBool("HideArmor", true);
	kvPanelSpecificValues->SetInt("ArmorFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetBool("HideNails", true);
	kvPanelSpecificValues->SetInt("NailsFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetBool("HideShells", true);
	kvPanelSpecificValues->SetInt("ShellsFormat", FORMAT_OVER_MAX);
	kvPanelSpecificValues->SetBool("HideRockets", false);
	kvPanelSpecificValues->SetInt("RocketsFormat", FORMAT_OVER_MAX);

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

	std::optional<AmountFormat> cellsFormat
		= GetAmountFormat(
			"CellsFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	std::optional<AmountFormat> ammoFormat
		= GetAmountFormat(
			"AmmoFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (cellsFormat.has_value())
	{
		m_qiCells->SetAmountFormat(
			cellsFormat.value());
	}

	if (ammoFormat.has_value())
	{
		m_qiAmmo->SetAmountFormat(
			ammoFormat.value());
	}

	int iHideAmmo = GetInt("HideAmmo", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideAmmo != -1)
		m_bHideAmmo = iHideAmmo == 1;

	int iHideArmor = GetInt("HideArmor", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideArmor != -1)
		m_hideStoredItems[0] = iHideArmor == 1;

	std::optional<AmountFormat> armorFormat = GetAmountFormat("ArmorFormat", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (armorFormat.has_value())
		m_storedItems[0]->SetAmountFormat(armorFormat.value());

	int iHideNails = GetInt("HideNails", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideNails != -1)
		m_hideStoredItems[1] = iHideNails == 1;

	std::optional<AmountFormat> nailsFormat = GetAmountFormat("NailsFormat", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (nailsFormat.has_value())
		m_storedItems[1]->SetAmountFormat(nailsFormat.value());

	int iHideShells = GetInt("HideShells", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideShells != -1)
		m_hideStoredItems[2] = iHideShells == 1;

	std::optional<AmountFormat> shellsFormat = GetAmountFormat("ShellsFormat", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (shellsFormat.has_value())
		m_storedItems[2]->SetAmountFormat(shellsFormat.value());

	int iHideRockets = GetInt("HideRockets", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iHideRockets != -1)
		m_hideStoredItems[3] = iHideRockets == 1;

	std::optional<AmountFormat> rocketsFormat = GetAmountFormat("RocketsFormat", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (rocketsFormat.has_value())
		m_storedItems[3]->SetAmountFormat(rocketsFormat.value());

	ApplyVisibilityOptions();
}

void CHudBuildableDispenser::Init()
{
	HOOK_HUD_MESSAGE(CHudBuildableDispenser, DispenserMsg);

	InitItemHealth();
	InitItemBuildProgress();

	m_qiAmmo = AddItem("Ammo");
	m_qiAmmo->SetLabel("#FF_ITEM_AMMO");
	m_qiAmmo->SetIcon('r');
	m_qiAmmo->SetAmountFormat(FORMAT_PERCENTAGE);
	m_qiAmmo->SetAmountMax(100);

	InitItemCells();
	m_qiCells->SetIntensityLimits(
		0,
		(int)(FF_BUILDCOST_DISPENSER / 3),
		(int)(FF_BUILDCOST_DISPENSER / 3) * 2,
		FF_BUILDCOST_DISPENSER);
	m_qiCells->SetIntensityAmountScaled(false);

	m_storedItems[0] = AddItem("Armor");
	m_storedItems[0]->SetIcon('s');
	m_storedItems[0]->SetLabel("#FF_ITEM_ARMOR");
	m_storedItems[0]->SetAmountFormat(FORMAT_OVER_MAX);
	DisableItem(m_storedItems[0]);

	m_storedItems[1] = AddItem("Nails");
	m_storedItems[1]->SetLabel("#FF_ITEM_NAILS");
	m_storedItems[1]->SetIcon('t');
	m_storedItems[1]->SetAmountFormat(FORMAT_OVER_MAX);
	DisableItem(m_storedItems[1]);

	m_storedItems[2] = AddItem("Shells");
	m_storedItems[2]->SetIcon('r');
	m_storedItems[2]->SetLabel("#FF_ITEM_SHELLS");
	m_storedItems[2]->SetAmountFormat(FORMAT_OVER_MAX);
	DisableItem(m_storedItems[2]);

	m_storedItems[3] = AddItem("Rockets");
	m_storedItems[3]->SetIcon('i');
	m_storedItems[3]->SetLabel("#FF_ITEM_ROCKETS");
	m_storedItems[3]->SetAmountFormat(FORMAT_OVER_MAX);
	DisableItem(m_storedItems[3]);

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
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	C_FFDispenser* pDispenser = pPlayer ? pPlayer->GetDispenser() : nullptr;
	if (pDispenser && pDispenser->IsBuilt())
		return pDispenser->GetCells() + pDispenser->NeedsCells();

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

	m_iCellRequirement = bBuilt ? 0 : FF_BUILDCOST_DISPENSER;
	m_qiCells->SetAmountMax(GetMaxCells());
	m_qiCells->SetIntensityAmountScaled(bBuilt);
	if (bBuilt)
		m_qiCells->SetIntensityLimits(0, 33, 66, 100);
	else
		m_qiCells->SetIntensityLimits(
			0,
			FF_BUILDCOST_DISPENSER / 3,
			FF_BUILDCOST_DISPENSER * 2 / 3,
			FF_BUILDCOST_DISPENSER);
}

void CHudBuildableDispenser::MsgFunc_DispenserMsg(
	bf_read& msg)
{
	int iHealth = (int)msg.ReadByte();
	int iAmmo = (int)msg.ReadByte();

	SetHealth(iHealth);
	m_qiAmmo->SetAmount(iAmmo);
}

void CHudBuildableDispenser::ApplyVisibilityOptions()
{
	CHudBuildable::ApplyVisibilityOptions();

	const bool bShowStored = m_bBuilt || IsInPreviewMode();

	if (m_hideCells == NEVER || bShowStored)
		EnableItem(m_qiCells);
	else
		DisableItem(m_qiCells);

	if (bShowStored && !m_bHideAmmo)
		EnableItem(m_qiAmmo);
	else
		DisableItem(m_qiAmmo);

	for (int i = 0; i < 4; ++i)
	{
		if (bShowStored && !m_hideStoredItems[i])
			EnableItem(m_storedItems[i]);
		else
			DisableItem(m_storedItems[i]);
	}
}

void CHudBuildableDispenser::OnTick()
{
	m_qiCells->SetAmountMax(GetMaxCells());
	CHudBuildable::OnTick();
	if (IsInPreviewMode())
	{
		m_qiAmmo->SetAmount(60);
		m_qiCells->SetAmountMax(100);
		m_qiCells->SetAmount(60);
		for (int i = 0; i < 4; ++i)
		{
			m_storedItems[i]->SetAmountMax(100);
			m_storedItems[i]->SetAmount(60);
		}

		return;
	}

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer || pPlayer->GetClassSlot() != CLASS_ENGINEER)
		return;

	C_FFDispenser* pDispenser = pPlayer->GetDispenser();
	if (!pDispenser || !pDispenser->IsBuilt())
		return;

	m_qiAmmo->SetAmount(pDispenser->GetAmmoPercent());
	m_storedItems[0]->SetAmountMax(pDispenser->GetArmor() + pDispenser->NeedsArmor());
	m_storedItems[0]->SetAmount(pDispenser->GetArmor());
	m_qiCells->SetAmountMax(pDispenser->GetCells() + pDispenser->NeedsCells());
	m_qiCells->SetAmount(pDispenser->GetCells());
	m_storedItems[1]->SetAmountMax(pDispenser->GetNails() + pDispenser->NeedsNails());
	m_storedItems[1]->SetAmount(pDispenser->GetNails());
	m_storedItems[2]->SetAmountMax(pDispenser->GetShells() + pDispenser->NeedsShells());
	m_storedItems[2]->SetAmount(pDispenser->GetShells());
	m_storedItems[3]->SetAmountMax(pDispenser->GetRockets() + pDispenser->NeedsRockets());
	m_storedItems[3]->SetAmount(pDispenser->GetRockets());
}
