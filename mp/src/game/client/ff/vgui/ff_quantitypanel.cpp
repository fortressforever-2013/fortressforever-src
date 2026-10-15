/********************************************************************
	created:	2010/08
	filename: 	cl_dll\ff\ff_hud_quantitypanel.cpp
	file path:	cl_dll\ff
	file base:	ff_hud_quantitypanel
	file ext:	cpp
	author:		Elmo

	purpose:	Customisable Quanitity Panel for the HUD
*********************************************************************/

#include "cbase.h"
#include "ff_quantitypanel.h"

#include "c_ff_player.h"
#include "hudelement.h"
#include "ff_utils.h"

#include <vgui/IVGui.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>

#include "ienginevgui.h"
#include "iclientmode.h"

#include "c_playerresource.h"
extern C_PlayerResource* g_PR;

#include "ff_customhudoptions_assignpresets.h"
#include <vprof.h>
#include <optional>
extern CFFCustomHudAssignPresets* g_AP;

extern ConVar cl_teamcolourhud;

KeyValues* FFQuantityPanel::s_kvAmountFormatOptions;

namespace vgui
{
	KeyValues* FFQuantityPanel::s_kvNoData = new KeyValues("NoData");

	FFQuantityPanel::FFQuantityPanel(
		Panel* parent,
		const char* panelName)
		: BaseClass(parent, panelName)
	{
		m_scale = GetScaleFromScreenSize();

		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(FFQuantityPanel::CORNERS_SQUARE);
		SetPaintBorderEnabled(false);
		SetPaintEnabled(true);
		SetBgColor(TEAM_COLOR_SPECTATOR);

		if (!s_kvAmountFormatOptions)
		{
			KeyValues* kvAmountFormatOptions
				= new KeyValues("Values");

			kvAmountFormatOptions->SetString(
				std::to_string(FORMAT_RAW).c_str(),
				"#HudPanel_FormatRaw");
			kvAmountFormatOptions->SetString(
				std::to_string(FORMAT_PERCENTAGE).c_str(),
				"#HudPanel_FormatPercentage");
			kvAmountFormatOptions->SetString(
				std::to_string(FORMAT_OVER_MAX).c_str(),
				"#HudPanel_FormatOverMax");

			s_kvAmountFormatOptions
				= kvAmountFormatOptions;
		}
	}

	void FFQuantityPanel::ApplySchemeSettings(
		IScheme* pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		HScheme quantityPanelScheme
			= scheme()->LoadSchemeFromFile(
				"resource/QuantityPanelScheme.res",
				"QuantityPanelScheme");

		IScheme* qbScheme
			= scheme()->GetIScheme(quantityPanelScheme);

		for (int i = 0; i < QUANTITYITEMICONSIZES; ++i)
		{
			m_hfHeaderIconFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityPanelHeaderIcon%d", i), true);
			m_hfHeaderIconFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityPanelHeaderIconShadow%d", i), true);
			m_hfHeaderIconFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityPanelHeaderIcon%d", i), false);
		}

		for (int i = 0; i < QUANTITYPANELTEXTSIZES; ++i)
		{
			m_hfHeaderTextFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityPanelHeader%d", i), true);
			m_hfHeaderTextFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityPanelHeaderShadow%d", i), true);
			m_hfHeaderTextFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityPanelHeader%d", i), false);

			m_hfTextFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityPanel%d", i), true);
			m_hfTextFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityPanelShadow%d", i), true);
			m_hfTextFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityPanel%d", i), false);

			m_hfVerdanaTextFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityItemVerdana%d", i), true);
			m_hfVerdanaTextFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityItemVerdanaShadow%d", i), true);
			m_hfVerdanaTextFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityItemVerdana%d", i), false);
		}

		m_bHeaderIconFontDirty = true;
		m_bHeaderTextFontDirty = true;
		m_bTextFontDirty = true;

		RecalculatePanelColor();
		InvalidateLayout();
	}

	KeyValues* FFQuantityPanel::GetDefaultStyleData()
	{
		KeyValues* kvPreset = new KeyValues("StyleData");

		kvPreset->SetInt("x", 580);
		kvPreset->SetInt("y", 330);
		kvPreset->SetInt("alignH", ALIGN_RIGHT);
		kvPreset->SetInt("alignV", ALIGN_TOP);

		kvPreset->SetInt("showPanel", 1);
		kvPreset->SetInt("panelMargin", 5);
		kvPreset->SetInt("showPanelBorder", 1);
		kvPreset->SetInt("panelBorderWidth", 1);
		kvPreset->SetInt("panelBorderColorMode", COLOR_MODE_CUSTOM);
		kvPreset->SetInt("panelBorderRed", 199);
		kvPreset->SetInt("panelBorderGreen", 219);
		kvPreset->SetInt("panelBorderBlue", 255);
		kvPreset->SetInt("panelBorderAlpha", 215);
		kvPreset->SetInt("panelType", CORNERS_SQUARE);
		kvPreset->SetInt("panelColorMode", COLOR_MODE_TEAM);
		kvPreset->SetInt("panelRed", 255);
		kvPreset->SetInt("panelGreen", 255);
		kvPreset->SetInt("panelBlue", 255);
		kvPreset->SetInt("panelAlpha", 150);

		kvPreset->SetInt("showHeaderIcon", 1);
		kvPreset->SetInt("headerIconSize", 7);
		kvPreset->SetInt("headerIconShadow", 0);
		kvPreset->SetInt("headerIconAnchorPosition", ANCHORPOS_TOPLEFT);
		kvPreset->SetInt("headerIconAlignHoriz", ALIGN_RIGHT);
		kvPreset->SetInt("headerIconAlignVert", ALIGN_BOTTOM);
		kvPreset->SetInt("headerIconX", 18);
		kvPreset->SetInt("headerIconY", -3);
		kvPreset->SetInt("headerIconColorMode", COLOR_MODE_CUSTOM);
		kvPreset->SetInt("headerIconRed", 255);
		kvPreset->SetInt("headerIconGreen", 255);
		kvPreset->SetInt("headerIconBlue", 255);
		kvPreset->SetInt("headerIconAlpha", 255);

		kvPreset->SetInt("showHeaderText", 1);
		kvPreset->SetInt("headerTextSize", 6);
		kvPreset->SetInt("headerTextShadow", 0);
		kvPreset->SetInt("headerTextFontVerdana", 0);
		kvPreset->SetInt("headerTextAnchorPosition", ANCHORPOS_TOPLEFT);
		kvPreset->SetInt("headerTextAlignHoriz", ALIGN_LEFT);
		kvPreset->SetInt("headerTextAlignVert", ALIGN_BOTTOM);
		kvPreset->SetInt("headerTextX", 20);
		kvPreset->SetInt("headerTextY", -10);
		kvPreset->SetInt("headerTextColorMode", COLOR_MODE_CUSTOM);
		kvPreset->SetInt("headerTextRed", 255);
		kvPreset->SetInt("headerTextGreen", 255);
		kvPreset->SetInt("headerTextBlue", 255);
		kvPreset->SetInt("headerTextAlpha", 255);

		kvPreset->SetInt("showText", 1);
		kvPreset->SetInt("textSize", 4);
		kvPreset->SetInt("textShadow", 0);
		kvPreset->SetInt("textFontVerdana", 0);
		kvPreset->SetInt("textAnchorPosition", ANCHORPOS_TOPLEFT);
		kvPreset->SetInt("textAlignHoriz", ALIGN_LEFT);
		kvPreset->SetInt("textAlignVert", ALIGN_BOTTOM);
		kvPreset->SetInt("textX", 21);
		kvPreset->SetInt("textY", -1);
		kvPreset->SetInt("textColorMode", COLOR_MODE_CUSTOM);
		kvPreset->SetInt("textRed", 255);
		kvPreset->SetInt("textGreen", 255);
		kvPreset->SetInt("textBlue", 255);
		kvPreset->SetInt("textAlpha", 255);

		kvPreset->SetInt("barOrientation", FFQuantityItem::ORIENTATION_HORIZONTAL);
		kvPreset->SetInt("barWidth", 80);
		kvPreset->SetInt("barHeight", 10);
		kvPreset->SetInt("barBorderWidth", 1);
		kvPreset->SetInt("itemColumns", 1);
		kvPreset->SetInt("itemMarginHorizontal", 2);
		kvPreset->SetInt("itemMarginVertical", 2);

		KeyValues* kvComponent = new KeyValues("Bar");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("colorMode", COLOR_MODE_FADED);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 100);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("BarBorder");
		kvComponent->SetInt("show", 0);
		kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 155);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("BarBackground");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 0);
		kvComponent->SetInt("green", 0);
		kvComponent->SetInt("blue", 0);
		kvComponent->SetInt("alpha", 120);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("Icon");
		kvComponent->SetInt("show", 0);
		kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 255);
		kvComponent->SetInt("shadow", 1);
		kvComponent->SetInt("size", 5);
		kvComponent->SetInt("anchorPosition", ANCHORPOS_MIDDLECENTER);
		kvComponent->SetInt("alignH", ALIGN_CENTER);
		kvComponent->SetInt("alignV", ALIGN_MIDDLE);
		kvComponent->SetInt("offsetX", 0);
		kvComponent->SetInt("offsetY", 0);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("Label");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 255);
		kvComponent->SetInt("fontVerdana", 0);
		kvComponent->SetInt("shadow", 1);
		kvComponent->SetInt("size", 5);
		kvComponent->SetInt("anchorPosition", ANCHORPOS_MIDDLELEFT);
		kvComponent->SetInt("alignH", ALIGN_LEFT);
		kvComponent->SetInt("alignV", ALIGN_MIDDLE);
		kvComponent->SetInt("offsetX", 1);
		kvComponent->SetInt("offsetY", 0);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("Amount");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("sticky", 1);
		kvComponent->SetInt("colorMode", COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 255);
		kvComponent->SetInt("fontVerdana", 0);
		kvComponent->SetInt("shadow", 1);
		kvComponent->SetInt("size", 5);
		kvComponent->SetInt("anchorPosition", ANCHORPOS_MIDDLERIGHT);
		kvComponent->SetInt("alignH", ALIGN_RIGHT);
		kvComponent->SetInt("alignV", ALIGN_MIDDLE);
		kvComponent->SetInt("offsetX", -1);
		kvComponent->SetInt("offsetY", 0);

		kvPreset->AddSubKey(kvComponent);

		return kvPreset;
	}

	void FFQuantityPanel::TriggerAlert()
	{
		if (m_alertTarget != ALERT_NONE && m_iAlertDuration > 0)
			m_flAlertStart = gpGlobals->curtime;
	}

	void FFQuantityPanel::ClearAlert()
	{
		m_flAlertStart = -1.0f;
	}

	Color FFQuantityPanel::GetAlertColor(Color normalColor, AlertTarget target) const
	{
		if (m_alertTarget != target || m_iAlertDuration <= 0)
			return normalColor;

		float flElapsed;
		if (m_bInPreviewMode && m_bHasAlertOptions)
		{
			// Preview the configured flash every four seconds, including at the main menu.
			flElapsed = static_cast<float>(fmod(Plat_FloatTime(), 3.0));
		}
		else
		{
			if (m_flAlertStart < 0.0f)
				return normalColor;

			flElapsed = gpGlobals->curtime - m_flAlertStart;
		}

		const float flDuration = m_iAlertDuration / 1000.0f;
		if (flElapsed < 0.0f || flElapsed >= flDuration)
			return normalColor;

		const float flProgress = flElapsed / flDuration;
		Color color;
		for (int i = 0; i < 4; ++i)
			color[i] = static_cast<unsigned char>(m_clrAlert[i] + (normalColor[i] - m_clrAlert[i]) * flProgress);
		return color;
	}

	void FFQuantityPanel::PaintBackground()
	{
		const Color normalColor = GetBgColor();
		SetBgColor(GetAlertColor(normalColor, ALERT_PANEL));
		if (m_iPanelBorderWidthScaled > 0)
		{
			const int inset = m_iPanelBorderWidthScaled;
			const int wide = GetWide() - 2 * inset;
			const int tall = GetTall() - 2 * inset;
			if (GetPaintBackgroundType() == CORNERS_ROUND)
				DrawBox(
					inset,
					inset,
					wide,
					tall,
					GetBgColor(),
					1.0f);
			else
			{
				surface()->DrawSetColor(GetBgColor());
				surface()->DrawFilledRect(
					inset,
					inset,
					inset + wide,
					inset + tall);
			}
		}
		else
			BaseClass::PaintBackground();

		SetBgColor(normalColor);
	}

	static int RoundedBorderInset(
		int y,
		int height,
		int radiusX,
		int radiusY)
	{
		if (radiusX <= 0 || radiusY <= 0)
			return 0;

		const float edge = Min(y, height - 1 - y) + 0.5f;
		if (edge >= radiusY)
			return 0;

		const float distance = (radiusY - edge) / radiusY;
		return static_cast<int>(ceilf(radiusX * (1.0f - sqrtf(1.0f - distance * distance))));
	}

	void FFQuantityPanel::DrawPanelBorder()
	{
		if (!m_bShowPanelBorder || m_iPanelBorderWidth <= 0)
			return;

		const int wide = GetWide();
		const int tall = GetTall();
		const int width = m_iPanelBorderWidthScaled;
		if (width <= 0)
			return;

		Color color;
		SetColor(color, GetRgbColor(m_iPanelBorderColorMode, m_clrPanelBorderCustom), m_clrPanelBorderCustom);
		surface()->DrawSetColor(color);
		int radiusX = 0, radiusY = 0;
		if (GetPaintBackgroundType() == CORNERS_ROUND)
		{
			GetCornerTextureSize(radiusX, radiusY);
			radiusX = Min(radiusX + width, wide / 2);
			radiusY = Min(radiusY + width, tall / 2);
		}

		// Draw only the ring, leaving the translucent centre untouched.
		for (int y = 0; y < tall; ++y)
		{
			const int outer = RoundedBorderInset(
				y,
				tall,
				radiusX,
				radiusY);
			if (y < width || y >= tall - width)
			{
				surface()->DrawFilledRect(
					outer,
					y,
					wide - outer,
					y + 1);
				continue;
			}

			const int inner = width + RoundedBorderInset(
				y - width,
				tall - 2 * width,
				Max(0, radiusX - width),
				Max(0, radiusY - width));
			surface()->DrawFilledRect(
				outer,
				y,
				inner,
				y + 1);
			surface()->DrawFilledRect(
				wide - inner,
				y,
				wide - outer,
				y + 1);
		}
	}


	void FFQuantityPanel::AddHeading(KeyValues* kvOptions, const char* pszText, int iGroup)
	{
		KeyValues* kvHeading = new KeyValues("Heading");
		kvHeading->SetString("name", pszText);
		kvHeading->SetString("text", pszText);
		kvHeading->SetInt("group", iGroup);
		kvOptions->AddSubKey(kvHeading);
	}

	void FFQuantityPanel::AddAlertOptions(KeyValues* kvOptions, const char* pszHeadingText, int iGroup)
	{
		AddHeading(kvOptions, "#HudPanel_Heading_Alerts", iGroup);
		KeyValues* kvAlert = new KeyValues("AlertButton");
		kvAlert->SetString("name", "DamageAlert");
		kvAlert->SetString("text", pszHeadingText);
		kvAlert->SetInt("group", iGroup);
		kvOptions->AddSubKey(kvAlert);
	}

	void FFQuantityPanel::AddAlertDefaults(KeyValues* kvValues)
	{
		kvValues->SetInt("AlertTarget", ALERT_HEADER);
		kvValues->SetInt("AlertColorRed", 255);
		kvValues->SetInt("AlertColorGreen", 80);
		kvValues->SetInt("AlertColorBlue", 80);
		kvValues->SetInt("AlertColorAlpha", 255);
		kvValues->SetInt("AlertDuration", 1000);
	}

	void FFQuantityPanel::Paint()
	{
		if (m_bInPreviewMode)
		{
			const float flTime = static_cast<float>(fmod(Plat_FloatTime(), FFQuantityItem::PREVIEW_CYCLE_DURATION));
			for (int i = 0; i < m_darQuantityItems.GetCount(); ++i)
			{
				FFQuantityItem* pItem = m_darQuantityItems[i];
				if (!pItem->IsDisabled())
					pItem->AnimatePreviewAmount(flTime);
			}
		}

		VPROF_BUDGET("FFQuantityPanel::Paint", "QuantityItems");

		BaseClass::Paint();

		if (HasHeaderText())
			DrawText(
				m_wHeaderText,
				m_hfHeaderText,
				GetAlertColor(m_clrHeaderText, ALERT_HEADER),
				m_headerTextPosition,
				m_paintOffset);

		if (HasHeaderIcon())
			DrawText(
				m_wHeaderIcon,
				m_hfHeaderIcon,
				GetAlertColor(m_clrHeaderIcon, ALERT_HEADER),
				m_headerIconPosition,
				m_paintOffset);

		if (HasText() && m_bToggleTextVisible)
			DrawText(
				m_wText,
				m_hfText,
				m_clrText,
				m_textPosition,
				m_paintOffset);
		DrawPanelBorder();
	}

	void FFQuantityPanel::DrawText(
		std::wstring wText,
		HFont font,
		Color color,
		Position position,
		Offset offset)
	{
		surface()->DrawSetTextFont(font);
		surface()->DrawSetTextColor(color);
		surface()->DrawSetTextPos(
			offset.X + position.X,
			offset.Y + position.Y);
		surface()->DrawUnicodeString(
			wText.c_str());
	}

	void FFQuantityPanel::AddPanelToHudOptions(
		const char* szSelfName,
		const char* szSelfText,
		const char* szGroupName,
		const char* szGroupText)
	{
		Q_strncpy(m_szSelfName, szSelfName, 127);
		Q_strncpy(m_szParentName, szGroupName, 127);
		Q_strncpy(m_szSelfText, szSelfText, 127);
		Q_strncpy(m_szParentText, szGroupText, 127);
		m_bAddToHud = true;
	}

	KeyValues* FFQuantityPanel::GetBuildableItemVisibilityOptions()
	{
		KeyValues* kvOptions = GetQuantityVisibilityOptions();
		kvOptions->SetString("2", "#HudPanel_VisibilityBeforeConstruction");
		kvOptions->SetString("3", "#HudPanel_VisibilityIfNotBuilt");
		return kvOptions;
	}

	KeyValues* FFQuantityPanel::GetQuantityVisibilityOptions()
	{
		// These controls store a hidden flag: false shows the item, true hides it.
		KeyValues* kvOptions = new KeyValues("Values");
		kvOptions->SetString("0", "#HudPanel_VisibilityAlways");
		kvOptions->SetString("1", "#HudPanel_VisibilityNever");
		return kvOptions;
	}

	KeyValues* FFQuantityPanel::GetCountDirectionOptions()
	{
		KeyValues* kvOptions = new KeyValues("Values");
		kvOptions->SetString("0", "#HudPanel_Option_CountUp");
		kvOptions->SetString("1", "#HudPanel_Option_CountDown");
		return kvOptions;
	}

	void FFQuantityPanel::AddShowOption(
		KeyValues* kvOptions,
		bool bDefaultValue,
		int iColumn)
	{
		AddBooleanOption(
			kvOptions,
			"ShowPanel",
			"#HudPanel_Option_ShowPanel",
			bDefaultValue,
			iColumn);
	}

	void FFQuantityPanel::AddShowTextOption(
		KeyValues* kvOptions,
		bool bDefaultValue,
		int iColumn)
	{
		AddBooleanOption(
			kvOptions,
			"ShowText",
			"#HudPanel_Option_ShowText",
			bDefaultValue,
			iColumn);
	}

	void FFQuantityPanel::AddHideWhenFullOption(
		KeyValues* kvOptions,
		bool bDefaultValue,
		int iColumn)
	{
		AddBooleanOption(
			kvOptions,
			"HideWhenFull",
			"#HudPanel_Option_HideWhenFull",
			bDefaultValue,
			iColumn);
	}

	void FFQuantityPanel::AddBooleanOption(
		KeyValues* kvMessage,
		const char* pszName,
		const char* pszText,
		const bool defaultValue,
		const int iGroup)
	{
		KeyValues* kv = new KeyValues("Boolean");
		kv->SetInt("group", iGroup);
		kv->SetString("name", pszName);
		kv->SetString("text", pszText);
		if (defaultValue)
			kv->SetInt("defaultValue", 1);

		kvMessage->AddSubKey(kv);
	}

	void FFQuantityPanel::AddNumericOption(
		KeyValues* kvOptions,
		const char* pszName,
		const char* pszText,
		int iDefault,
		int iMinimum,
		int iMaximum,
		int iGroup)
	{
		KeyValues* kvOption = new KeyValues("NumericInput");
		kvOption->SetString("name", pszName);
		kvOption->SetString("text", pszText);
		kvOption->SetInt("defaultValue", iDefault);
		kvOption->SetInt("minimum", iMinimum);
		kvOption->SetInt("maximum", iMaximum);
		kvOption->SetInt("group", iGroup);
		kvOptions->AddSubKey(kvOption);
	}

	void FFQuantityPanel::AddItemOption(
		KeyValues* kvOptions,
		const char* pszItem,
		const char* pszText,
		std::initializer_list<ItemOption> options)
	{
		KeyValues* kvItem = new KeyValues("Item");
		kvItem->SetString("item", pszItem);
		kvItem->SetString("text", pszText);

		for (const ItemOption& option : options)
		{
			KeyValues* kvControl = AddComboOption(
				kvItem,
				option.pszName,
				pszText,
				option.kvValues,
				option.iDefaultValue);

			kvControl->SetInt("column", option.column);
		}

		kvOptions->AddSubKey(kvItem);
	}

	KeyValues* FFQuantityPanel::AddComboOption(
		KeyValues* kvMessage,
		const char* pszName,
		const char* pszText,
		KeyValues* kvOptions,
		const int defaultValue,
		const int iGroup)
	{
		KeyValues* kv = new KeyValues("ComboBox");
		kv->SetInt("group", iGroup);
		kv->SetString("name", pszName);
		kv->SetString("text", pszText);
		if (defaultValue != -1)
		{
			//fix default value one way or another!
			kv->SetInt("defaultValue", defaultValue);
		}

		kv->AddSubKey(kvOptions);
		kvMessage->AddSubKey(kv);

		return kv;
	}

	int FFQuantityPanel::GetColorTeamNumber()
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (pPlayer)
			return pPlayer->GetTeamNumber();

		return TEAM_UNASSIGNED;
	}

	void FFQuantityPanel::SetTeamColor()
	{
		C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
		if (!pPlayer)
			return;

		int iTeamNumber = GetColorTeamNumber();

		const Color& clrTeam
			= g_PR->GetTeamColor(iTeamNumber);

		Color clrTeamBright;
		SetColorByTeam(iTeamNumber, clrTeamBright);

		bool bHasChanged = Change(
			m_clrTeam,
			clrTeam);
		bHasChanged |= Change(m_clrTeamBright, clrTeamBright);

		if (!bHasChanged)
			return;

		for (int i = 0; i < m_darQuantityItems.GetCount(); ++i)
		{
			m_darQuantityItems[i]->SetTeamColor(clrTeam, clrTeamBright);
		}

		if (m_iPanelColorMode == COLOR_MODE_TEAM || m_iPanelColorMode == COLOR_MODE_TEAM_BRIGHT)
		{
			RecalculatePanelColor();
			Color clrPanel;
			SetColor(clrPanel, GetRgbColor(m_iPanelColorMode, m_clrPanelCustom), m_clrPanelCustom);
			SetBgColor(clrPanel);
		}

		if (m_iHeaderIconColorMode == COLOR_MODE_TEAM || m_iHeaderIconColorMode == COLOR_MODE_TEAM_BRIGHT)
			SetColor(m_clrHeaderIcon, GetRgbColor(m_iHeaderIconColorMode, m_clrHeaderIconCustom), m_clrHeaderIconCustom);

		if (m_iHeaderTextColorMode == COLOR_MODE_TEAM || m_iHeaderTextColorMode == COLOR_MODE_TEAM_BRIGHT)
			SetColor(m_clrHeaderText, GetRgbColor(m_iHeaderTextColorMode, m_clrHeaderTextCustom), m_clrHeaderTextCustom);

		if (m_iTextColorMode == COLOR_MODE_TEAM || m_iTextColorMode == COLOR_MODE_TEAM_BRIGHT)
			SetColor(m_clrText, GetRgbColor(m_iTextColorMode, m_clrTextCustom), m_clrTextCustom);
	}

	void FFQuantityPanel::OnTick()
	{
		VPROF_BUDGET("FFQuantityPanel::OnTick", "QuantityItems");

		if (!m_bAddToHudSent
			&& m_bAddToHud
			&& g_AP != NULL
			&& g_AP->IsReady())
		{
			KeyValues* kvGenericOptions
				= new KeyValues("AddQuantityPanel");

			kvGenericOptions->SetString("selfName", m_szSelfName);
			kvGenericOptions->SetString("selfText", m_szSelfText);
			kvGenericOptions->SetString("parentName", m_szParentName);
			kvGenericOptions->SetString("parentText", m_szParentText);
			kvGenericOptions->SetPtr("panel", this);

			KeyValues* kvItemsOptions
				= new KeyValues("Items");
			AddItemStyles(kvItemsOptions);
			kvGenericOptions->AddSubKey(kvItemsOptions);

			KeyValues* kvPanelSpecificOptions
				= new KeyValues("PanelSpecificOptions");
			AddPanelSpecificOptions(kvPanelSpecificOptions);
			kvGenericOptions->AddSubKey(kvPanelSpecificOptions);

			PostMessage(g_AP, kvGenericOptions);

			m_bAddToHudSent = true;
		}

		if (m_bInPreviewMode)
		{
			CHudElement* pHudElement = dynamic_cast<CHudElement*>(this);
			SetVisible(pHudElement && pHudElement->ShouldDraw());
		}

		if (!engine->IsInGame())
			return;

		SetTeamColor();
	}

	KeyValues* FFQuantityPanel::AddItemStyles(
		KeyValues* kvItemStyleList)
	{
		for (int i = 0; i < m_darQuantityItems.Count(); ++i)
		{
			kvItemStyleList->SetString(
				m_darQuantityItems[i]->GetName(),
				"Default");
		}

		return kvItemStyleList;
	}

	void FFQuantityPanel::PerformLayout()
	{
		// Let base do any housekeeping (safe/no-op in many cases)
		BaseClass::PerformLayout();

		if (m_bHeaderIconFontDirty)
			RecalculateHeaderIconFont();

		if (m_bHeaderTextFontDirty)
			RecalculateHeaderTextFont();

		if (m_bTextFontDirty)
			RecalculateTextFont();

		if (m_bItemsPositionDirty)
			RecalculateItemPositions();

		if (m_bHeaderIconPositionDirty)
			RecalculateHeaderIconPosition();

		if (m_bHeaderTextPositionDirty)
			RecalculateHeaderTextPosition();

		if (m_bTextPositionDirty)
			RecalculateTextPosition();

		if (m_bPaintOffsetDirty)
			RecalculatePaintOffsetAndSize();

		if (m_bPositionDirty)
			RecalculatePosition();
	}

	// -------------------------------------------------------------
	// 1) ComputeGridDimensions
	//    - For Horizontal* flows: primaryDim = number of COLUMNS
	//    - For Vertical*   flows: primaryDim = number of ROWS
	// -------------------------------------------------------------
	static inline void ComputeGridDimensions(
		int enabledCount,
		int primaryDim,
		vgui::FFQuantityPanel::Flow flow,
		int& rowCount,
		int& colCount)
	{
		switch (flow)
		{
		case vgui::FFQuantityPanel::Flow::Horizontal:
		case vgui::FFQuantityPanel::Flow::HorizontalReverse:
			colCount = Max(1, Min(primaryDim, enabledCount));      // occupied columns
			rowCount = (enabledCount + colCount - 1) / colCount;    // ceil
			break;

		case vgui::FFQuantityPanel::Flow::Vertical:
		case vgui::FFQuantityPanel::Flow::VerticalReverse:
			rowCount = Max(1, Min(primaryDim, enabledCount));      // occupied rows
			colCount = (enabledCount + rowCount - 1) / rowCount;    // ceil
			break;
		}
	}

	// -------------------------------------------------------------
	// 2) MapIndexToGrid
	//    - Converts a linear item index → (rowIndex, colIndex)
	//    - Uses the precomputed rowCount/colCount and the flow
	// -------------------------------------------------------------
	static inline void MapIndexToGrid(
		int itemIndex,
		int rowCount,
		int colCount,
		vgui::FFQuantityPanel::Flow flow,
		int& rowIndex,
		int& colIndex)
	{
		switch (flow)
		{
		case vgui::FFQuantityPanel::Flow::Horizontal:
		case vgui::FFQuantityPanel::Flow::HorizontalReverse:
			rowIndex = itemIndex / colCount;
			colIndex = itemIndex % colCount;
			if (flow == vgui::FFQuantityPanel::Flow::HorizontalReverse)
				colIndex = (colCount - 1) - colIndex;

			break;

		case vgui::FFQuantityPanel::Flow::Vertical:
		case vgui::FFQuantityPanel::Flow::VerticalReverse:
			colIndex = itemIndex / rowCount;
			rowIndex = itemIndex % rowCount;
			if (flow == vgui::FFQuantityPanel::Flow::VerticalReverse)
				rowIndex = (rowCount - 1) - rowIndex;

			break;
		}
	}

	// -------------------------------------------------------------
	// 3) FFQuantityPanel::RecalculateItemPositions
	//    - Pass 1: measure per-column/row maxima (cell extents + content offsets)
	//    - Build origins
	//    - Pass 2: place items using normalized offsets (position only; no sizing)
	//    - Then header/icon/body and panel frame as you already have
	// -------------------------------------------------------------
	void FFQuantityPanel::RecalculateItemPositions()
	{
		// 0) Collect enabled items (preserving declared order)
		CUtlVector<FFQuantityItem*> enabledItems;
		enabledItems.Purge();

		const int totalItems = m_darQuantityItems.GetCount();
		for (int i = 0; i < totalItems; ++i)
		{
			FFQuantityItem* item = m_darQuantityItems[i];
			if (item && !item->IsDisabled())
				enabledItems.AddToTail(item);
		}

		const int enabledCount = enabledItems.Count();

		if (enabledCount <= 0
			&& Change(m_itemsSizeScaled, Size()))
		{
			m_bHeaderIconPositionDirty = true;
			m_bHeaderTextPositionDirty = true;
			m_bTextPositionDirty = true;
			m_bPaintOffsetDirty = true;
			return;
		}

		// 1) Compute grid dimensions once
		int rowCount = 0, colCount = 0;
		ComputeGridDimensions(
			enabledCount,
			m_iItemColumns,
			m_flow,
			rowCount,
			colCount);

		// Per-column/row maxima
		CUtlVector<int> colMaxWidth;           colMaxWidth.SetSize(colCount);
		CUtlVector<int> colMaxContentOffsetX;  colMaxContentOffsetX.SetSize(colCount);
		CUtlVector<int> rowMaxHeight;          rowMaxHeight.SetSize(rowCount);
		CUtlVector<int> rowMaxContentOffsetY;  rowMaxContentOffsetY.SetSize(rowCount);
		for (int c = 0; c < colCount; ++c) { colMaxWidth[c] = 0; colMaxContentOffsetX[c] = 0; }
		for (int r = 0; r < rowCount; ++r) { rowMaxHeight[r] = 0; rowMaxContentOffsetY[r] = 0; }

		// PASS 1 — measure each item: width/height + contentOffsetX/Y
		for (int itemIndex = 0; itemIndex < enabledCount; ++itemIndex)
		{
			int rowIndex = 0, colIndex = 0;
			MapIndexToGrid(
				itemIndex,
				rowCount,
				colCount,
				m_flow,
				rowIndex,
				colIndex);

			Size itemSize;
			Offset itemContentOffset;

			enabledItems[itemIndex]->GetContentMetrics(
				itemSize,
				itemContentOffset);

			if (itemSize.Width > colMaxWidth[colIndex])
				colMaxWidth[colIndex] = itemSize.Width;

			if (itemContentOffset.X > colMaxContentOffsetX[colIndex])
				colMaxContentOffsetX[colIndex] = itemContentOffset.X;

			if (itemSize.Height > rowMaxHeight[rowIndex])
				rowMaxHeight[rowIndex] = itemSize.Height;

			if (itemContentOffset.Y > rowMaxContentOffsetY[rowIndex])
				rowMaxContentOffsetY[rowIndex] = itemContentOffset.Y;
		}

		// 2) Build column/row origins (prefix sums + margins)
		CUtlVector<int> colOriginX; colOriginX.SetSize(colCount);
		CUtlVector<int> rowOriginY; rowOriginY.SetSize(rowCount);

		int runningX = 0;
		for (int col = 0; col < colCount; ++col)
		{
			if (col > 0) runningX += m_iItemMarginHorizontal * m_scale.X;
			colOriginX[col] = runningX;
			runningX += colMaxWidth[col];
		}

		// Use the same scale for both margins so equal values produce equal pixel gaps.
		int runningY = 0;
		for (int row = 0; row < rowCount; ++row)
		{
			if (row > 0) runningY += m_iItemMarginVertical * m_scale.X;
			rowOriginY[row] = runningY;
			runningY += rowMaxHeight[row];
		}

		const int itemsAreaWidth = runningX; // includes internal margins
		const int itemsAreaHeight = runningY;

		// 3) PASS 2 — place each item using normalized offsets (position only)
		m_itemLocalPositions.SetSize(enabledCount);

		for (int itemIndex = 0; itemIndex < enabledCount; ++itemIndex)
		{
			int rowIndex = 0, colIndex = 0;
			MapIndexToGrid(
				itemIndex,
				rowCount,
				colCount,
				m_flow,
				rowIndex,
				colIndex);

			FFQuantityItem* item = enabledItems[itemIndex];

			Size itemSize;
			Offset itemContentOffset;

			item->GetContentMetrics(
				itemSize,
				itemContentOffset);

			// Normalize: align all items in this column/row using the max offsets
			const int localX
				= colOriginX[colIndex]
					+ (colMaxContentOffsetX[colIndex] - itemContentOffset.X);
			const int localY
				= rowOriginY[rowIndex]
					+ (rowMaxContentOffsetY[rowIndex] - itemContentOffset.Y);

			m_itemLocalPositions[itemIndex]
				= Position(localX, localY);

			Position currentPosition;
			item->GetPos(currentPosition.X, currentPosition.Y);

			const Position desiredPosition(
				localX + m_paintOffset.X,
				localY + m_paintOffset.Y);

			// Items can move without changing the overall area size. Set this flag
			// so RecalculatePaintOffsetAndSize() calls SetPos() with their new positions.
			if (currentPosition != desiredPosition)
			{
				m_bPaintOffsetDirty = true;
			}
		}

		// 4) Cache items area; then headers / outer panel frame
		if (Change(
			m_itemsSizeScaled,
			Size(itemsAreaWidth, itemsAreaHeight)))
		{
			m_bHeaderIconPositionDirty = true;
			m_bHeaderTextPositionDirty = true;
			m_bTextPositionDirty = true;
			m_bPaintOffsetDirty = true;
		}

		m_bItemsPositionDirty = false;
	}

	void FFQuantityPanel::RecalculatePaintOffsetAndSize()
	{
		Bounds bounds = {
			0,
			0,
			static_cast<float>(m_itemsSizeScaled.Width),
			static_cast<float>(m_itemsSizeScaled.Height)
		};

		auto expand = [&](Position p, Size s) {
			if (p.X < bounds.X0)
				bounds.X0 = static_cast<float>(p.X);

			if (p.Y < bounds.Y0)
				bounds.Y0 = static_cast<float>(p.Y);

			if (p.X + s.Width > bounds.X1)
				bounds.X1 = static_cast<float>(p.X + s.Width);

			if (p.Y + s.Height > bounds.Y1)
				bounds.Y1 = static_cast<float>(p.Y + s.Height);
			};

		if (HasHeaderText())
			expand(m_headerTextPosition, m_headerTextSizeScaled);

		if (HasHeaderIcon())
			expand(m_headerIconPosition, m_headerIconSizeScaled);

		if (HasText())
			expand(m_textPosition, m_textSizeScaled);

		bounds.X0 -= m_iPanelMargin;
		bounds.Y0 -= m_iPanelMargin;
		bounds.X1 += m_iPanelMargin;
		bounds.Y1 += m_iPanelMargin;

		m_iPanelBorderWidthScaled = m_bShowPanelBorder && m_iPanelBorderWidth > 0
			? Max(1, static_cast<int>(m_iPanelBorderWidth * m_scale.X)) : 0;
		bounds.X0 -= m_iPanelBorderWidthScaled;
		bounds.Y0 -= m_iPanelBorderWidthScaled;
		bounds.X1 += m_iPanelBorderWidthScaled;
		bounds.Y1 += m_iPanelBorderWidthScaled;

		int offsetX = static_cast<int>(-bounds.X0);
		int offsetY = static_cast<int>(-bounds.Y0);
		m_paintOffset = Offset(offsetX, offsetY);

		int width = static_cast<int>(bounds.X1 - bounds.X0);
		int height = static_cast<int>(bounds.Y1 - bounds.Y0);
		SetSize(width, height);

		// Place items after paint offset is recalculated
		CUtlVector<FFQuantityItem*> enabledItems;
		enabledItems.Purge();
		const int totalItems = m_darQuantityItems.GetCount();
		for (int i = 0; i < totalItems; ++i)
		{
			FFQuantityItem* item = m_darQuantityItems[i];
			if (item && !item->IsDisabled())
				enabledItems.AddToTail(item);
		}

		const int enabledCount = enabledItems.Count();
		for (int itemIndex = 0; itemIndex < enabledCount; ++itemIndex)
		{
			FFQuantityItem* item = enabledItems[itemIndex];
			item->SetPos(
				m_itemLocalPositions[itemIndex].X + m_paintOffset.X,
				m_itemLocalPositions[itemIndex].Y + m_paintOffset.Y);
		}

		m_bPaintOffsetDirty = false;
		m_bPositionDirty = true;
	}

	void FFQuantityPanel::RecalculateHeaderIconPosition()
	{
		m_bHeaderIconPositionDirty = false;

		Size sizeScaled = Size();
		surface()->GetTextSize(
			m_hfHeaderIcon,
			m_wHeaderIcon.c_str(),
			sizeScaled.Width,
			sizeScaled.Height);

		Position position
			= CalculatePosition(
				m_headerIconAnchorPosition,
				m_itemsSizeScaled,
				sizeScaled,
				m_iHeaderIconAlignHoriz,
				m_iHeaderIconAlignVert,
				m_headerIconPositionOffset.Scaled(m_scale));

		bool textSizeChanged
			= Change(
				m_headerIconSizeScaled,
				sizeScaled);

		bool positionChanged
			= Change(
				m_headerIconPosition,
				position);

		if (textSizeChanged || positionChanged)
			m_bPaintOffsetDirty = true;
	}

	void FFQuantityPanel::RecalculateHeaderTextPosition()
	{
		m_bHeaderTextPositionDirty = false;

		Size sizeScaled = Size();
		surface()->GetTextSize(
			m_hfHeaderText,
			m_wHeaderText.c_str(),
			sizeScaled.Width,
			sizeScaled.Height);

		Position position
			= CalculatePosition(
				m_headerTextAnchorPosition,
				m_itemsSizeScaled,
				sizeScaled,
				m_iHeaderTextAlignHoriz,
				m_iHeaderTextAlignVert,
				m_headerTextPositionOffset.Scaled(m_scale));

		bool textSizeChanged
			= Change(
				m_headerTextSizeScaled,
				sizeScaled);

		bool positionChanged
			= Change(
				m_headerTextPosition,
				position);

		if (textSizeChanged || positionChanged)
			m_bPaintOffsetDirty = true;
	}

	void FFQuantityPanel::RecalculateTextPosition()
	{
		m_bTextPositionDirty = false;

		Size sizeScaled = Size();
		surface()->GetTextSize(
			m_hfText,
			m_wText.c_str(),
			sizeScaled.Width,
			sizeScaled.Height);

		Position position
			= CalculatePosition(
				m_textAnchorPosition,
				m_itemsSizeScaled,
				sizeScaled,
				m_iTextAlignHoriz,
				m_iTextAlignVert,
				m_textPositionOffset.Scaled(m_scale));

		bool textSizeChanged
			= Change(
				m_textSizeScaled,
				sizeScaled);

		bool positionChanged
			= Change(
				m_textPosition,
				position);

		if (textSizeChanged || positionChanged)
			m_bPaintOffsetDirty = true;
	}

	void FFQuantityPanel::SetHeaderIconChar(
		const char cHeaderIcon)
	{
		std::wstring wHeaderIcon = {
			static_cast<wchar_t>(cHeaderIcon),
				L'\0'
		};

		if (m_wHeaderIcon == wHeaderIcon)
			return;

		m_wHeaderIcon = wHeaderIcon;
		m_bHeaderIconPositionDirty = true;
		InvalidateLayout();
	}

	void FFQuantityPanel::SetHeaderText(
		const std::wstring wHeaderText)
	{
		if (m_wHeaderText == wHeaderText)
			return;

		m_wHeaderText = wHeaderText;
		m_bHeaderTextPositionDirty = true;
		InvalidateLayout();
	}

	void FFQuantityPanel::SetText(
		const std::wstring wText)
	{
		if (m_wText == wText)
			return;

		m_wText = wText;
		m_bTextPositionDirty = true;
		InvalidateLayout();
	}

	void FFQuantityPanel::SetUseToggleText(
		bool bUseToggleText)
	{
		m_bUseToggleText = bUseToggleText;
		m_bPaintOffsetDirty = true;
		InvalidateLayout();
	}

	void FFQuantityPanel::SetToggleTextVisible(
		bool bIsVisible)
	{
		m_bToggleTextVisible = bIsVisible;
	}

	void FFQuantityPanel::ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData)
	{
		KeyValues* kvAlertValues = kvStyleData->FindKey("PanelSpecificValues", true);
		KeyValues* kvAlertDefaults = kvDefaultStyleData->FindKey("PanelSpecificValues", true);
		int iAlertTarget = GetInt("AlertTarget", kvAlertValues, kvAlertDefaults);
		if (iAlertTarget != -1)
		{
			m_alertTarget = static_cast<AlertTarget>(iAlertTarget);
			m_bHasAlertOptions = true;
		}

		int iAlertDuration = GetInt("AlertDuration", kvAlertValues, kvAlertDefaults);
		if (iAlertDuration != -1)
			m_iAlertDuration = iAlertDuration;

		const char* colorKeys[] = { "AlertColorRed", "AlertColorGreen", "AlertColorBlue", "AlertColorAlpha" };
		for (int i = 0; i < 4; ++i)
		{
			int iValue = GetInt(colorKeys[i], kvAlertValues, kvAlertDefaults);
			if (iValue != -1)
				m_clrAlert[i] = static_cast<unsigned char>(iValue);
		}

		int iPreviewMode = GetInt("previewMode", kvStyleData, kvDefaultStyleData);
		if (iPreviewMode != -1)
		{
			SetPreviewMode(iPreviewMode == 1);
		}

		int iX = GetInt("x", kvStyleData, kvDefaultStyleData);
		int iY = GetInt("y", kvStyleData, kvDefaultStyleData);
		if ((iX != -1 && iY != -1)
			&& Change(m_position, Position(iX, iY)))
		{
			m_bPositionDirty = true;
		}

		int iHorizontalAlign = GetInt("alignH", kvStyleData, kvDefaultStyleData);
		if (iHorizontalAlign != -1
			&& Change(m_iHorizontalAlign, iHorizontalAlign))
		{
			m_bPositionDirty = true;
		}

		int iVerticalAlign = GetInt("alignV", kvStyleData, kvDefaultStyleData);
		if (iVerticalAlign != -1
			&& Change(m_iVerticalAlign, iVerticalAlign))
		{
			m_bPositionDirty = true;
		}

		int iPanelMargin = GetInt("panelMargin", kvStyleData, kvDefaultStyleData);
		if (iPanelMargin != -1
			&& Change(m_iPanelMargin, iPanelMargin))
		{
			m_bPaintOffsetDirty = true;
		}

		int iColumns = GetInt("itemColumns", kvStyleData, kvDefaultStyleData);
		if (iColumns != -1
			&& Change(m_iItemColumns, iColumns))
		{
			m_bItemsPositionDirty = true;
		}

		int iPanelType = GetInt("panelType", kvStyleData, kvDefaultStyleData);
		if (iPanelType != -1)
		{
			SetPaintBackgroundType(iPanelType);
		}


		int iShowPanelBorder = GetInt("showPanelBorder", kvStyleData, kvDefaultStyleData);
		if (iShowPanelBorder != -1
			&& Change(m_bShowPanelBorder, iShowPanelBorder == 1))
			m_bPaintOffsetDirty = true;
		int iPanelBorderWidth = GetInt("panelBorderWidth", kvStyleData, kvDefaultStyleData);
		if (iPanelBorderWidth != -1
			&& Change(m_iPanelBorderWidth, iPanelBorderWidth))
			m_bPaintOffsetDirty = true;
		int iPanelBorderColorMode = GetInt("panelBorderColorMode", kvStyleData, kvDefaultStyleData);
		if (iPanelBorderColorMode != -1)
			m_iPanelBorderColorMode = iPanelBorderColorMode;

		int iBorderRed = GetInt("panelBorderRed", kvStyleData, kvDefaultStyleData);
		int iBorderGreen = GetInt("panelBorderGreen", kvStyleData, kvDefaultStyleData);
		int iBorderBlue = GetInt("panelBorderBlue", kvStyleData, kvDefaultStyleData);
		int iBorderAlpha = GetInt("panelBorderAlpha", kvStyleData, kvDefaultStyleData);
		if (iBorderRed != -1 && iBorderGreen != -1 && iBorderBlue != -1 && iBorderAlpha != -1)
			m_clrPanelBorderCustom.SetColor(
				iBorderRed,
				iBorderGreen,
				iBorderBlue,
				iBorderAlpha);

		int iPanelColorMode = GetInt("panelColorMode", kvStyleData, kvDefaultStyleData);
		if (iPanelColorMode != -1
			&& Change(m_iPanelColorMode, iPanelColorMode))
		{
			RecalculatePanelColor();
		}

		int iPanelRed = GetInt("panelRed", kvStyleData, kvDefaultStyleData);
		int iPanelGreen = GetInt("panelGreen", kvStyleData, kvDefaultStyleData);
		int iPanelBlue = GetInt("panelBlue", kvStyleData, kvDefaultStyleData);
		int iPanelAlpha = GetInt("panelAlpha", kvStyleData, kvDefaultStyleData);
		if (iPanelRed != -1
			&& iPanelGreen != -1
			&& iPanelBlue != -1
			&& iPanelAlpha != -1)
		{
			SetCustomPanelColor(
				iPanelRed,
				iPanelGreen,
				iPanelBlue,
				iPanelAlpha);
		}

		int iShowHeaderIcon = GetInt("showHeaderIcon", kvStyleData, kvDefaultStyleData);
		int iHeaderIconShadow = GetInt("headerIconShadow", kvStyleData, kvDefaultStyleData);
		int iHeaderIconSize = GetInt("headerIconSize", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAnchorPosition = GetInt("headerIconAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlignHoriz = GetInt("headerIconAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlignVert = GetInt("headerIconAlignVert", kvStyleData, kvDefaultStyleData);
		int iHeaderIconX = GetInt(
			"headerIconX",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		int iHeaderIconY = GetInt(
			"headerIconY",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		int iHeaderIconColorMode = GetInt("headerIconColorMode", kvStyleData, kvDefaultStyleData);
		int iHeaderIconRed = GetInt("headerIconRed", kvStyleData, kvDefaultStyleData);
		int iHeaderIconGreen = GetInt("headerIconGreen", kvStyleData, kvDefaultStyleData);
		int iHeaderIconBlue = GetInt("headerIconBlue", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlpha = GetInt("headerIconAlpha", kvStyleData, kvDefaultStyleData);

		if (iShowHeaderIcon != -1
			&& Change(m_bShowHeaderIcon, iShowHeaderIcon == 1))
		{
			m_bPaintOffsetDirty = true;
		}

		if (iHeaderIconShadow != -1
			&& Change(m_bHeaderIconShadow, iHeaderIconShadow == 1))
		{
			m_bHeaderIconFontDirty = true;
		}

		if (iHeaderIconSize != -1
			&& Change(m_iHeaderIconSize, iHeaderIconSize))
		{
			m_bHeaderIconFontDirty = true;
		}

		if (iHeaderIconAnchorPosition != -1
			&& Change(
				m_headerIconAnchorPosition,
				static_cast<AnchorPosition>(iHeaderIconAnchorPosition)))
		{
			m_bHeaderIconPositionDirty = true;
		}

		if (iHeaderIconAlignHoriz != -1
			&& Change(m_iHeaderIconAlignHoriz, iHeaderIconAlignHoriz))
		{
			m_bHeaderIconPositionDirty = true;
		}

		if (iHeaderIconAlignVert != -1
			&& Change(m_iHeaderIconAlignVert, iHeaderIconAlignVert))
		{
			m_bHeaderIconPositionDirty = true;
		}

		if ((iHeaderIconX != -9999 && iHeaderIconY != -9999)
			&& Change(
				m_headerIconPositionOffset,
				Offset(iHeaderIconX, iHeaderIconY)))
		{
			m_bHeaderIconPositionDirty = true;
		}

		if (iHeaderIconColorMode != -1
			&& Change(m_iHeaderIconColorMode, iHeaderIconColorMode))
		{
			RecalculateHeaderIconColor();
		}

		if (iHeaderIconRed != -1
			&& iHeaderIconGreen != -1
			&& iHeaderIconBlue != -1
			&& iHeaderIconAlpha != -1)
		{
			SetCustomHeaderIconColor(
				iHeaderIconRed,
				iHeaderIconGreen,
				iHeaderIconBlue,
				iHeaderIconAlpha);
		}

		int iShowHeaderText = GetInt("showHeaderText", kvStyleData, kvDefaultStyleData);
		int iHeaderTextShadow = GetInt("headerTextShadow", kvStyleData, kvDefaultStyleData);
		int iHeaderTextFontVerdana = GetInt("headerTextFontVerdana", kvStyleData, kvDefaultStyleData);
		int iHeaderTextSize = GetInt("headerTextSize", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAnchorPosition = GetInt("headerTextAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlignHoriz = GetInt("headerTextAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlignVert = GetInt("headerTextAlignVert", kvStyleData, kvDefaultStyleData);
		int iHeaderTextX = GetInt(
			"headerTextX",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		int iHeaderTextY = GetInt(
			"headerTextY",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		int iHeaderTextColorMode = GetInt("headerTextColorMode", kvStyleData, kvDefaultStyleData);
		int iHeaderTextRed = GetInt("headerTextRed", kvStyleData, kvDefaultStyleData);
		int iHeaderTextGreen = GetInt("headerTextGreen", kvStyleData, kvDefaultStyleData);
		int iHeaderTextBlue = GetInt("headerTextBlue", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlpha = GetInt("headerTextAlpha", kvStyleData, kvDefaultStyleData);

		if (iShowHeaderText != -1
			&& Change(m_bShowHeaderText, iShowHeaderText == 1))
		{
			m_bPaintOffsetDirty = true;
		}

		if (iHeaderTextFontVerdana != -1
			&& Change(m_bHeaderTextFontVerdana, iHeaderTextFontVerdana == 1))
		{
			m_bHeaderTextFontDirty = true;
		}

		if (iHeaderTextShadow != -1
			&& Change(m_bHeaderTextShadow, iHeaderTextShadow == 1))
		{
			m_bHeaderTextFontDirty = true;
		}

		if (iHeaderTextSize != -1
			&& Change(m_iHeaderTextSize, iHeaderTextSize))
		{
			m_bHeaderTextFontDirty = true;
		}

		if (iHeaderTextAnchorPosition != -1
			&& Change(
				m_headerTextAnchorPosition,
				static_cast<AnchorPosition>(iHeaderTextAnchorPosition)))
		{
			m_bHeaderTextPositionDirty = true;
		}

		if (iHeaderTextAlignHoriz != -1
			&& Change(m_iHeaderTextAlignHoriz, iHeaderTextAlignHoriz))
		{
			m_bHeaderTextPositionDirty = true;
		}

		if (iHeaderTextAlignVert != -1
			&& Change(m_iHeaderTextAlignVert, iHeaderTextAlignVert))
		{
			m_bHeaderTextPositionDirty = true;
		}

		if ((iHeaderTextX != -9999 && iHeaderTextY != -9999)
			&& Change(
				m_headerTextPositionOffset,
				Offset(iHeaderTextX, iHeaderTextY)))
		{
			m_bHeaderTextPositionDirty = true;
		}

		if (iHeaderTextColorMode != -1
			&& Change(m_iHeaderTextColorMode, iHeaderTextColorMode))
		{
			RecalculateHeaderTextColor();
		}

		if (iHeaderTextRed != -1
			&& iHeaderTextGreen != -1
			&& iHeaderTextBlue != -1
			&& iHeaderTextAlpha != -1)
		{
			SetCustomHeaderTextColor(
				iHeaderTextRed,
				iHeaderTextGreen,
				iHeaderTextBlue,
				iHeaderTextAlpha);
		}

		int iShowText = GetInt("showText", kvStyleData, kvDefaultStyleData);
		int iTextShadow = GetInt("textShadow", kvStyleData, kvDefaultStyleData);
		int iTextFontVerdana = GetInt("textFontVerdana", kvStyleData, kvDefaultStyleData);
		int iTextSize = GetInt("textSize", kvStyleData, kvDefaultStyleData);
		int iTextAnchorPosition = GetInt("textAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iTextAlignHoriz = GetInt("textAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iTextAlignVert = GetInt("textAlignVert", kvStyleData, kvDefaultStyleData);
		int iTextX = GetInt(
			"textX",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		int iTextY = GetInt(
			"textY",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		int iTextColorMode = GetInt("textColorMode", kvStyleData, kvDefaultStyleData);
		int iTextRed = GetInt("textRed", kvStyleData, kvDefaultStyleData);
		int iTextGreen = GetInt("textGreen", kvStyleData, kvDefaultStyleData);
		int iTextBlue = GetInt("textBlue", kvStyleData, kvDefaultStyleData);
		int iTextAlpha = GetInt("textAlpha", kvStyleData, kvDefaultStyleData);

		if (iTextFontVerdana != -1
			&& Change(m_bTextFontVerdana, iTextFontVerdana == 1))
		{
			m_bTextFontDirty = true;
		}

		if (iShowText != -1
			&& Change(m_bShowText, iShowText == 1))
		{
			m_bPaintOffsetDirty = true;
		}

		if (iTextShadow != -1
			&& Change(m_bTextShadow, iTextShadow == 1))
		{
			m_bTextFontDirty = true;
		}

		if (iTextSize != -1
			&& Change(m_iTextSize, iTextSize))
		{
			m_bTextFontDirty = true;
		}

		if (iTextAnchorPosition != -1
			&& Change(
				m_textAnchorPosition,
				static_cast<AnchorPosition>(iTextAnchorPosition)))
		{
			m_bTextPositionDirty = true;
		}

		if (iTextAlignHoriz != -1
			&& Change(m_iTextAlignHoriz, iTextAlignHoriz))
		{
			m_bTextPositionDirty = true;
		}

		if (iTextAlignVert != -1
			&& Change(m_iTextAlignVert, iTextAlignVert))
		{
			m_bTextPositionDirty = true;
		}

		if ((iTextX != -9999 && iTextY != -9999)
			&& Change(m_textPositionOffset, Offset(iTextX, iTextY)))
		{
			m_bTextPositionDirty = true;
		}

		if (iTextColorMode != -1
			&& Change(m_iTextColorMode, iTextColorMode))
		{
			RecalculateTextColor();
		}

		if (iTextRed != -1 && iTextGreen != -1 && iTextBlue != -1 && iTextAlpha != -1)
		{
			SetCustomTextColor(
				iTextRed,
				iTextGreen,
				iTextBlue,
				iTextAlpha);
		}

		int iShowPanel = GetInt("showPanel", kvStyleData, kvDefaultStyleData);
		if (iShowPanel != -1)
		{
			SetPaintBackgroundEnabled(iShowPanel == 1);
		}

		int iItemMarginHorizontal = GetInt("itemMarginHorizontal", kvStyleData, kvDefaultStyleData);
		if (iItemMarginHorizontal != -1
			&& Change(m_iItemMarginHorizontal, iItemMarginHorizontal))
		{
			m_bItemsPositionDirty = true;
		}

		int iItemMarginVertical = GetInt("itemMarginVertical", kvStyleData, kvDefaultStyleData);
		if (iItemMarginVertical != -1
			&& Change(m_iItemMarginVertical, iItemMarginVertical))
		{
			m_bItemsPositionDirty = true;
		}

		for (int i = 0; i < m_darQuantityItems.GetCount(); ++i)
		{
			if (m_darQuantityItems[i]->ApplyStyleData(
				kvStyleData,
				kvDefaultStyleData))
			{
				m_bItemsPositionDirty = true;
			}
		}

		if (m_bHeaderIconFontDirty
			|| m_bHeaderTextFontDirty
			|| m_bTextFontDirty
			|| m_bHeaderIconPositionDirty
			|| m_bHeaderTextPositionDirty
			|| m_bTextPositionDirty
			|| m_bItemsPositionDirty
			|| m_bPaintOffsetDirty
			|| m_bPositionDirty)
		{
			InvalidateLayout(true);
		}
	}

	void FFQuantityPanel::RecalculateHeaderIconFont()
	{
		m_bHeaderIconFontDirty = false;

		bool bHasChanged
			= Change(
				m_hfHeaderIcon,
				GetFont(
					m_hfHeaderIconFamily,
					m_iHeaderIconSize,
					m_bHeaderIconShadow,
					QUANTITYPANELICONSIZES));

		if (bHasChanged)
			m_bHeaderIconPositionDirty = true;
	}

	void FFQuantityPanel::RecalculateHeaderTextFont()
	{
		m_bHeaderTextFontDirty = false;

		bool bHasChanged
			= Change(
				m_hfHeaderText,
				GetFont(
					m_bHeaderTextFontVerdana ? m_hfVerdanaTextFamily : m_hfHeaderTextFamily,
					m_iHeaderTextSize,
					m_bHeaderTextShadow,
					QUANTITYPANELTEXTSIZES));

		if (bHasChanged)
			m_bHeaderTextPositionDirty = true;
	}

	void FFQuantityPanel::RecalculateTextFont()
	{
		m_bTextFontDirty = false;

		bool bHasChanged
			= Change(
				m_hfText,
				GetFont(
					m_bTextFontVerdana ? m_hfVerdanaTextFamily : m_hfTextFamily,
					m_iTextSize,
					m_bTextShadow,
					QUANTITYPANELTEXTSIZES));

		if (bHasChanged)
			m_bTextPositionDirty = true;
	}

	void FFQuantityPanel::RecalculatePanelColor()
	{
		Color clrPanel;
		RecalculateColor(clrPanel, m_iPanelColorMode, m_clrPanelCustom);
		SetBgColor(clrPanel);
	}

	void FFQuantityPanel::RecalculateHeaderIconColor()
	{

		RecalculateColor(m_clrHeaderIcon, m_iHeaderIconColorMode, m_clrHeaderIconCustom);
	}

	void FFQuantityPanel::RecalculateHeaderTextColor()
	{
		RecalculateColor(m_clrHeaderText, m_iHeaderTextColorMode, m_clrHeaderTextCustom);
	}

	void FFQuantityPanel::RecalculateTextColor()
	{
		RecalculateColor(m_clrText, m_iTextColorMode, m_clrTextCustom);
	}

	bool FFQuantityPanel::SetCustomPanelColor(
		int iRed,
		int iGreen,
		int iBlue,
		int iAlpha)
	{
		Color clrCustom = Color(
			iRed,
			iGreen,
			iBlue,
			iAlpha);
		bool bHasChanged = Change(m_clrPanelCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculatePanelColor();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomHeaderIconColor(
		int iRed,
		int iGreen,
		int iBlue,
		int iAlpha)
	{
		Color clrCustom = Color(
			iRed,
			iGreen,
			iBlue,
			iAlpha);
		bool bHasChanged = Change(m_clrHeaderIconCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor(m_clrHeaderIcon, m_iHeaderIconColorMode, m_clrHeaderIconCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomHeaderTextColor(
		int iRed,
		int iGreen,
		int iBlue,
		int iAlpha)
	{
		Color clrCustom = Color(
			iRed,
			iGreen,
			iBlue,
			iAlpha);
		bool bHasChanged = Change(m_clrHeaderTextCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor(m_clrHeaderText, m_iHeaderTextColorMode, m_clrHeaderTextCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomTextColor(
		int iRed,
		int iGreen,
		int iBlue,
		int iAlpha)
	{
		Color clrCustom = Color(
			iRed,
			iGreen,
			iBlue,
			iAlpha);
		bool bHasChanged = Change(m_clrTextCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor(m_clrText, m_iTextColorMode, m_clrTextCustom);
		}

		return bHasChanged;
	}

	void FFQuantityPanel::RecalculateColor(Color& clr, int iColorMode, Color& clrCustom)
	{
		Color rgbColor = GetRgbColor(iColorMode, clrCustom);

		SetColor(clr, rgbColor, clrCustom);
	}

	Color& FFQuantityPanel::GetRgbColor(int iColorMode, Color& clrCustom)
	{
		switch (iColorMode)
		{
		case COLOR_MODE_TEAM_BRIGHT:
			return m_clrTeamBright;

		case COLOR_MODE_TEAM:
			return m_clrTeam;

		case COLOR_MODE_CUSTOM:
		default:
			return clrCustom;
		}
	}

	void FFQuantityPanel::SetColor(Color& clr, Color& clrRgb, Color& clrAlpha)
	{
		clr.SetColor(
			clrRgb.r(),
			clrRgb.g(),
			clrRgb.b(),
			clrAlpha.a());
	}

	int FFQuantityPanel::GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData,
		int iDefaultValue)
	{
		return FFQuantityHelper::GetInt(
			keyName,
			kvStyleData,
			kvDefaultStyleData,
			iDefaultValue);
	}

	std::optional<AmountFormat> FFQuantityPanel::GetAmountFormat(
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
			? std::optional<AmountFormat>()
			: static_cast<AmountFormat>(value);
	}

	void FFQuantityPanel::OnStyleDataReceived(KeyValues* kvStyleData)
	{
		KeyValues* kvDefaultStyleData = GetDefaultStyleData();
		ApplyStyleData(
			kvStyleData,
			kvDefaultStyleData);
		kvDefaultStyleData->deleteThis();
	}

	void FFQuantityPanel::OnPresetPreviewDataRecieved(KeyValues* kvPresetPreviewData)
	{
		ApplyStyleData(
			kvPresetPreviewData,
			s_kvNoData);
	}

	KeyValues* FFQuantityPanel::GetDefaultPanelSpecificValues()
	{
		KeyValues* kvStyle = GetDefaultStyleData();
		KeyValues* kvValues = kvStyle->FindKey("PanelSpecificValues");
		KeyValues* kvDefaults = new KeyValues("PanelSpecificValues");
		if (kvValues)
			kvValues->CopySubkeys(kvDefaults);

		kvStyle->deleteThis();
		return kvDefaults;
	}

	void FFQuantityPanel::OnDefaultStyleDataRequested(KeyValues* data)
	{
		KeyValues* kvDefaults = GetDefaultStyleData();
		g_AP->CreatePresetFromPanelDefault(kvDefaults, this);
		kvDefaults->deleteThis();
	}

	FFQuantityItem* FFQuantityPanel::AddItem(
		const char* pElementName)
	{
		FFQuantityItem* newQBar
			= new FFQuantityItem(
				this,
				pElementName);

		// Items added after the team was set need its current colour too.
		newQBar->SetTeamColor(m_clrTeam, m_clrTeamBright);

		m_darQuantityItems
			.AddElement(newQBar);

		return newQBar;
	}

	void FFQuantityPanel::InvalidateItemLayout()
	{
		m_bItemsPositionDirty = true;
		InvalidateLayout(true);
	}

	void FFQuantityPanel::HideItem(FFQuantityItem* qBar)
	{
		qBar->SetVisible(false);
	}

	void FFQuantityPanel::ShowItem(FFQuantityItem* qBar)
	{
		qBar->SetVisible(true);
	}

	void FFQuantityPanel::DisableItem(FFQuantityItem* qBar)
	{
		if (qBar->IsDisabled())
			return;

		qBar->SetDisabled(true);

		m_bItemsPositionDirty = true;
		InvalidateLayout();
	}

	void FFQuantityPanel::EnableItem(FFQuantityItem* qBar)
	{
		if (!qBar->IsDisabled())
			return;

		qBar->SetDisabled(false);

		m_bItemsPositionDirty = true;
		InvalidateLayout();
	}

	void FFQuantityPanel::SetPreviewMode(bool bInPreviewMode)
	{
		if (m_bInPreviewMode && !bInPreviewMode)
		{
			for (int i = 0; i < m_darQuantityItems.GetCount(); ++i)
				m_darQuantityItems[i]->RestorePreviewAmount();
		}

		m_bInPreviewMode = bInPreviewMode;

		if (m_bInPreviewMode)
		{
			SetParent(enginevgui->GetPanel(PANEL_ROOT));

			// Draw above the game UI without taking mouse or keyboard input.
			surface()->CreatePopup(
				GetVPanel(),
				false,
				false,
				false,
				false,
				false);
			ipanel()->SetTopmostPopup(GetVPanel(), true);
		}
		else
		{
			ipanel()->SetTopmostPopup(GetVPanel(), false);
			ipanel()->SetPopup(GetVPanel(), false);

			SetParent(g_pClientMode->GetViewport());
			if (!engine->IsInGame())
				SetVisible(false);
		}
	}

	bool FFQuantityPanel::IsInPreviewMode()
	{
		return m_bInPreviewMode;
	}

	void FFQuantityPanel::OnSizeChanged(
		int wide,
		int tall)
	{
		Panel::OnSizeChanged(wide, tall);

		m_bPositionDirty = true;

		InvalidateLayout();
	}

	void FFQuantityPanel::OnScreenSizeChanged(
		int iOldWide,
		int iOldTall)
	{
		Panel::OnScreenSizeChanged(iOldWide, iOldTall);

		int iScreenWide, iScreenTall;

		surface()
			->GetScreenSize(
				iScreenWide,
				iScreenTall);

		Scale scale(
			iScreenWide / 640.0f,
			iScreenTall / 480.0f);

		m_scale = scale;

		m_bHeaderIconFontDirty = true;
		m_bHeaderTextFontDirty = true;
		m_bTextFontDirty = true;

		m_bItemsPositionDirty = true;
		m_bHeaderIconPositionDirty = true;
		m_bHeaderTextPositionDirty = true;
		m_bTextPositionDirty = true;

		m_bPaintOffsetDirty = true;
		m_bPositionDirty = true;

		InvalidateLayout();
	}

	void FFQuantityPanel::RecalculatePosition()
	{
		int iWidth, iHeight;
		GetSize(iWidth, iHeight);

		Offset offset
			= CalculatePositionOffset(
				Size(iWidth - 2 * m_iPanelBorderWidthScaled, iHeight - 2 * m_iPanelBorderWidthScaled),
				m_iHorizontalAlign,
				m_iVerticalAlign);

		SetPos(
			m_scale.X * m_position.X - offset.X - m_iPanelBorderWidthScaled,
			m_scale.Y * m_position.Y - offset.Y - m_iPanelBorderWidthScaled);

		m_bPositionDirty = false;
	}
}
