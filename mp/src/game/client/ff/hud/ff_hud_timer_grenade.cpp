#include "cbase.h"

#include "ff_hud_timer_grenade.h"
#include "hud.h"
#include "c_ff_player.h"
#include "ff_playerclass_parse.h"
#include "ff_grenade_parse.h"
#include "ff_utils.h"
#include "ff_gamerules.h"
#include <algorithm>

using namespace vgui;

extern ConVar hud_grenadetimers;

CHudGrenadeTimer::CHudGrenadeTimer(
	const char* pElementName,
	const char* szPanelName,
	const char* szTitle,
	int iPrimeState)
	: CHudElement(pElementName), BaseClass(NULL, szPanelName),
	m_szTitle(szTitle), m_iPrimeState(iPrimeState)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);
	SetHeaderIconChar('f');
	SetUseToggleText(false);

	wchar_t wszTitle[64];
	g_pVGuiLocalize->ConvertANSIToUnicode(szTitle, wszTitle, sizeof(wszTitle));
	SetHeaderText(wszTitle);

	m_kvItemStyle = new KeyValues("GrenadeTimerStyle");
}

CHudGrenadeTimer::~CHudGrenadeTimer()
{
	m_kvItemStyle->deleteThis();
}

void CHudGrenadeTimer::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	wchar_t* localized = g_pVGuiLocalize->Find(m_szTitle);
	if (localized)
		SetHeaderText(localized);

	m_iHeaderClass = -1;
	UpdateGrenadeHeader();
}

void CHudGrenadeTimer::UpdateGrenadeHeader()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	int iClass = CLASS_NONE;
	if (pPlayer)
		iClass = pPlayer->GetClassSlot();

	if (iClass == m_iHeaderClass)
		return;

	m_iHeaderClass = iClass;

	SetHeaderIconChar('f');
	wchar_t* localized = g_pVGuiLocalize->Find(m_szTitle);
	if (localized)
		SetHeaderText(localized);

	if (iClass == CLASS_NONE)
		return;

	PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
	if (!ReadPlayerClassDataFromFileForSlot(
		filesystem,
		Class_IntToString(iClass),
		&hClassInfo,
		g_pGameRules->GetEncryptionKey()))
		return;

	const CFFPlayerClassInfo* pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);
	if (!pClassInfo)
		return;

	const char* pszGrenade = pClassInfo->m_szPrimaryClassName;
	if (m_iPrimeState == FF_GREN_PRIMETWO)
		pszGrenade = pClassInfo->m_szSecondaryClassName;

	if (Q_stricmp(pszGrenade, "None") == 0)
		return;

	GRENADE_FILE_INFO_HANDLE hGrenadeInfo = LookupGrenadeInfoSlot(pszGrenade);
	if (hGrenadeInfo == GetInvalidGrenadeInfoHandle())
		return;

	CFFGrenadeInfo* pGrenadeInfo = GetFileGrenadeInfoFromHandle(hGrenadeInfo);
	if (!pGrenadeInfo)
		return;

	localized = g_pVGuiLocalize->Find(pGrenadeInfo->szPrintName);
	if (localized)
	{
		SetHeaderText(localized);
	}
	else
	{
		wchar_t wszName[MAX_GRENADE_STRING];
		g_pVGuiLocalize->ConvertANSIToUnicode(pGrenadeInfo->szPrintName, wszName, sizeof(wszName));
		SetHeaderText(wszName);
	}

	// ScaledGlyphs uses different characters from the grenade scripts' Status Glyphs.
	char cIcon = 'Q';
	if (Q_stricmp(pszGrenade, "ff_grenade_mirv") == 0)
		cIcon = 'W';
	else if (Q_stricmp(pszGrenade, "ff_grenade_nail") == 0
		|| Q_stricmp(pszGrenade, "ff_grenade_laser") == 0)
		cIcon = 'E';
	else if (Q_stricmp(pszGrenade, "ff_grenade_concussion") == 0
		|| Q_stricmp(pszGrenade, "ff_grenade_slowfield") == 0)
		cIcon = 'R';
	else if (Q_stricmp(pszGrenade, "ff_grenade_emp") == 0)
		cIcon = 'T';
	else if (Q_stricmp(pszGrenade, "ff_grenade_napalm") == 0)
		cIcon = 'Y';

	SetHeaderIconChar(cIcon);
}

void CHudGrenadeTimer::AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	AddNumericOption(
		kvPanelSpecificOptions,
		"MaxVisibleTimers",
		"#HudPanel_Option_MaxVisible",
		3,
		1,
		6,
		0);
	KeyValues* kvMode = GetCountDirectionOptions();

	AddItemOption(
		kvPanelSpecificOptions,
		"Progress",
		"#FF_ITEM_PROGRESS",
		{
			{
				ITEM_FORMAT,
				"CountDown",
				kvMode,
				0
			}
		});

	AddBooleanOption(
		kvPanelSpecificOptions,
		"ReverseOrder",
		"#HudPanel_Option_ReverseOrder",
		false,
		0);

	AddHeading(kvPanelSpecificOptions, "#HudPanel_Heading_ThrownGrenade", 2);

	AddNumericOption(
		kvPanelSpecificOptions,
		"ThrownOpacity",
		"#HudPanel_Option_Opacity",
		50,
		0,
		100,
		2);
	KeyValues* kvTargets = new KeyValues("Values");
	kvTargets->SetString("0", "#HudPanel_Option_All");
	kvTargets->SetString("1", "#GameUI_Bar");
	kvTargets->SetString("2", "#GameUI_BarBorder");
	kvTargets->SetString("3", "#GameUI_BarBackground");
	kvTargets->SetString("4", "#GameUI_Icon");
	kvTargets->SetString("5", "#GameUI_Label");
	kvTargets->SetString("6", "#GameUI_Amount");

	AddComboOption(
		kvPanelSpecificOptions,
		"ThrownOpacityTarget",
		"#HudPanel_Option_OpacityTarget",
		kvTargets,
		FFQuantityItem::OPACITY_ALL,
		2);
}

KeyValues* CHudGrenadeTimer::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	kvPreset->SetInt("showHeaderText", 0);
	kvPreset->SetInt("barWidth", 100);
	kvPreset->SetInt("barHeight", 13);
	kvPreset->SetInt("headerIconSize", 5);
	kvPreset->SetInt("headerIconAnchorPosition", ANCHORPOS_MIDDLELEFT);
	kvPreset->SetInt("headerIconAlignHoriz", ALIGN_RIGHT);
	kvPreset->SetInt("headerIconAlignVert", ALIGN_CENTER);
	kvPreset->SetInt("headerIconX", -2);
	kvPreset->SetInt("headerIconY", 0);
	kvPreset->FindKey("Icon", true)->SetInt("show", 0);
	KeyValues* kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetBool("CountDown", false);
	kvPanelSpecificValues->SetInt("MaxVisibleTimers", 3);
	kvPanelSpecificValues->SetBool("ReverseOrder", false);
	kvPanelSpecificValues->SetInt("ThrownOpacity", 50);
	kvPanelSpecificValues->SetInt("ThrownOpacityTarget", FFQuantityItem::OPACITY_ALL);
	kvPreset->AddSubKey(kvPanelSpecificValues);
	return kvPreset;
}

void CHudGrenadeTimer::ApplyStyleData(KeyValues* kvStyleData,
	KeyValues* kvDefaultStyleData)
{
	// Preserve omitted settings so new rows match existing rows after partial previews.
	KeyValues* kvItemStyle = kvStyleData->MakeCopy();
	kvItemStyle->RecursiveMergeKeyValues(kvDefaultStyleData);
	kvItemStyle->RecursiveMergeKeyValues(m_kvItemStyle);

	m_kvItemStyle->deleteThis();
	m_kvItemStyle = kvItemStyle;
	BaseClass::ApplyStyleData(kvStyleData, kvDefaultStyleData);

	KeyValues* kvPanelSpecificValues = kvStyleData->FindKey("PanelSpecificValues", true);
	KeyValues* kvDefaultPanelSpecificValues = kvDefaultStyleData->FindKey("PanelSpecificValues", true);
	int iShowPanel = GetInt("ShowPanel", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iShowPanel != -1)
		m_bShowPanel = iShowPanel == 1;

	int iMaxVisibleTimers = GetInt("MaxVisibleTimers", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iMaxVisibleTimers != -1)
		m_iMaxVisibleTimers = clamp(iMaxVisibleTimers, 1, 6);

	int iCountDown = GetInt("CountDown", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iCountDown != -1)
		m_bCountDown = iCountDown == 1;

	int iReverseOrder = GetInt("ReverseOrder", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iReverseOrder != -1)
		m_bReverseOrder = iReverseOrder == 1;

	int iThrownOpacity = GetInt("ThrownOpacity", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iThrownOpacity != -1)
		m_iThrownOpacity = clamp(iThrownOpacity, 0, 100);
	int iThrownOpacityTarget = GetInt("ThrownOpacityTarget", kvPanelSpecificValues, kvDefaultPanelSpecificValues);
	if (iThrownOpacityTarget != -1)
	{
		m_thrownOpacityTarget = static_cast<FFQuantityItem::OpacityTarget>(clamp(
			iThrownOpacityTarget,
			static_cast<int>(FFQuantityItem::OPACITY_ALL),
			static_cast<int>(FFQuantityItem::OPACITY_AMOUNT)));
	}

	for (FFQuantityItem* pItem : m_items)
		ApplyTimerMode(pItem);

	UpdatePreview();
	ApplyVisibilityOptions();
}

void CHudGrenadeTimer::ApplyVisibilityOptions()
{
	if (m_bShowPanel)
	{
		if (IsInPreviewMode())
			SetHiddenBits(0);
		else
			SetHiddenBits(HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);
	}
	else
	{
		SetHiddenBits(HIDEHUD_ALWAYS);
	}
}

void CHudGrenadeTimer::ApplyTimerMode(FFQuantityItem* pItem)
{
	if (m_bCountDown)
	{
		pItem->SetLabel("#FF_ITEM_TIMELEFT");
		pItem->SetAmountFormat(FORMAT_RAW);
	}
	else
	{
		pItem->SetLabel("#FF_ITEM_PROGRESS");
		pItem->SetAmountFormat(FORMAT_PERCENTAGE);
	}
}

FFQuantityItem* CHudGrenadeTimer::CreateTimerItem()
{
	char szItemName[32];
	Q_snprintf(
		szItemName,
		sizeof(szItemName),
		"Fuse%u",
		static_cast<unsigned int>(m_items.size() + 1));
	FFQuantityItem* pItem = AddItem(szItemName);
	pItem->SetIcon('f');
	pItem->SetAmountMax(1.0f);
	pItem->ApplyStyleData(m_kvItemStyle, m_kvItemStyle);
	pItem->SetAmountDecimalPlaces(1);
	ApplyTimerMode(pItem);
	DisableItem(pItem);
	m_items.push_back(pItem);
	return pItem;
}

void CHudGrenadeTimer::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);
	CreateTimerItem();
	AddPanelToHudOptions(
		FFQuantityPanel::GetName(),
		m_szTitle,
		"Timers",
		"#HudPanel_Timers");
}

void CHudGrenadeTimer::VidInit()
{
	m_iHeaderClass = -1;
	UpdateGrenadeHeader();
	ResetTimer();
}

void CHudGrenadeTimer::Reset()
{
	ResetTimer();
}

void CHudGrenadeTimer::ResetTimer()
{
	m_timers.clear();
	for (FFQuantityItem* pItem : m_items)
		DisableItem(pItem);

	UpdatePreview();
}

void CHudGrenadeTimer::SetTimer(float flDuration)
{
	UpdateGrenadeHeader();
	Assert(flDuration > 0);
	if (flDuration <= 0)
		return;

	if (m_timers.size() >= m_iMaxVisibleTimers)
	{
		m_timers.push_back({gpGlobals->curtime, flDuration});
		return;
	}

	if (m_timers.size() == m_items.size())
		CreateTimerItem();

	FFQuantityItem* pItem = m_items[m_timers.size()];
	pItem->SetAmountMax(flDuration);
	if (m_bCountDown)
		pItem->SetAmount(flDuration);
	else
		pItem->SetAmount(0);

	pItem->SetOpacity(1.0f);
	EnableItem(pItem);
	m_timers.push_back({gpGlobals->curtime, flDuration});
}

int CHudGrenadeTimer::ActiveTimerCount() const
{
	int iCount = 0;
	for (const Timer& timer : m_timers)
	{
		if (gpGlobals->curtime < timer.m_flStart + timer.m_flDuration)
			++iCount;
	}

	return iCount;
}

bool CHudGrenadeTimer::ActiveTimer() const
{
	return ActiveTimerCount() > 0;
}

bool CHudGrenadeTimer::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	return engine->IsInGame() && hud_grenadetimers.GetBool() && ActiveTimer();
}

void CHudGrenadeTimer::UpdatePreview()
{
	if (!m_timers.empty() || m_items.empty())
		return;

	if (IsInPreviewMode())
	{
		while (m_items.size() < m_iMaxVisibleTimers)
			CreateTimerItem();

		for (size_t i = 0; i < m_items.size(); ++i)
		{
			FFQuantityItem* pItem = m_items[i];
			if (i >= m_iMaxVisibleTimers)
			{
				DisableItem(pItem);
				continue;
			}

			pItem->SetAmountMax(1.0f);
			size_t timerIndex = i;
			if (m_bReverseOrder)
				timerIndex = m_iMaxVisibleTimers - 1 - i;

			float flProgress = 1.0f
				- (static_cast<float>(timerIndex) + 1.0f)
				/ (static_cast<float>(m_iMaxVisibleTimers) + 1.0f);
			if (m_bCountDown)
				pItem->SetAmount(1.0f - flProgress);
			else
				pItem->SetAmount(flProgress);

			bool bHeld = i + 1 == m_iMaxVisibleTimers;
			if (m_bReverseOrder)
				bHeld = i == 0;

			if (bHeld)
				pItem->SetOpacity(1.0f);
			else
				pItem->SetOpacity(m_iThrownOpacity / 100.0f, m_thrownOpacityTarget);

			EnableItem(pItem);
		}
	}
	else
	{
		for (FFQuantityItem* pItem : m_items)
			DisableItem(pItem);
	}
}

void CHudGrenadeTimer::OnThink()
{
	BaseClass::OnThink();
	UpdateGrenadeHeader();
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	// Expire timers independently of painting, including when the HUD is hidden.
	for (auto it = m_timers.begin(); it != m_timers.end();)
	{
		if (gpGlobals->curtime >= it->m_flStart + it->m_flDuration)
			it = m_timers.erase(it);
		else
			++it;
	}

	// Keep the grenades closest to exploding; optionally reverse their display order.
	std::stable_sort(m_timers.begin(), m_timers.end(), [](const Timer& first, const Timer& second)
	{
		return first.m_flStart + first.m_flDuration < second.m_flStart + second.m_flDuration;
	});
	size_t visibleCount = m_timers.size();
	if (visibleCount > m_iMaxVisibleTimers)
		visibleCount = m_iMaxVisibleTimers;

	while (m_items.size() < visibleCount)
		CreateTimerItem();

	for (size_t i = 0; i < m_items.size(); ++i)
	{
		FFQuantityItem* pItem = m_items[i];
		if (i >= visibleCount)
		{
			DisableItem(pItem);
			continue;
		}

		size_t timerIndex = i;
		if (m_bReverseOrder)
			timerIndex = visibleCount - 1 - i;

		const Timer& timer = m_timers[timerIndex];
		float flElapsed = gpGlobals->curtime - timer.m_flStart;
		pItem->SetAmountMax(timer.m_flDuration);
		if (m_bCountDown)
			pItem->SetAmount(timer.m_flDuration - flElapsed);
		else
			pItem->SetAmount(flElapsed);

		EnableItem(pItem);

		const bool bNewest = std::none_of(m_timers.begin(), m_timers.end(), [&timer](const Timer& other)
		{
			return other.m_flStart > timer.m_flStart;
		});
		const bool bHeld = bNewest && pPlayer && pPlayer->m_iGrenadeState == m_iPrimeState;
		if (flElapsed < timer.m_flDuration * 0.15f || bHeld)
			pItem->SetOpacity(1.0f);
		else
			pItem->SetOpacity(m_iThrownOpacity / 100.0f, m_thrownOpacityTarget);
	}

	UpdatePreview();
}
