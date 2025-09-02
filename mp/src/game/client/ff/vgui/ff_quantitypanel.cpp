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

#include <vgui/IVGui.h>
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

KeyValues* FFQuantityPanel::s_kvAmountDisplayOptions;

namespace vgui
{
	KeyValues* FFQuantityPanel::s_kvNoData = new KeyValues("NoData");

	// TODO initialize member variables (in the header?!)
	FFQuantityPanel::FFQuantityPanel(
		Panel* parent,
		const char* panelName)
		: BaseClass(parent, panelName)
	{
		m_bToggleTextVisible = true;

		m_paintOffset = Offset();
		m_position = Position();

		m_iPositionalHashCode = 0;
		m_headerTextPositionOffset.X = 0;
		m_headerTextPositionOffset.Y = -2;
		m_headerIconPositionOffset.X = 0;
		m_headerIconPositionOffset.Y = -2;
		m_textPositionOffset.X = 0;
		m_textPositionOffset.Y = 0;

		m_headerTextAnchorPosition = ANCHORPOS_TOPRIGHT;
		m_headerIconAnchorPosition = ANCHORPOS_TOPLEFT;
		m_textAnchorPosition = ANCHORPOS_TOPLEFT;

		m_iHeaderTextAlignHoriz = ALIGN_RIGHT;
		m_iHeaderIconAlignHoriz = ALIGN_LEFT;
		m_iTextAlignHoriz = ALIGN_LEFT;

		m_iHeaderTextAlignVert = ALIGN_BOTTOM;
		m_iHeaderIconAlignVert = ALIGN_BOTTOM;
		m_iTextAlignVert = ALIGN_TOP;

		m_iPanelColorMode = COLOR_MODE_TEAM;
		m_iHeaderTextColorMode = COLOR_MODE_CUSTOM;
		m_iHeaderIconColorMode = COLOR_MODE_CUSTOM;
		m_iTextColorMode = COLOR_MODE_CUSTOM;

		m_clrHeaderText = Color(255, 255, 255, 255);
		m_clrHeaderIcon = Color(255, 255, 255, 255);
		m_clrText = Color(255, 255, 255, 255);
		m_clrTeam = TEAM_COLOR_SPECTATOR;

		m_iHorizontalAlign = ALIGN_RIGHT;
		m_iVerticalAlign = ALIGN_TOP;
		m_iHeaderTextSize = 4;
		m_iHeaderIconSize = 4;
		m_bShowHeaderText = true;
		m_bShowHeaderIcon = true;
		m_bHeaderIconShadow = false;
		m_bHeaderTextShadow = false;

		m_iPanelMargin = 5;

		m_iItemMarginHorizontal = 5;
		m_iItemMarginVertical = 5;
		m_iItemColumns = 1;

		m_scale = GetScaleFromScreenSize();

		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(FFQuantityPanel::CORNERS_ROUND);
		SetPaintBorderEnabled(true);
		SetPaintEnabled(true);
		SetBgColor(TEAM_COLOR_SPECTATOR);

		m_bHeaderIconFontDirty = true;
		m_bHeaderTextFontDirty = true;
		m_bTextFontDirty = true;
		m_bHeaderIconPositionDirty = true;
		m_bHeaderTextPositionDirty = true;
		m_bTextPositionDirty = true;
		m_bItemsPositionDirty = true;
		m_bPaintOffsetDirty = true;
		m_bPositionDirty = true;

		if (!s_kvAmountDisplayOptions)
		{
			KeyValues* kvAmountDisplayOptions
				= new KeyValues("Values");

			kvAmountDisplayOptions->SetString(
				std::to_string(DISPLAY_RAW).c_str(),
				"#HudPanel_DisplayRaw");
			kvAmountDisplayOptions->SetString(
				std::to_string(DISPLAY_PERCENTAGE).c_str(),
				"#HudPanel_DisplayPercentage");
			kvAmountDisplayOptions->SetString(
				std::to_string(DISPLAY_MAX).c_str(),
				"#HudPanel_DisplayOverMax");

			s_kvAmountDisplayOptions
				= kvAmountDisplayOptions;
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
		}

		m_bHeaderIconFontDirty = true;
		m_bHeaderTextFontDirty = true;
		m_bTextFontDirty = true;
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
		kvPreset->SetInt("panelType", CORNERS_ROUND);
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

		kvPreset->SetInt("textSize", 4);
		kvPreset->SetInt("textShadow", 0);
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
		kvComponent->SetInt("fontTahoma", 0);
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
		kvComponent->SetInt("fontTahoma", 0);
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

	void FFQuantityPanel::Paint()
	{
		VPROF_BUDGET("FFQuantityPanel::Paint", "QuantityItems");

		BaseClass::Paint();

		if (HasHeaderText())
			DrawText(
				m_wHeaderText,
				m_hfHeaderText,
				m_clrHeaderText,
				m_headerTextPosition,
				m_paintOffset);

		if (HasHeaderIcon())
			DrawText(
				m_wHeaderIcon,
				m_hfHeaderIcon,
				m_clrHeaderIcon,
				m_headerIconPosition,
				m_paintOffset);

		if (HasText() && m_bToggleTextVisible)
			DrawText(
				m_wText,
				m_hfText,
				m_clrText,
				m_textPosition,
				m_paintOffset);
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
		// TODO: localise "text" first
		kv->SetString("text", pszText);
		if (defaultValue)
			kv->SetInt("defaultValue", 1);
		kvMessage->AddSubKey(kv);
	}

	void FFQuantityPanel::AddComboOption(
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
	}

	void FFQuantityPanel::SetTeamColor()
	{
		int iTeamNumber
			= C_FFPlayer::GetLocalFFPlayer()->GetTeamNumber();

		const Color& clrTeam
			= g_PR->GetTeamColor(iTeamNumber);

		bool bHasChanged = Change(
			m_clrTeam,
			clrTeam);

		if (!bHasChanged)
			return;

		for (int i = 0; i < m_darQuantityItems.GetCount(); ++i)
		{
			m_darQuantityItems[i]->SetTeamColor(clrTeam);
		}

		if (m_iPanelColorMode == COLOR_MODE_TEAM)
		{
			RecalculatePanelColor();
			Color& clrPanel = Color();
			SetColor(clrPanel, m_clrTeam, m_clrPanelCustom);
			SetBgColor(clrPanel);
		}

		if (m_iHeaderIconColorMode == COLOR_MODE_TEAM)
			SetColor(m_clrHeaderIcon, m_clrTeam, m_clrHeaderIconCustom);
		if (m_iHeaderTextColorMode == COLOR_MODE_TEAM)
			SetColor(m_clrHeaderText, m_clrTeam, m_clrHeaderTextCustom);
		if (m_iTextColorMode == COLOR_MODE_TEAM)
			SetColor(m_clrText, m_clrTeam, m_clrTextCustom);
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

		// If invalidation was called without any Dirty flags being set
		// then this was triggered by an item changing size. Therefore
		// we must recalculate the item positions.
		if (!m_bHeaderIconFontDirty
			&& !m_bHeaderTextFontDirty
			&& !m_bTextFontDirty
			&& !m_bItemsPositionDirty
			&& !m_bHeaderIconPositionDirty
			&& !m_bHeaderTextPositionDirty
			&& !m_bTextPositionDirty
			&& !m_bPaintOffsetDirty)
		{
			m_bItemsPositionDirty = true;
		}

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
			colCount = Max(1, primaryDim);                          // columns fixed
			rowCount = (enabledCount + colCount - 1) / colCount;    // ceil
			break;

		case vgui::FFQuantityPanel::Flow::Vertical:
		case vgui::FFQuantityPanel::Flow::VerticalReverse:
			rowCount = Max(1, primaryDim);                          // rows fixed
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
		ComputeGridDimensions(enabledCount, m_iItemColumns, m_flow, rowCount, colCount);

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
			MapIndexToGrid(itemIndex, rowCount, colCount, m_flow, rowIndex, colIndex);

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
			MapIndexToGrid(itemIndex, rowCount, colCount, m_flow, rowIndex, colIndex);

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
			SetCustomPanelColor(iPanelRed, iPanelGreen, iPanelBlue, iPanelAlpha);
		}

		int iShowHeaderIcon = GetInt("showHeaderIcon", kvStyleData, kvDefaultStyleData);
		int iHeaderIconShadow = GetInt("headerIconShadow", kvStyleData, kvDefaultStyleData);
		int iHeaderIconSize = GetInt("headerIconSize", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAnchorPosition = GetInt("headerIconAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlignHoriz = GetInt("headerIconAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlignVert = GetInt("headerIconAlignVert", kvStyleData, kvDefaultStyleData);
		int iHeaderIconX = GetInt("headerIconX", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderIconY = GetInt("headerIconY", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderIconColorMode = GetInt("headerIconColorMode", kvStyleData, kvDefaultStyleData);
		int iHeaderIconRed = GetInt("headerIconRed", kvStyleData, kvDefaultStyleData);
		int iHeaderIconGreen = GetInt("headerIconGreen", kvStyleData, kvDefaultStyleData);
		int iHeaderIconBlue = GetInt("headerIconBlue", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlpha = GetInt("headerIconAlpha", kvStyleData, kvDefaultStyleData);

		if (iShowHeaderIcon != -1
			&& Change(m_bShowHeaderIcon, iShowHeaderIcon == 1))
		{
			m_bHeaderIconPositionDirty = true;
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
		int iHeaderTextSize = GetInt("headerTextSize", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAnchorPosition = GetInt("headerTextAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlignHoriz = GetInt("headerTextAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlignVert = GetInt("headerTextAlignVert", kvStyleData, kvDefaultStyleData);
		int iHeaderTextX = GetInt("headerTextX", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderTextY = GetInt("headerTextY", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderTextColorMode = GetInt("headerTextColorMode", kvStyleData, kvDefaultStyleData);
		int iHeaderTextRed = GetInt("headerTextRed", kvStyleData, kvDefaultStyleData);
		int iHeaderTextGreen = GetInt("headerTextGreen", kvStyleData, kvDefaultStyleData);
		int iHeaderTextBlue = GetInt("headerTextBlue", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlpha = GetInt("headerTextAlpha", kvStyleData, kvDefaultStyleData);

		if (iShowHeaderText != -1
			&& Change(m_bShowHeaderText, iShowHeaderText == 1))
		{
			m_bHeaderTextPositionDirty = true;
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

		int iTextShadow = GetInt("textShadow", kvStyleData, kvDefaultStyleData);
		int iTextSize = GetInt("textSize", kvStyleData, kvDefaultStyleData);
		int iTextAnchorPosition = GetInt("textAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iTextAlignHoriz = GetInt("textAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iTextAlignVert = GetInt("textAlignVert", kvStyleData, kvDefaultStyleData);
		int iTextX = GetInt("textX", kvStyleData, kvDefaultStyleData, -9999);
		int iTextY = GetInt("textY", kvStyleData, kvDefaultStyleData, -9999);
		int iTextColorMode = GetInt("textColorMode", kvStyleData, kvDefaultStyleData);
		int iTextRed = GetInt("textRed", kvStyleData, kvDefaultStyleData);
		int iTextGreen = GetInt("textGreen", kvStyleData, kvDefaultStyleData);
		int iTextBlue = GetInt("textBlue", kvStyleData, kvDefaultStyleData);
		int iTextAlpha = GetInt("textAlpha", kvStyleData, kvDefaultStyleData);

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
			SetCustomTextColor(iTextRed, iTextGreen, iTextBlue, iTextAlpha);
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
					m_bHeaderIconShadow));

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
					m_hfHeaderTextFamily,
					m_iHeaderTextSize,
					m_bHeaderTextShadow));

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
					m_hfTextFamily,
					m_iTextSize,
					m_bTextShadow));

		if (bHasChanged)
			m_bTextPositionDirty = true;
	}

	void FFQuantityPanel::RecalculatePanelColor()
	{
		Color& clrPanel = Color();
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

	bool FFQuantityPanel::SetCustomPanelColor(int iRed, int iGreen, int iBlue, int iAlpha)
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = Change(m_clrPanelCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculatePanelColor();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomHeaderIconColor(
		int iRed, int iGreen, int iBlue, int iAlpha)
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = Change(m_clrHeaderIconCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor(m_clrHeaderIcon, m_iHeaderIconColorMode, m_clrHeaderIconCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomHeaderTextColor(
		int iRed, int iGreen, int iBlue, int iAlpha)
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = Change(m_clrHeaderTextCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor(m_clrHeaderText, m_iHeaderTextColorMode, m_clrHeaderTextCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomTextColor(
		int iRed, int iGreen, int iBlue, int iAlpha)
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
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

	std::optional<AmountDisplay> FFQuantityPanel::GetAmountDisplay(
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
			? std::optional<AmountDisplay>()
			: static_cast<AmountDisplay>(value);
	}

	void FFQuantityPanel::OnStyleDataReceived(KeyValues* kvStyleData)
	{
		ApplyStyleData(
			kvStyleData,
			GetDefaultStyleData());
	}

	void FFQuantityPanel::OnPresetPreviewDataRecieved(KeyValues* kvPresetPreviewData)
	{
		ApplyStyleData(
			kvPresetPreviewData,
			s_kvNoData);
	}

	void FFQuantityPanel::OnDefaultStyleDataRequested(KeyValues* data)
	{
		g_AP->CreatePresetFromPanelDefault(GetDefaultStyleData());
	}

	FFQuantityItem* FFQuantityPanel::AddItem(
		const char* pElementName)
	{
		FFQuantityItem* newQBar
			= new FFQuantityItem(
				this,
				pElementName);

		m_darQuantityItems
			.AddElement(newQBar);

		return newQBar;
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
		m_bInPreviewMode = bInPreviewMode;

		if (m_bInPreviewMode)
		{
			SetParent(enginevgui->GetPanel(PANEL_ROOT));
		}
		else
		{
			SetParent(g_pClientMode->GetViewport());
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

		m_bPositionDirty = true;

		InvalidateLayout();
	}

	void FFQuantityPanel::RecalculatePosition()
	{
		int iWidth, iHeight;
		GetSize(iWidth, iHeight);

		Offset offset
			= CalculatePositionOffset(
				Size(iWidth, iHeight),
				m_iHorizontalAlign,
				m_iVerticalAlign);

		SetPos(
			m_scale.X * m_position.X - offset.X,
			m_scale.Y * m_position.Y - offset.Y);

		m_bPositionDirty = false;
	}
}