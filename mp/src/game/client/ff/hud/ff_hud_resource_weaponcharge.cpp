#include "cbase.h"
#include "ff_hud_resource_weaponcharge.h"
#include "hud_macros.h"
#include "c_ff_player.h"
#include "ff_weapon_assaultcannon.h"
#include "ff_weapon_sniperrifle.h"
#include "ff_weapon_jumpgun.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"
#include <vgui/IVGui.h>

using namespace vgui;
using namespace FFQuantityHelper;

DECLARE_HUDELEMENT(CHudResourceSniperCharge);
DECLARE_HUDELEMENT(CHudResourceAssaultCannonCharge);
// Jump Gun is currently disabled in the normal loadout.
// DECLARE_HUDELEMENT(CHudResourceJumpgunCharge);

CHudResourceSniperCharge::CHudResourceSniperCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_SNIPERRIFLE,
		"#HudPanel_SniperCharge",
		true)
{
}

CHudResourceAssaultCannonCharge::CHudResourceAssaultCannonCharge(const char* pElementName)
	: CHudResourceWeaponCharge(
		pElementName,
		FF_WEAPON_ASSAULTCANNON,
		"#HudPanel_AssaultCannonCharge",
		true)
{
}

// CHudResourceJumpgunCharge::CHudResourceJumpgunCharge(const char* pElementName)
// 	: CHudResourceWeaponCharge(
// 		pElementName,
// 		FF_WEAPON_JUMPGUN,
// 		"#HudPanel_JumpgunCharge",
// 		true)
// {
// }

CHudResourceWeaponCharge::CHudResourceWeaponCharge(
	const char* pElementName,
	int iWeaponID,
	const char* pszTitle,
	bool bShowPanel)
	: CHudElement(pElementName), BaseClass(NULL, pElementName),
	m_iWeaponID(iWeaponID),
	m_pszTitle(pszTitle),
	m_bDefaultShowPanel(bShowPanel),
	m_showPanel(bShowPanel ? WHEN_ACTIVE : NEVER)
{
	SetParent(g_pClientMode->GetViewport());

	SetHiddenBits(DEFAULT_HIDDEN_BITS);
}

void CHudResourceWeaponCharge::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 250);

	const char* szLabel
		= m_iWeaponID == FF_WEAPON_ASSAULTCANNON
			? "#HudPanel_WindUp"
			: "#HudPanel_Charge";

	m_pCharge = AddItem("Charge");
	m_pCharge->SetLabel(szLabel);
	m_pCharge->SetIcon('f');
	m_pCharge->SetAmountFormat(FORMAT_PERCENTAGE);

	switch (m_iWeaponID)
	{
	case FF_WEAPON_SNIPERRIFLE:
		m_pCharge->SetAmountMax(FF_SNIPER_MAXCHARGE);
		m_pCharge->SetIcon('l');
		SetHeaderIconChar('x');
		break;

	case FF_WEAPON_ASSAULTCANNON:
		m_pCharge->SetAmountMax(FF_AC_WINDUPTIME);
		m_pCharge->SetIcon('b');

		m_pAnchor = AddItem("Anchored");
		m_pAnchor->SetLabel("#HudPanel_Anchored");
		m_pAnchor->SetIcon('f');
		m_pAnchor->SetAmountMax(1.0f);
		m_pAnchor->SetAmountFormat(FORMAT_RAW);
		m_pAnchor->SetAmountDecimalPlaces(1);
		EnableItem(m_pAnchor);

		SetHeaderIconChar('z');
		break;

	case FF_WEAPON_JUMPGUN:
		m_pCharge->SetAmountMax(JUMPGUN_CHARGEUPTIME);
		m_pCharge->SetIcon('j');
		break;
	}


	AddPanelToHudOptions(
		FFQuantityPanel::GetName(),
		m_pszTitle,
		"Resources",
		"#HudPanel_Resources");
}

void CHudResourceWeaponCharge::VidInit()
{
	m_pCharge->SetAmount(0);

	if (m_pAnchor)
	{
		m_pAnchor->SetAmount(0);
		if (m_bShowAnchor)
			EnableItem(m_pAnchor);
		else
			DisableItem(m_pAnchor);
	}


}

void CHudResourceWeaponCharge::ApplySchemeSettings(
	IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	const wchar_t* pszTitle
		= g_pVGuiLocalize->Find(m_pszTitle);

	if (pszTitle)
		SetHeaderText(pszTitle);

	if (m_iWeaponID != FF_WEAPON_ASSAULTCANNON)
		return;

	wchar_t* pszAnchored
		= g_pVGuiLocalize->Find("#HudPanel_Anchored");

	if (pszAnchored)
		SetText(pszAnchored);

	SetUseToggleText(m_bShowText);
}

void CHudResourceWeaponCharge::AddPanelSpecificOptions(
	KeyValues* kvOptions)
{
	KeyValues* kvVisibility = new KeyValues("Values");
	kvVisibility->SetString("0", "#HudPanel_VisibilityNever");
	kvVisibility->SetString("1", "#HudPanel_VisibilityWhenActive");
	kvVisibility->SetString("2", "#HudPanel_VisibilityWhileEquipped");

	AddComboOption(
		kvOptions,
		"ShowPanel",
		"#HudPanel_Option_ShowPanel",
		kvVisibility,
		m_bDefaultShowPanel ? WHEN_ACTIVE : NEVER,
		0);

	AddItemOption(
		kvOptions,
		"Charge",
		m_iWeaponID == FF_WEAPON_ASSAULTCANNON ? "#HudPanel_WindUp" : "#HudPanel_Charge",
		{
			{
				ITEM_FORMAT,
				"ValueFormat",
				s_kvAmountFormatOptions->MakeCopy(),
				FORMAT_PERCENTAGE
			}
		});

	if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
	{
		AddShowTextOption(kvOptions);

		KeyValues* kvAnchorVisibility = new KeyValues("Values");
		kvAnchorVisibility->SetString("1", "#HudPanel_VisibilityAlways");
		kvAnchorVisibility->SetString("0", "#HudPanel_VisibilityNever");

		KeyValues* kvAnchorFormat = new KeyValues("Values");
		kvAnchorFormat->SetString(std::to_string(FORMAT_RAW).c_str(), "#FF_ITEM_TIMELEFT");

		AddItemOption(
			kvOptions,
			"Anchored",
			"#HudPanel_Anchored",
			{
				{
					ITEM_VISIBILITY,
					"ShowAnchor",
					kvAnchorVisibility,
					true
				},
				{
					ITEM_FORMAT,
					"AnchorFormat",
					kvAnchorFormat,
					FORMAT_RAW
				}
			});
	}
}

KeyValues* CHudResourceWeaponCharge::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();

	kvPreset->SetInt("x", 580);
	kvPreset->SetInt("y", 325);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_BOTTOM);

	KeyValues* kvValues = new KeyValues("PanelSpecificValues");
	kvValues->SetBool("ShowPanel", m_bDefaultShowPanel);
	kvValues->SetInt("ValueFormat", FORMAT_PERCENTAGE);

	switch (m_iWeaponID)
	{
	case FF_WEAPON_ASSAULTCANNON:
	{
		kvValues->SetBool("ShowText", true);
		kvValues->SetBool("ShowAnchor", true);
		kvValues->SetInt("AnchorFormat", FORMAT_RAW);
		break;
	}

	case FF_WEAPON_SNIPERRIFLE:
	{
		kvPreset->SetInt("x", 320);
		kvPreset->SetInt("y", 255);
		kvPreset->SetInt("alignH", ALIGN_CENTER);
		kvPreset->SetInt("alignV", ALIGN_TOP);
		kvPreset->SetInt("showPanel", 0);

		kvPreset->SetInt("showHeaderText", 0);
		kvPreset->SetInt("showHeaderIcon", 0);
		kvPreset->SetInt("showText", 0);
		kvPreset->SetInt("barWidth", 60);
		kvPreset->SetInt("barBorderWidth", 1);
		kvPreset->FindKey("Label")->SetInt("show", 0);
		kvPreset->FindKey("Amount")->SetInt("show", 0);
		kvPreset->SetInt("showPanelBorder", 0);
		kvPreset->SetInt("barHeight", 10);

		KeyValues* kvBar = kvPreset->FindKey("Bar");
		kvBar->SetInt("colorMode", COLOR_MODE_STEPPED);
		kvBar->SetInt("alpha", 30);

		KeyValues* kvBarBorder = kvPreset->FindKey("BarBorder");
		kvBarBorder->SetInt("show", 0);
		kvBarBorder->SetInt("colorMode", COLOR_MODE_STEPPED);
		kvBarBorder->SetInt("alpha", 155);

		KeyValues* kvIcon = kvPreset->FindKey("Icon");
		kvIcon->SetInt("show", 1);
		kvIcon->SetInt("shadow", 0);
		kvIcon->SetInt("anchorPosition", ANCHORPOS_MIDDLELEFT);
		kvIcon->SetInt("alignH", ALIGN_RIGHT);
		kvIcon->SetInt("offsetX", -2);

		kvPreset->FindKey("Label")->SetInt("shadow", 0);
		break;
	}

	default:
		break;
	}

	kvPreset->AddSubKey(kvValues);
	return kvPreset;
}

void CHudResourceWeaponCharge::ApplyStyleData(
	KeyValues* kvStyleData,
	KeyValues* kvDefaults)
{
	BaseClass::ApplyStyleData(
		kvStyleData,
		kvDefaults);

	KeyValues* kvPanelSpecificValues
		= kvStyleData->FindKey("PanelSpecificValues", true);

	KeyValues* kvDefaultPanelSpecificValues
		= kvDefaults->FindKey("PanelSpecificValues", true);

	std::optional<PanelVisibility> showPanel
		= GetPanelVisibility(
			"ShowPanel",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (showPanel.has_value())
		m_showPanel = showPanel.value();

	std::optional<AmountFormat> chargeFormat
		= GetAmountFormat(
			"ValueFormat",
			kvPanelSpecificValues,
			kvDefaultPanelSpecificValues);

	if (chargeFormat.has_value())
	{
		m_pCharge->SetAmountFormat(
			chargeFormat.value());
	}

	if (m_pAnchor)
	{
		int iShowText
			= GetInt(
				"ShowText",
				kvPanelSpecificValues,
				kvDefaultPanelSpecificValues);

		if (iShowText != -1)
			m_bShowText = iShowText == 1;

		SetUseToggleText(m_bShowText);

		int iShowAnchor
			= GetInt(
				"ShowAnchor",
				kvPanelSpecificValues,
				kvDefaultPanelSpecificValues);

		if (iShowAnchor != -1)
			m_bShowAnchor = iShowAnchor == 1;

		if (m_bShowAnchor)
			EnableItem(m_pAnchor);
		else
			DisableItem(m_pAnchor);

		m_pAnchor->SetAmountFormat(FORMAT_RAW);
	}
}

std::optional<CHudResourceWeaponCharge::PanelVisibility> CHudResourceWeaponCharge::GetPanelVisibility(
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
		? std::optional<PanelVisibility>()
		: static_cast<PanelVisibility>(value);
}

bool CHudResourceWeaponCharge::IsEquipped()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer || FF_IsPlayerSpec(pPlayer) || !FF_HasPlayerPickedClass(pPlayer))
		return false;

	CFFWeaponBase* pWeapon = pPlayer->GetActiveFFWeapon();
	if (!pWeapon || pWeapon->GetWeaponID() != m_iWeaponID)
		return false;

	return true;
}

float CHudResourceWeaponCharge::GetCharge()
{
	CFFWeaponBase* pWeapon
		= C_FFPlayer::GetLocalFFPlayer()
		->GetActiveFFWeapon();

	float flCharge = 0;

	switch (m_iWeaponID)
	{
	case FF_WEAPON_SNIPERRIFLE:
	{
		CFFWeaponSniperRifle* pRifle
			= static_cast<CFFWeaponSniperRifle*>(pWeapon);

		// Charge starts at the rifle's minimum damage while the shot is held.
		if (!pRifle->IsInFire())
		{
			flCharge = 0.0f;
		}
		else
		{
			flCharge
				= clamp(
					gpGlobals->curtime - pRifle->GetFireStartTime(),
					1.0f,
					FF_SNIPER_MAXCHARGE);
		}
		break;
	}

	case FF_WEAPON_ASSAULTCANNON:
		flCharge = static_cast<CFFWeaponAssaultCannon*>(pWeapon)->m_flChargeTime;
		break;

	case FF_WEAPON_JUMPGUN:
		flCharge = static_cast<CFFWeaponJumpgun*>(pWeapon)->GetClampedCharge();
		break;
	}

	return clamp(
		flCharge,
		0.0f,
		m_pCharge->GetAmountMax());
}

void CHudResourceWeaponCharge::UpdateCharge()
{
	if (m_showPanel == NEVER)
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
		return;
	}

	bool bIsAssultCannon
		= m_iWeaponID == FF_WEAPON_ASSAULTCANNON;

	if (IsInPreviewMode())
	{
		SetHiddenBits(0);

		if (m_iWeaponID == FF_WEAPON_ASSAULTCANNON)
			SetToggleTextVisible(true);

		m_pCharge->SetAmount(m_pCharge->GetAmountMax() * 0.6f);

		if (m_pAnchor)
		{
			m_pAnchor->SetAmountMax(1.0f);
			m_pAnchor->SetAmount(0.6f);
		}
		return;
	}

	if (!engine->IsInGame() || !IsEquipped())
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
		return;
	}

	const bool bAnchored
		= bIsAssultCannon
			&& C_FFPlayer::GetLocalFFPlayer()
				->m_bAssaultCannonAnchored;

	if (bIsAssultCannon)
		SetToggleTextVisible(bAnchored);

	if (m_pAnchor)
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		const float flDuration = pPlayer->m_flAssaultCannonAnchorDuration;
		if (bAnchored && flDuration > 0.0f)
		{
			const float flTimeLeft = clamp(
				pPlayer->m_flAssaultCannonAnchorEndTime - gpGlobals->curtime,
				0.0f,
				flDuration);

			m_pAnchor->SetAmountMax(flDuration);
			m_pAnchor->SetAmount(flTimeLeft);
		}
		else
			m_pAnchor->SetAmount(0.0f);
	}

	const float flCharge = GetCharge();

	m_pCharge->SetAmount(flCharge);

	bool bHidden
		= m_showPanel == WHEN_ACTIVE
			&& flCharge <= 0.0f
			&& !bAnchored;

	SetHiddenBits(
		bHidden? HIDEHUD_ALWAYS : DEFAULT_HIDDEN_BITS);
}

bool CHudResourceWeaponCharge::ShouldDraw()
{
	UpdateCharge();

	return CHudElement::ShouldDraw();
}
