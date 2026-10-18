//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_crosshairinfo.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 02/03/2006
//	@brief client side Hud crosshair info
//
//	REVISIONS
//	---------
//	02/03/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_hud_crosshairinfo.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

//#include "debugoverlay_shared.h"

//#include "c_ff_player.h"
#include "c_ff_playerresource.h"
#include "c_ff_team.h"
#include "ff_gamerules.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"
#include "ff_playerclass_parse.h"

#define CROSSHAIRTYPE_NORMAL 0
#define CROSSHAIRTYPE_DISPENSER 1
#define CROSSHAIRTYPE_SENTRYGUN 2
#define CROSSHAIRTYPE_DETPACK 3
#define CROSSHAIRTYPE_MANCANNON 4

//=============================================================================
//
//	class CHudCrosshairInfo
//
//=============================================================================
CHudCrosshairInfoPanel::CHudCrosshairInfoPanel(const char* pElementName, int iTargetType)
	: CHudElement(pElementName), BaseClass(NULL, pElementName), m_iTargetType(iTargetType)
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits(DEFAULT_HIDDEN_BITS);
	SetUseToggleText(true);
	if (m_iTargetType == CROSSHAIRTYPE_DISPENSER)
	{
		m_bHideAmmo = false;
		m_bHideArmor = true;
		m_bHideCells = true;
		m_bHideNails = true;
		m_bHideShells = true;
		m_bHideRockets = true;
	}

	switch (m_iTargetType)
	{
	case CROSSHAIRTYPE_NORMAL:
		SetHeaderIconChar('s');
		break;
	case CROSSHAIRTYPE_SENTRYGUN:
		SetHeaderIconChar('1');
		break;
	case CROSSHAIRTYPE_DISPENSER:
		SetHeaderIconChar('4');
		break;
	case CROSSHAIRTYPE_MANCANNON:
		SetHeaderIconChar('6');
		break;
	}

	for (int i = 0; i < QUANTITY_COUNT; ++i)
	{
		m_items[i] = nullptr;
		m_values[i] = -1;
		m_maximums[i] = 0;
		m_quantityFormats[i] = FORMAT_OVER_MAX;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshairInfoPanel::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);

	const char* names[QUANTITY_COUNT] = {
		"Health", "Ammo", "Armor", "Cells", "Rockets", "Nails", "Shells", "Level"
	};
	const char* labels[QUANTITY_COUNT] = {
		"#FF_ITEM_HEALTH", "#FF_ITEM_AMMO", "#FF_ITEM_ARMOR", "#FF_ITEM_CELLS",
		"#FF_ITEM_ROCKETS", "#FF_ITEM_NAILS", "#FF_ITEM_SHELLS", "#FF_ITEM_LEVEL"
	};
	const char icons[QUANTITY_COUNT] = { 'a', 'r', 's', 'p', 'i', 't', 'r', 'd' };

	for (int i = 0; i < QUANTITY_COUNT; ++i)
	{
		const bool bSupported = i == HEALTH
			|| (i == ARMOR && (m_iTargetType == CROSSHAIRTYPE_NORMAL || m_iTargetType == CROSSHAIRTYPE_DISPENSER))
			|| (i == AMMO && m_iTargetType == CROSSHAIRTYPE_DISPENSER)
			|| (i >= CELLS && i <= SHELLS && m_iTargetType == CROSSHAIRTYPE_DISPENSER)
			|| ((i == LEVEL || i == SHELLS || i == ROCKETS) && m_iTargetType == CROSSHAIRTYPE_SENTRYGUN);
		if (!bSupported)
			continue;

		m_items[i] = AddItem(names[i]);
		m_items[i]->SetLabel(labels[i]);

		if (icons[i])
			m_items[i]->SetIcon(icons[i]);

		m_items[i]->SetAmountFormat(FORMAT_OVER_MAX);
		DisableItem(m_items[i]);
	}

	switch (m_iTargetType)
	{
	case CROSSHAIRTYPE_NORMAL:
		AddPanelToHudOptions(
			"CrosshairInfoPlayer",
			"#HudPanel_Player",
			"CrosshairInfo",
			"#HudPanel_CrosshairInfo");
		break;
	case CROSSHAIRTYPE_SENTRYGUN:
		AddPanelToHudOptions(
			"CrosshairInfoSentry",
			"#HudPanel_SentryGun",
			"CrosshairInfo",
			"#HudPanel_CrosshairInfo");
		break;
	case CROSSHAIRTYPE_DISPENSER:
		AddPanelToHudOptions(
			"CrosshairInfoDispenser",
			"#HudPanel_Dispenser",
			"CrosshairInfo",
			"#HudPanel_CrosshairInfo");
		break;
	case CROSSHAIRTYPE_MANCANNON:
		AddPanelToHudOptions(
			"CrosshairInfoManCannon",
			"#HudPanel_ManCannon",
			"CrosshairInfo",
			"#HudPanel_CrosshairInfo");
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Reset on map/vgui load
//-----------------------------------------------------------------------------
void CHudCrosshairInfoPanel::VidInit()
{
	Reset();
	ApplyVisibilityOptions();
}

//-----------------------------------------------------------------------------
// Purpose: Trace out and touch someone
//-----------------------------------------------------------------------------


void CHudCrosshairInfoPanel::Reset()
{
	if (IsInPreviewMode())
		return;

	m_flDrawTime = -m_flDrawDuration;
	for (int i = 0; i < QUANTITY_COUNT; ++i)
		SetQuantity(static_cast<Quantity>(i), -1, 0);
	UpdateQuantityVisibility();
}

void CHudCrosshairInfoPanel::SetQuantity(Quantity quantity, int iValue, int iMaximum)
{
	if (quantity == LEVEL && iValue >= 1 && iValue <= 3)
		SetHeaderIconChar('1' + iValue - 1);

	m_values[quantity] = iValue;
	m_maximums[quantity] = iMaximum;
	if (m_items[quantity] && iValue >= 0 && iMaximum > 0)
	{
		m_items[quantity]->SetAmountMax(iMaximum);
		m_items[quantity]->SetAmount(iValue);
	}
}

void CHudCrosshairInfoPanel::UpdateQuantityVisibility()
{
	for (int i = 0; i < QUANTITY_COUNT; ++i)
	{
		if (!m_items[i])
			continue;

		bool bShow = m_values[i] >= 0 && m_maximums[i] > 0;
		if (i == AMMO)
			bShow = bShow && !m_bHideAmmo;
		else if (i == ARMOR)
			bShow = bShow && !m_bHideArmor;
		else if (i == CELLS)
			bShow = bShow && !m_bHideCells;
		else if (i == NAILS)
			bShow = bShow && !m_bHideNails;
		else if (i == SHELLS)
			bShow = bShow && !m_bHideShells;
		else if (i == ROCKETS)
			bShow = bShow && !m_bHideRockets;
		else if (i == LEVEL)
			bShow = bShow && !m_bHideLevel;

		if (bShow)
			EnableItem(m_items[i]);
		else
			DisableItem(m_items[i]);
	}
}

bool CHudCrosshairInfoPanel::ShouldDraw()
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (IsInPreviewMode())
		return true;

	if (m_bOwnBuildable && !m_bShowOwnBuildable)
		return false;

	return gpGlobals->curtime < m_flDrawTime + m_flDrawDuration;
}

int CHudCrosshairInfoPanel::GetColorTeamNumber()
{
	if (IsInPreviewMode())
		return BaseClass::GetColorTeamNumber();

	return m_iColorTeam;
}

void CHudCrosshairInfoPanel::ApplyVisibilityOptions()
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
}

void CHudCrosshairInfoPanel::ShowPreview()
{
	for (int i = 0; i < QUANTITY_COUNT; ++i)
		SetQuantity(static_cast<Quantity>(i), -1, 0);

	SetQuantity(HEALTH, 75, 100);
	const char* pszPreviewTitle = nullptr;
	wchar_t* localized = g_pVGuiLocalize->Find("#HudPanel_Player");
	if (localized)
		SetHeaderText(localized);

	switch (m_iTargetType)
	{
	case CROSSHAIRTYPE_NORMAL:
		pszPreviewTitle = Class_IntToResourceString(CLASS_MEDIC);
		SetQuantity(ARMOR, 50, 100);
		break;
	case CROSSHAIRTYPE_SENTRYGUN:
		pszPreviewTitle = "#HudPanel_SentryGun";
		SetQuantity(LEVEL, 2, 3);
		SetQuantity(SHELLS, 75, 100);
		SetQuantity(ROCKETS, 10, 20);
		break;
	case CROSSHAIRTYPE_DISPENSER:
		pszPreviewTitle = "#HudPanel_Dispenser";
		SetQuantity(AMMO, 60, 100);
		SetQuantity(ARMOR, 50, 100);
		SetQuantity(CELLS, 60, 100);
		SetQuantity(ROCKETS, 20, 50);
		SetQuantity(NAILS, 40, 100);
		SetQuantity(SHELLS, 60, 100);
		break;
	case CROSSHAIRTYPE_MANCANNON:
		pszPreviewTitle = "#HudPanel_ManCannon";
		break;
	}

	if (pszPreviewTitle)
	{
		localized = g_pVGuiLocalize->Find(pszPreviewTitle);
		if (localized)
			SetText(localized);
	}
}

void CHudCrosshairInfoPanel::AddPanelSpecificOptions(KeyValues* kvPanelSpecificOptions)
{
	AddShowOption(kvPanelSpecificOptions);

	if (m_iTargetType != CROSSHAIRTYPE_NORMAL)
		AddBooleanOption(
			kvPanelSpecificOptions,
			"ShowOwnBuildable",
			"#HudPanel_Option_ShowOwnBuildable",
			false,
			0);

	AddNumericOption(
		kvPanelSpecificOptions,
		"DisplayDuration",
		"#HudPanel_Option_DisplayDuration",
		1000,
		500,
		5000,
		0);

	for (int i = 0; i < QUANTITY_COUNT; ++i)
	{
		if (!m_items[i])
			continue;

		char szOption[64], szLabel[96];
		Q_snprintf(
			szOption,
			sizeof(szOption),
			"%sFormat",
			m_items[i]->GetName());
		Q_snprintf(
			szLabel,
			sizeof(szLabel),
			"#FF_ITEM_%s",
			m_items[i]->GetName());
		Q_strupr(szLabel);

		ItemOption formatOption = {
			ITEM_FORMAT,
			szOption,
			s_kvAmountFormatOptions->MakeCopy(),
			FORMAT_OVER_MAX
		};

		if (i == HEALTH || (i == AMMO && m_iTargetType != CROSSHAIRTYPE_DISPENSER))
		{
			AddItemOption(
				kvPanelSpecificOptions,
				m_items[i]->GetName(),
				szLabel,
				{ formatOption });

			continue;
		}

		char szVisibility[64];
		Q_snprintf(
			szVisibility,
			sizeof(szVisibility),
			"Hide%s",
			m_items[i]->GetName());

		const bool bHideByDefault = m_iTargetType == CROSSHAIRTYPE_DISPENSER
			&& i != AMMO && i != LEVEL;

		AddItemOption(
			kvPanelSpecificOptions,
			m_items[i]->GetName(),
			szLabel,
			{
				{
					ITEM_VISIBILITY,
					szVisibility,
					GetQuantityVisibilityOptions(),
					bHideByDefault
				},
				formatOption
			});
	}

}

KeyValues* CHudCrosshairInfoPanel::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	kvPreset->SetInt("x", 285);
	kvPreset->SetInt("y", 240);
	kvPreset->SetInt("alignH", ALIGN_RIGHT);
	kvPreset->SetInt("alignV", ALIGN_MIDDLE);
	kvPreset->SetInt("showHeaderIcon", 1);
	KeyValues* kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
	kvPanelSpecificValues->SetBool("ShowPanel", true);
	kvPanelSpecificValues->SetInt("DisplayDuration", 1000);
	kvPanelSpecificValues->SetBool("HideAmmo", false);
	kvPanelSpecificValues->SetBool("HideArmor", m_iTargetType == CROSSHAIRTYPE_DISPENSER);
	if (m_items[CELLS])
		kvPanelSpecificValues->SetBool("HideCells", m_iTargetType == CROSSHAIRTYPE_DISPENSER);

	if (m_items[NAILS])
		kvPanelSpecificValues->SetBool("HideNails", m_iTargetType == CROSSHAIRTYPE_DISPENSER);

	if (m_items[SHELLS])
		kvPanelSpecificValues->SetBool("HideShells", m_iTargetType == CROSSHAIRTYPE_DISPENSER);

	if (m_items[ROCKETS])
		kvPanelSpecificValues->SetBool("HideRockets", m_iTargetType == CROSSHAIRTYPE_DISPENSER);

	if (m_items[LEVEL])
		kvPanelSpecificValues->SetBool("HideLevel", false);

	kvPanelSpecificValues->SetBool("ShowOwnBuildable", false);
	for (int i = 0; i < QUANTITY_COUNT; ++i)
	{
		if (!m_items[i])
			continue;

		char szOption[64];
		Q_snprintf(
			szOption,
			sizeof(szOption),
			"%sFormat",
			m_items[i]->GetName());
		kvPanelSpecificValues->SetInt(szOption, FORMAT_OVER_MAX);
	}

	kvPreset->AddSubKey(kvPanelSpecificValues);
	return kvPreset;
}

void CHudCrosshairInfoPanel::ApplyStyleData(KeyValues* kvStyleData, KeyValues* kvDefaultStyleData)
{
	const bool bWasPreview = IsInPreviewMode();
	BaseClass::ApplyStyleData(kvStyleData, kvDefaultStyleData);
	KeyValues* kvValues = kvStyleData->FindKey("PanelSpecificValues", true);
	KeyValues* kvDefaults = kvDefaultStyleData->FindKey("PanelSpecificValues", true);
	int iDuration = GetInt("DisplayDuration", kvValues, kvDefaults);
	if (iDuration != -1)
		m_flDrawDuration = clamp(iDuration, 500, 5000) / 1000.0f;

	int iShowPanel = GetInt("ShowPanel", kvValues, kvDefaults);
	if (iShowPanel != -1)
		m_bShowPanel = iShowPanel == 1;

	int iHideAmmo = GetInt("HideAmmo", kvValues, kvDefaults);
	if (iHideAmmo != -1)
		m_bHideAmmo = iHideAmmo == 1;

	int iHideArmor = GetInt("HideArmor", kvValues, kvDefaults);
	if (iHideArmor != -1)
		m_bHideArmor = iHideArmor == 1;

	int iShowOwnBuildable = GetInt("ShowOwnBuildable", kvValues, kvDefaults);
	if (iShowOwnBuildable != -1)
		m_bShowOwnBuildable = iShowOwnBuildable == 1;

	int iHideCells = GetInt("HideCells", kvValues, kvDefaults);
	if (iHideCells != -1)
		m_bHideCells = iHideCells == 1;

	int iHideNails = GetInt("HideNails", kvValues, kvDefaults);
	if (iHideNails != -1)
		m_bHideNails = iHideNails == 1;

	int iHideShells = GetInt("HideShells", kvValues, kvDefaults);
	if (iHideShells != -1)
		m_bHideShells = iHideShells == 1;

	int iHideRockets = GetInt("HideRockets", kvValues, kvDefaults);
	if (iHideRockets != -1)
		m_bHideRockets = iHideRockets == 1;

	int iHideLevel = GetInt("HideLevel", kvValues, kvDefaults);
	if (iHideLevel != -1)
		m_bHideLevel = iHideLevel == 1;

	for (int i = 0; i < QUANTITY_COUNT; ++i)
	{
		if (!m_items[i])
			continue;

		char szOption[64];
		Q_snprintf(
			szOption,
			sizeof(szOption),
			"%sFormat",
			m_items[i]->GetName());
		std::optional<AmountFormat> display = GetAmountFormat(szOption, kvValues, kvDefaults);
		if (display.has_value())
			m_quantityFormats[i] = display.value();

		m_items[i]->SetAmountFormat(m_quantityFormats[i]);
	}

	if (IsInPreviewMode())
		ShowPreview();
	else if (bWasPreview)
		Reset();

	UpdateQuantityVisibility();
	ApplyVisibilityOptions();
}

DECLARE_HUDELEMENT(CHudCrosshairInfo);
DECLARE_HUDELEMENT(CHudCrosshairInfoPlayer);
DECLARE_HUDELEMENT(CHudCrosshairInfoSentry);
DECLARE_HUDELEMENT(CHudCrosshairInfoDispenser);
DECLARE_HUDELEMENT(CHudCrosshairInfoManCannon);

CHudCrosshairInfoPlayer::CHudCrosshairInfoPlayer(const char* pElementName)
	: CHudCrosshairInfoPanel(pElementName, CROSSHAIRTYPE_NORMAL)
{
}

CHudCrosshairInfoSentry::CHudCrosshairInfoSentry(const char* pElementName)
	: CHudCrosshairInfoPanel(pElementName, CROSSHAIRTYPE_SENTRYGUN)
{
}

CHudCrosshairInfoDispenser::CHudCrosshairInfoDispenser(const char* pElementName)
	: CHudCrosshairInfoPanel(pElementName, CROSSHAIRTYPE_DISPENSER)
{
}

CHudCrosshairInfoManCannon::CHudCrosshairInfoManCannon(const char* pElementName)
	: CHudCrosshairInfoPanel(pElementName, CROSSHAIRTYPE_MANCANNON)
{
}

CHudCrosshairInfo::CHudCrosshairInfo(const char* pElementName)
	: CHudElement(pElementName), BaseClass(NULL, "HudCrosshairInfo")
{
	SetParent(g_pClientMode->GetViewport());
	SetPaintEnabled(false);
	SetPaintBackgroundEnabled(false);
}

void CHudCrosshairInfo::Init()
{
	ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CHudCrosshairInfo::VidInit()
{
	Reset();
}

void CHudCrosshairInfo::Reset()
{
	m_flStartTime = 0.0f;
	CHudCrosshairInfoPanel* panels[] = {
		GET_HUDELEMENT(CHudCrosshairInfoPlayer),
		GET_HUDELEMENT(CHudCrosshairInfoSentry),
		GET_HUDELEMENT(CHudCrosshairInfoDispenser),
		GET_HUDELEMENT(CHudCrosshairInfoManCannon)
	};
	for (int i = 0; i < ARRAYSIZE(panels); ++i)
	{
		if (panels[i] && !panels[i]->IsInPreviewMode())
			panels[i]->Reset();
	}
}

void CHudCrosshairInfo::OnTick( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return;

	if( !pPlayer->IsAlive() )
	{
		Reset();
		return;
	}

	// Check for crosshair info every x seconds
	if( m_flStartTime < gpGlobals->curtime )
	{
		// Store off when to trace next
		m_flStartTime = gpGlobals->curtime + m_flDuration;

		// Get our forward vector
		Vector vecForward;
		pPlayer->EyeVectors( &vecForward );

		VectorNormalize( vecForward );

		// Get eye position
		Vector vecOrigin = pPlayer->EyePosition();

		//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 1024.f ), 0, 0, 255, false, 3.0f );
		//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 64.f ) + Vector( 0, 0, 8 ), 255, 0, 0, false, 3.0f );
		//debugoverlay->AddLineOverlay( vecOrigin + ( vecForward * 64.f ), vecOrigin + ( vecForward * 64.f ) + Vector( 0, 0, -8 ), 255, 0, 0, false, 3.0f );

		trace_t tr;
		//Trace needs COLLISION_GROUP_PUSHAWAY flag added or it wont hit jumppads -GreenMushy
		UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * 1024.f ), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_PLAYER | COLLISION_GROUP_PUSHAWAY, &tr );

		// If we hit something...
		if( tr.DidHit() )
		{
			// Some defaults...
			bool bBuildable = false;
			C_FFPlayer *pHitPlayer = NULL;

			// If we hit a player
			if( tr.m_pEnt->IsPlayer() )
			{					
				if( tr.m_pEnt->IsAlive() )
					pHitPlayer = ToFFPlayer( tr.m_pEnt );
			}
			// If we hit a sentrygun
			else if( tr.m_pEnt->Classify() == CLASS_SENTRYGUN )
			{
				C_FFSentryGun *pSentryGun = ( C_FFSentryGun * )tr.m_pEnt;
				if( !pSentryGun->IsBuilt() )
					return;					

				if( pSentryGun->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pSentryGun->m_hOwner.Get() );
				}
			}
			// If we hit a dispenser
			else if( tr.m_pEnt->Classify() == CLASS_DISPENSER )
			{
				C_FFDispenser *pDispenser = ( C_FFDispenser * )tr.m_pEnt;
				if( !pDispenser->IsBuilt() )
					return;

				if( pDispenser->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pDispenser->m_hOwner.Get() );
				}					
			}
			// If we hit a man cannon
			else if( tr.m_pEnt->Classify() == CLASS_MANCANNON )
			{
				C_FFManCannon *pManCannon = (C_FFManCannon *)tr.m_pEnt;
				if( !pManCannon->IsBuilt() )
					return;

				if( pManCannon->IsAlive() )
				{
					bBuildable = true;
					pHitPlayer = ToFFPlayer( pManCannon->m_hOwner.Get() );
				}
			}

			// If the players/objects aren't "alive" pHitPlayer will still be NULL
			// and we'll bail here...

			// If we got a player/owner
			if( pHitPlayer )
			{
				// Get at the game resources
				if( !g_FF_PR )
				{
					Warning( "[Crosshair Info] Failed to get game resources!\n" );
					return;
				}

				// Are we a medic?
				//bool bWeMedic = ( pPlayer->GetClassSlot() == CLASS_MEDIC );
				// Are we an engineer?
				//bool bWeEngy = ( pPlayer->GetClassSlot() == 9 );
				// Are we looking at a spy?
				bool bTheySpy = ( pHitPlayer->GetClassSlot() == CLASS_SPY );
				
				// For the player/owner name
				char szName[ MAX_PLAYER_NAME_LENGTH ];
				// For the class
				char szClass[ MAX_PLAYER_NAME_LENGTH ];
					
				// Get their real name now (deal w/ non teammate/ally spies later)
				Q_strcpy( szName, g_FF_PR->GetPlayerName( pHitPlayer->index ) );

				if( bBuildable )
				{
					switch( tr.m_pEnt->Classify() )
					{
					case CLASS_SENTRYGUN:
						Q_strcpy( szClass, "#FF_PLAYER_SENTRYGUN" );
						break;
					case CLASS_DISPENSER:
						Q_strcpy( szClass, "#FF_PLAYER_DISPENSER" );
						break;
					case CLASS_MANCANNON:
						Q_strcpy( szClass, "#FF_PLAYER_MANCANNON" );
						break;
					}
				}
				else
				{
					// Get the players' class always
					Q_strcpy( szClass, Class_IntToResourceString( g_FF_PR->GetClass( pHitPlayer->index ) ) );
				}

				// Default
				int iHealth = -1, iArmor = -1, iCells = -1, iRockets = -1, iNails = -1, iShells = -1, iLevel = -1;
				bool bOwnBuildable = false;
				int CROSSHAIRTYPE = CROSSHAIRTYPE_NORMAL;
				// Default
				m_iTeam = pHitPlayer->GetTeamNumber();
				m_iClass = g_FF_PR->GetClass(pHitPlayer->index);
				
				if ( (pPlayer == pHitPlayer) && (bBuildable) ) // looking at our own buildable
					bOwnBuildable = true;

				if( bBuildable )
				{
					// Now on teammates/allies can see teammate/allies
					// buildable info according to:
					// Bug #0000463: Hud Crosshair Info - douched
					//if( bWeEngy )
					//{
						C_FFBuildableObject *pBuildable = (C_FFBuildableObject *)tr.m_pEnt;

						//Get Buildable health percent here no matter what class
						iHealth = pBuildable->GetHealth();
							
						if( pBuildable->Classify() == CLASS_DISPENSER )
						{
							CROSSHAIRTYPE = CROSSHAIRTYPE_DISPENSER;
							iRockets = ( ( C_FFDispenser * )pBuildable )->GetRockets();
							iShells = ( ( C_FFDispenser * )pBuildable )->GetShells();
							iCells = ( ( C_FFDispenser * )pBuildable )->GetCells();
							iNails = ( ( C_FFDispenser* )pBuildable )->GetNails();
							iArmor = ( ( C_FFDispenser * )pBuildable )->GetArmor();
						}
						else if( pBuildable->Classify() == CLASS_SENTRYGUN )
						{
							CROSSHAIRTYPE = CROSSHAIRTYPE_SENTRYGUN;
							iLevel = ( ( C_FFSentryGun * )pBuildable )->GetLevel();
							iRockets = ( ( C_FFSentryGun * )pBuildable )->GetRockets();
							iShells = ( ( C_FFSentryGun * )pBuildable )->GetShells();
						}
						else if( pBuildable->Classify() == CLASS_MANCANNON )
						{
							CROSSHAIRTYPE = CROSSHAIRTYPE_MANCANNON;
							iArmor = -1;
						}
						else
						{
							iArmor = -1;
						}
					//}
				}
				else
				{						
					iHealth = pHitPlayer->GetHealth();
					iArmor = pHitPlayer->GetArmor();
				}

				if( FFGameRules()->PlayerRelationship( pPlayer, pHitPlayer ) != GR_TEAMMATE )
				{
					// We're looking at a non teammate/ally
					// Only thing we care about is if we are a medic or we're looking
					// at a spy because otherwise we've done everything above
	
					if( !bBuildable )
					{
						// We're looking at a player
							
						if( bTheySpy )
						{
							// We're looking at an enemy/non-allied spy

							if( pHitPlayer->IsDisguised() )
							{
								// The spy is disguised so we do some special stuff
								// to try and fake out the player - like show the class
								// we're disguised as and try to steal a name from a
								// a player on whatever team we are disguised as playing
								// as whatever class we are disguised as. If that fails
								// we use a name from the team we're disguised as. If that
								// fails we use the real name.

								// Get the disguised class
								/*int iClassSlot*/ m_iClass = pHitPlayer->GetDisguisedClass();
								Q_strcpy( szClass, Class_IntToResourceString( m_iClass ) );

								// Get the disguised team
								m_iTeam = pHitPlayer->GetDisguisedTeam();

								// If this spy is disguised as our team we need to show his
								// health/armor
								if( m_iTeam == pPlayer->GetTeamNumber() )
								{
									iHealth = pHitPlayer->GetHealth();
									iArmor = pHitPlayer->GetArmor();
								}

								// Or, if this spy is disguised as an ally of our team we
								// need to show his health/armor
								if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), m_iTeam ) == GR_TEAMMATE )
								{
									iHealth = pHitPlayer->GetHealth();
									iArmor = pHitPlayer->GetArmor();

									PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
									if ( ReadPlayerClassDataFromFileForSlot(filesystem, Class_IntToString(m_iClass), &hClassInfo, g_pGameRules->GetEncryptionKey()) )
									{
										const CFFPlayerClassInfo*pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);
										iHealth = pHitPlayer->GetHealthPercentage()*pClassInfo->m_iHealth/100;
										iArmor = pHitPlayer->GetArmorPercentage()*pClassInfo->m_iMaxArmour/100;
									}
								}

								// TODO: Could be bugs with this spy tracking thing in that
								// a player who's name is being used as a spy ID drops
								// and that name is still being used because the spy hasn't
								// changed disguises...

								// Check to see if we've ID'd this spy before as the 
								// disguise he's currently disguised as
								if( pPlayer->m_hSpyTracking[ pHitPlayer->index ].SameGuy( m_iTeam, m_iClass ) )
									Q_strcpy( szName, pPlayer->m_hSpyTracking[ pHitPlayer->index ].m_szName );
								else
								{
									// Change name (if we can) to someone on the team iTeam
									// that is playing the class this guy is disguised as

									// Gonna generate an array of people on the team we're disguised as
									// and a sub-array of people who share the same class we're disguised as
									// so that we can randomly pick a name later
									int iPlayers[ 128 ], iPlayersWithSameClass[ 128 ], iCount = 0, iSameClassCount = 0;
	
									bool bDone = false;
									for( int i = 1; i < gpGlobals->maxClients; i++ )
									{
										// Skip this spy - kind of useless if it tells us
										// our real name, eh? Using our real name is a last resort
										if( i == pHitPlayer->index )
											continue;

										if( g_FF_PR->IsConnected( i ) )
										{
											// If the guy's on the team we're disguised as...
											if( g_FF_PR->GetTeam( i ) == m_iTeam )
											{
												// Store off the player index since we found
												// someone on the team we're disguised as
												iPlayers[ iCount++ ] = i;

												// If the guy's playing as the class we're disguised as...
												if( g_FF_PR->GetClass( i ) == m_iClass )
												{
													// Store off the player index since we found
													// someone with the same class 
													iPlayersWithSameClass[ iSameClassCount++ ] = i;
												}
											}
										}
									}

									// If no one was on the other team, add the real name
									// to the array of possible choices
									if( iCount == 0 )
										iPlayers[ iCount++ ] = pHitPlayer->index;
									
									// If someone was on the other team with the same class we're disguised as
									// we steal a random name from one of the players in the set
									if( iSameClassCount != 0 )
									{			
										// So we got an array of indexes to players of whom we can steal
										// their name, so randomly steal one
										Q_strcpy( szName, g_FF_PR->GetPlayerName( iPlayersWithSameClass[ random->RandomInt( 0, iSameClassCount - 1 ) ] ) );
										bDone = true;
									}
	
									// We iterated around and found no one on the team we're disguised as
									// playing as the class we're disguised as so just pick a guy from
									// the team we're disguised as (or use real name if iCount was 0)
									if( !bDone )
									{
										// So we got an array of indexes to players of whom we can steal
										// their name, so randomly steal one
										Q_strcpy( szName, g_FF_PR->GetPlayerName( iPlayers[ random->RandomInt( 0, iCount - 1 ) ] ) );
									}

									// Store off the spies name, class & team in case we ID him again
									// and he hasn't changed disguise
									pPlayer->m_hSpyTracking[ pHitPlayer->index ].Set( szName, m_iTeam, m_iClass );
								}
							}
							// Jiggles: Don't draw anything if we're looking at a cloaked enemy spy
							if( pHitPlayer->IsCloaked() )
								return;  
						}
					}					
				}

				// Set up local crosshair info struct
				pPlayer->m_hCrosshairInfo.Set( szName, m_iTeam, m_iClass );

				// NOW! Remember team is 1 higher than the actual team
				// If health/armor are -1 then we don't show it

				// Convert to unicode & localize stuff

				const char *pszOldName = szName;
				int iBufSize = ( int )strlen( pszOldName ) * 2;
				char *pszNewName = ( char * )_alloca( iBufSize );

				UTIL_MakeSafeName( pszOldName, pszNewName, iBufSize );

				wchar_t wszName[ 256 ];
				g_pVGuiLocalize->ConvertANSIToUnicode( pszNewName, wszName, sizeof( wszName ) );

				wchar_t wszClass[ 256 ];					
				wchar_t *pszTemp = g_pVGuiLocalize->Find( szClass );
				if( pszTemp )
					wcscpy( wszClass, pszTemp );
				else
				{
					wchar_t* pszUnknown = g_pVGuiLocalize->Find("#HudPanel_Unknown");
					V_snwprintf(
						wszClass,
						ARRAYSIZE(wszClass),
						L"%ls",
						pszUnknown ? pszUnknown : L"Unknown");
				}
				
				// Only the current target type should linger; leave editor previews alone.
				CHudCrosshairInfoPanel* panels[] = {
					GET_HUDELEMENT(CHudCrosshairInfoPlayer),
					GET_HUDELEMENT(CHudCrosshairInfoDispenser),
					GET_HUDELEMENT(CHudCrosshairInfoSentry),
					GET_HUDELEMENT(CHudCrosshairInfoManCannon)
				};
				CHudCrosshairInfoPanel* pTargetPanel = nullptr;
				for (int i = 0; i < ARRAYSIZE(panels); ++i)
				{
					CHudCrosshairInfoPanel* pPanel = panels[i];
					if (!pPanel || pPanel->IsInPreviewMode())
						continue;

					if (pPanel->m_iTargetType == CROSSHAIRTYPE)
						pTargetPanel = pPanel;
					else
						pPanel->Reset();
				}

				if (!pTargetPanel)
					return;

				pTargetPanel->m_bOwnBuildable = bOwnBuildable;
				pTargetPanel->SetHeaderText(wszName);
				pTargetPanel->SetText(wszClass);
				pTargetPanel->m_iColorTeam = m_iTeam;
				pTargetPanel->SetTeamColor();

				for (int i = 0; i < CHudCrosshairInfoPanel::QUANTITY_COUNT; ++i)
					pTargetPanel->SetQuantity(static_cast<CHudCrosshairInfoPanel::Quantity>(i), -1, 0);

				if (bBuildable)
				{
					C_FFBuildableObject* pBuildable = static_cast<C_FFBuildableObject*>(tr.m_pEnt);
					pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::HEALTH, iHealth, pBuildable->GetMaxHealth());
					const bool bFriendlyBuildable = bOwnBuildable
						|| FFGameRules()->PlayerRelationship(pPlayer, pHitPlayer) == GR_TEAMMATE;
					if (bFriendlyBuildable && CROSSHAIRTYPE == CROSSHAIRTYPE_DISPENSER)
					{
						C_FFDispenser* pDispenser = static_cast<C_FFDispenser*>(pBuildable);
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::AMMO, pDispenser->GetAmmoPercent(), 100);
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::ARMOR, iArmor, iArmor + pDispenser->NeedsArmor());
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::CELLS, iCells, iCells + pDispenser->NeedsCells());
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::ROCKETS, iRockets, iRockets + pDispenser->NeedsRockets());
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::NAILS, iNails, iNails + pDispenser->NeedsNails());
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::SHELLS, iShells, iShells + pDispenser->NeedsShells());
					}
					else if (CROSSHAIRTYPE == CROSSHAIRTYPE_SENTRYGUN)
					{
						pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::LEVEL, iLevel, 3);

						if (bFriendlyBuildable)
						{
							C_FFSentryGun* pSentryGun = static_cast<C_FFSentryGun*>(pBuildable);
							pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::SHELLS, iShells, pSentryGun->GetMaxShells());
							pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::ROCKETS, iRockets, pSentryGun->GetMaxRockets());
						}
					}
				}
				else
				{
					int iMaxHealth = pHitPlayer->GetMaxHealth();
					int iMaxArmor = pHitPlayer->GetMaxArmor();
					// Use the displayed class for enemy disguises, not the real spy class.
					if (bTheySpy && pHitPlayer->IsDisguised()
						&& FFGameRules()->PlayerRelationship(pPlayer, pHitPlayer) != GR_TEAMMATE)
					{
						PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
						if (ReadPlayerClassDataFromFileForSlot(
							filesystem,
							Class_IntToString(m_iClass),
							&hClassInfo,
							g_pGameRules->GetEncryptionKey()))
						{
							const CFFPlayerClassInfo* pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);
							if (pClassInfo)
							{
								iMaxHealth = pClassInfo->m_iHealth;
								iMaxArmor = pClassInfo->m_iMaxArmour;
							}
						}
					}

					pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::HEALTH, iHealth, iMaxHealth);
					pTargetPanel->SetQuantity(CHudCrosshairInfoPanel::ARMOR, iArmor, iMaxArmor);
				}

				pTargetPanel->UpdateQuantityVisibility();

				// Start drawing
				pTargetPanel->m_flDrawTime = gpGlobals->curtime;
			}
			else
			{
				// Hit something but not a player/dispenser/sentrygun
				pPlayer->m_hCrosshairInfo.Set( "", 0, 0 );
			}
		}
		else
		{
			// Didn't hit anything!
			pPlayer->m_hCrosshairInfo.Set( "", 0, 0 );
		}
	}
}
