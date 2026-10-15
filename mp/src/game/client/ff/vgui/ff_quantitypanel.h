/********************************************************************
	created:	2010/08
	filename: 	cl_dll\ff\ff_hud_quantitypanel.h
	file path:	cl_dll\ff
	file base:	ff_hud_quantitypanel
	file ext:	h
	author:		Elmo

	purpose:	Customisable Quanitity Panel for the HUD
*********************************************************************/

#ifndef FF_QUANTITYPANEL_H
#define FF_QUANTITYPANEL_H

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include "KeyValues.h"

// Source's min/max macros must not expand inside the C++ standard headers.
#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#include <algorithm>
#include <string>
#include <optional>
#include <initializer_list>
#include <vector>
#pragma pop_macro("max")
#pragma pop_macro("min")

#include "ff_quantityhelper.h"
#include "ff_shareddefs.h"
#include "ff_quantityitem.h"

//this is defined in the panel style preset .h too, keep it in sync
#define QUANTITYPANELTEXTSIZES 25
#define QUANTITYPANELICONSIZES 20

using namespace FFQuantityHelper;

namespace vgui
{
	class FFQuantityPanel : public Panel
	{
	private:
		DECLARE_CLASS_SIMPLE(FFQuantityPanel, Panel);

		// Keep this private unless you know what you're doing!
		void PerformLayout() override;

	public:

		static constexpr int DEFAULT_HIDDEN_BITS
			= HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED;

		FFQuantityPanel(Panel* parent, const char* panelName);

		enum class Flow {
			Horizontal,         // row-major: left→right, then next row
			HorizontalReverse,  // row-major: right→left, then next row
			Vertical,           // column-major: top→bottom, then next column
			VerticalReverse     // column-major: bottom→top, then next column
		};

		enum Corners {
			CORNERS_ROUND = 2,
			CORNERS_SQUARE = 0
		};

		enum ItemOptionColumn
		{
			ITEM_VISIBILITY,
			ITEM_FORMAT
		};

		struct ItemOption
		{
			ItemOptionColumn column;
			const char* pszName;
			KeyValues* kvValues;
			int iDefaultValue;
		};

		void SetFlow(Flow flow) {
			if (m_flow != flow) {
				m_flow = flow;

				m_bItemsPositionDirty = true;
				InvalidateLayout();
			}
		}

		Flow GetFlow() const { return m_flow; }

		void SetHeaderText(const std::wstring wHeaderText);
		void SetHeaderIconChar(const char cHeaderIcon);
		void SetText(const std::wstring wText);

		void SetUseToggleText(bool bUseToggletext);
		void SetToggleTextVisible(bool bIsVisible);
		KeyValues* GetDefaultPanelSpecificValues();

		FFQuantityItem* AddItem(const char* pElementName);
		void InvalidateItemLayout();
		void HideItem(FFQuantityItem* qBar);
		void ShowItem(FFQuantityItem* qBar);
		void DisableItem(FFQuantityItem* qBar);
		void EnableItem(FFQuantityItem* qBar);

		void SetPreviewMode(bool bInPreviewMode);
		bool IsInPreviewMode();
		void TriggerAlert();
		void ClearAlert();

	protected:
		void AddAlertOptions(KeyValues* kvOptions, const char* pszHeadingText, int iGroup);
		void AddAlertDefaults(KeyValues* kvValues);
		void PaintBackground() override;
		void DrawPanelBorder();

		virtual int GetColorTeamNumber();
		void SetTeamColor();

		static KeyValues* s_kvAmountFormatOptions;

		void AddPanelToHudOptions(
			const char* szSelfName,
			const char* szSelfText,
			const char* szParentName,
			const char* szParentText);
		void AddHeading(KeyValues* kvOptions, const char* pszText, int iGroup);
		static KeyValues* GetQuantityVisibilityOptions();
		static KeyValues* GetCountDirectionOptions();
		static KeyValues* GetBuildableItemVisibilityOptions();

		void AddShowOption(
			KeyValues* kvOptions,
			bool bDefaultValue = true,
			int iColumn = 0);
		void AddShowTextOption(
			KeyValues* kvOptions,
			bool bDefaultValue = true,
			int iColumn = 0);
		void AddHideWhenFullOption(
			KeyValues* kvOptions,
			bool bDefaultValue = false,
			int iColumn = 0);

		void AddBooleanOption(
			KeyValues* kvMessage,
			const char* pszName,
			const char* pszText,
			const bool defaultValue = false,
			int iGroup = 0);
		void AddNumericOption(
			KeyValues* kvOptions,
			const char* pszName,
			const char* pszText,
			int iDefault,
			int iMinimum,
			int iMaximum,
			int iGroup = 0);

		void AddItemOption(
			KeyValues* kvOptions,
			const char* pszItem,
			const char* pszText,
			std::initializer_list<ItemOption> options);

		KeyValues* AddComboOption(
			KeyValues* kvMessage,
			const char* pszName,
			const char* pszText,
			KeyValues* kvOptions,
			const int defaultValueId = -1,
			int iGroup = 0);

		virtual void Paint();
		virtual void OnTick();

		virtual void AddPanelSpecificOptions(
			KeyValues* kvPanelSpecificOptions) = 0;
		virtual KeyValues* GetDefaultStyleData();
		virtual void ApplyStyleData(
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData);

		virtual void ApplySchemeSettings(IScheme* pScheme) override;

		static int GetInt(
			const char* keyName,
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData,
			int iDefaultValue = -1);

		static std::optional<AmountFormat> GetAmountFormat(
			const char* keyName,
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData);

	private:
		static KeyValues* s_kvNoData;

		char m_szSelfName[128]{};
		char m_szSelfText[128]{};
		char m_szParentName[128]{};
		char m_szParentText[128]{};

		bool m_bAddToHud= false;
		bool m_bAddToHudSent = false;

		bool m_bInPreviewMode = false;

		bool m_bHeaderIconFontDirty = true;
		bool m_bHeaderTextFontDirty = true;
		bool m_bTextFontDirty = true;

		bool m_bHeaderIconPositionDirty = true;
		bool m_bHeaderTextPositionDirty = true;
		bool m_bTextPositionDirty = true;
		bool m_bItemsPositionDirty = true;
		bool m_bPositionDirty = true;

		bool m_bPaintOffsetDirty = true;

		Scale m_scale = Scale();
		Offset m_paintOffset = Offset();
		Position m_position = Position();

		int m_iPanelMargin = 5;

		HFont m_hfHeaderIconFamily[QUANTITYPANELICONSIZES * 3]{};
		HFont m_hfHeaderTextFamily[QUANTITYPANELTEXTSIZES * 3]{};
		HFont m_hfTextFamily[QUANTITYPANELTEXTSIZES * 3]{};
		HFont m_hfVerdanaTextFamily[QUANTITYPANELTEXTSIZES * 3]{};

		HFont m_hfHeaderIcon = 0;
		HFont m_hfHeaderText = 0;
		HFont m_hfText = 0;

		bool m_bHeaderTextFontVerdana = false;
		bool m_bTextFontVerdana = false;

		bool m_bHeaderTextShadow = false;
		bool m_bHeaderIconShadow = false;
		bool m_bTextShadow = false;

		bool m_bShowHeaderText = true;
		bool m_bShowHeaderIcon = true;

		bool m_bShowText = true;
		bool m_bUseToggleText = false;
		bool m_bToggleTextVisible = true;

		int m_iHeaderTextSize = 4;
		int m_iHeaderIconSize = 4;
		int m_iTextSize = 4;

		enum AlertTarget
		{
			ALERT_NONE, ALERT_HEADER, ALERT_PANEL
		};
		AlertTarget m_alertTarget = ALERT_HEADER;
		Color m_clrAlert = Color(255, 80, 80, 255);
		int m_iAlertDuration = 800;
		float m_flAlertStart = -1.0f;
		bool m_bHasAlertOptions = false;
		Color GetAlertColor(Color normalColor, AlertTarget target) const;

		bool m_bShowPanelBorder = true;
		int m_iPanelBorderWidth = 1;
		int m_iPanelBorderWidthScaled = 0;
		int m_iPanelBorderColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		Color m_clrPanelBorderCustom{ 199, 219, 255, 215 };

		Color m_clrTeam = TEAM_COLOR_SPECTATOR;
		Color m_clrTeamBright = Color(255, 255, 255, 255);

		int m_iPositionalHashCode = 0;
		Dar<FFQuantityItem*> m_darQuantityItems;

		bool m_bCheckUpdates = false;

		int m_iWidth = 0;
		int m_iHeight = 0;
		int m_iHorizontalAlign = ALIGN_RIGHT;
		int m_iVerticalAlign = ALIGN_TOP;

		std::wstring m_wHeaderText;
		std::wstring m_wHeaderIcon;
		std::wstring m_wText;

		Color m_clrHeaderText = Color(255, 255, 255, 255);
		Color m_clrHeaderIcon = Color(255, 255, 255, 255);
		Color m_clrText = Color(255, 255, 255, 255);

		Color m_clrPanelCustom;
		Color m_clrHeaderTextCustom;
		Color m_clrHeaderIconCustom;
		Color m_clrTextCustom;

		int m_iPanelColorMode = COLOR_MODE_TEAM;
		int m_iHeaderTextColorMode = COLOR_MODE_CUSTOM;
		int m_iHeaderIconColorMode = COLOR_MODE_CUSTOM;
		int m_iTextColorMode = COLOR_MODE_CUSTOM;

		Size m_headerTextSizeScaled;
		Size m_headerIconSizeScaled;
		Size m_textSizeScaled;
		Size m_itemsSizeScaled;

		AnchorPosition m_headerTextAnchorPosition = ANCHORPOS_TOPRIGHT;
		AnchorPosition m_headerIconAnchorPosition = ANCHORPOS_TOPLEFT;
		AnchorPosition m_textAnchorPosition = ANCHORPOS_TOPLEFT;

		Offset m_headerTextAnchorOffset;
		Offset m_headerIconAnchorOffset;
		Offset m_textAnchorOffset;

		int m_iHeaderTextAlignHoriz = ALIGN_RIGHT;
		int m_iHeaderIconAlignHoriz = ALIGN_LEFT;
		int m_iTextAlignHoriz = ALIGN_LEFT;

		int m_iHeaderTextAlignVert = ALIGN_BOTTOM;
		int m_iHeaderIconAlignVert = ALIGN_BOTTOM;
		int m_iTextAlignVert = ALIGN_TOP;

		Offset m_headerTextPositionOffset = Offset(0, -2);
		Offset m_headerIconPositionOffset = Offset(0, -2);
		Offset m_textPositionOffset = Offset(0, 0);

		int m_iItemMarginHorizontal = 5;
		int m_iItemMarginVertical = 5;
		int m_iItemColumns = 1;
		Dar<Position> m_itemLocalPositions;

		Position m_itemsPosition;
		Position m_headerTextPosition;
		Position m_headerIconPosition;
		Position m_textPosition;

		Flow m_flow{ Flow::Horizontal };

		void RecalculateHeaderIconFont();
		void RecalculateHeaderTextFont();
		void RecalculateTextFont();

		void RecalculateHeaderIconColor();
		void RecalculateHeaderTextColor();
		void RecalculateTextColor();

		bool SetCustomPanelColor(
			int iRed,
			int iGreen,
			int iBlue,
			int iAlpha);
		bool SetCustomHeaderIconColor(
			int iRed,
			int iGreen,
			int iBlue,
			int iAlpha);
		bool SetCustomHeaderTextColor(
			int iRed,
			int iGreen,
			int iBlue,
			int iAlpha);
		bool SetCustomTextColor(
			int iRed,
			int iGreen,
			int iBlue,
			int iAlpha);


		void RecalculatePanelColor();

		KeyValues* AddItemStyles(KeyValues* kvItemStyleList);

		void RecalculateColor(Color& clr, int iColorMode, Color& clrCustom);
		Color& GetRgbColor(int iColorMode, Color& clrCustom);
		void SetColor(Color& clr, Color& clrRgb, Color& clrAlpha);

	private:
		void RecalculateItemPositions();
		void RecalculateHeaderIconPosition();
		void RecalculateHeaderTextPosition();
		void RecalculateTextPosition();
		void RecalculatePaintOffsetAndSize();
		void RecalculatePosition();

		bool HasHeaderText() const { return m_bShowHeaderText && m_wHeaderText.length(); }
		bool HasHeaderIcon() const { return m_bShowHeaderIcon && m_wHeaderIcon.length(); }
		bool HasText() const { return m_wText.length() && m_bShowText && m_bUseToggleText; }

		void DrawText(
			std::wstring wText,
			HFont font,
			Color color,
			Position position,
			Offset offset);

		void OnSizeChanged(int wide, int tall) override;

		MESSAGE_FUNC_PARAMS(OnDefaultStyleDataRequested, "GetStyleData", data);
		MESSAGE_FUNC_PARAMS(OnStyleDataReceived, "SetStyleData", kvStyleData);
		MESSAGE_FUNC_PARAMS(OnPresetPreviewDataRecieved, "SetPresetPreviewData", kvPresetPreviewData);
		MESSAGE_FUNC_INT_INT(
			OnScreenSizeChanged,
			"OnScreenSizeChanged",
			iOldWide,
			iOldTall);
	};
}

#endif
