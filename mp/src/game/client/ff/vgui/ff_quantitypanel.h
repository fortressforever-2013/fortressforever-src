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
#include "keyValues.h"

#include "ff_quantityhelper.h"
#include "ff_quantityitem.h"
#include <optional>

//this is defined in the panel style preset .h too, keep it in sync
#define QUANTITYPANELTEXTSIZES 15
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

		FFQuantityItem* AddItem(const char* pElementName);
		void HideItem(FFQuantityItem* qBar);
		void ShowItem(FFQuantityItem* qBar);
		void DisableItem(FFQuantityItem* qBar);
		void EnableItem(FFQuantityItem* qBar);

		void SetPreviewMode(bool bInPreviewMode);
		bool IsInPreviewMode();
		// TODO void FlashColor(Color colorFlash);

	protected:
		static KeyValues* s_kvAmountDisplayOptions;

		void AddPanelToHudOptions(const char* szSelfName, const char* szSelfText, const char* szParentName, const char* szParentText);
		void AddBooleanOption(KeyValues* kvMessage, const char* pszName, const char* pszText, const bool defaultValue = false, int iGroup = 0);
		void AddComboOption(KeyValues* kvMessage, const char* pszName, const char* pszText, KeyValues* kvOptions, const int defaultValueId = -1, int iGroup = 0);

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

		static std::optional<AmountDisplay> GetAmountDisplay(
			const char* keyName,
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData);

	private:
		static KeyValues* s_kvNoData;

		char m_szSelfName[128];
		char m_szSelfText[128];
		char m_szParentName[128];
		char m_szParentText[128];

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
		Offset m_paintOffset= Offset();
		Position m_position = Position();

		int m_iPanelMargin;

		HFont m_hfHeaderIconFamily[QUANTITYPANELICONSIZES * 3];
		HFont m_hfHeaderTextFamily[QUANTITYPANELTEXTSIZES * 3];
		HFont m_hfTextFamily[QUANTITYPANELTEXTSIZES * 3];

		HFont m_hfHeaderIcon;
		HFont m_hfHeaderText;
		HFont m_hfText;

		bool m_bHeaderTextShadow;
		bool m_bHeaderIconShadow;
		bool m_bTextShadow;

		bool m_bShowHeaderText;
		bool m_bShowHeaderIcon;

		bool m_bUseToggleText;
		bool m_bToggleTextVisible;

		int m_iHeaderTextSize;
		int m_iHeaderIconSize;
		int m_iTextSize;

		Color m_clrTeam;

		int m_iPositionalHashCode;
		Dar<FFQuantityItem*> m_darQuantityItems;

		bool m_bCheckUpdates;

		int m_iWidth;
		int m_iHeight;
		int m_iHorizontalAlign;
		int m_iVerticalAlign;

		std::wstring m_wHeaderText;
		std::wstring m_wHeaderIcon;
		std::wstring m_wText;

		Color m_clrHeaderText;
		Color m_clrHeaderIcon;
		Color m_clrText;

		Color m_clrPanelCustom;
		Color m_clrHeaderTextCustom;
		Color m_clrHeaderIconCustom;
		Color m_clrTextCustom;

		int m_iPanelColorMode;
		int m_iHeaderTextColorMode;
		int m_iHeaderIconColorMode;
		int m_iTextColorMode;

		Size m_headerTextSizeScaled;
		Size m_headerIconSizeScaled;
		Size m_textSizeScaled;
		Size m_itemsSizeScaled;

		AnchorPosition m_headerTextAnchorPosition;
		AnchorPosition m_headerIconAnchorPosition;
		AnchorPosition m_textAnchorPosition;

		Offset m_headerTextAnchorOffset;
		Offset m_headerIconAnchorOffset;
		Offset m_textAnchorOffset;

		int m_iHeaderTextAlignHoriz;
		int m_iHeaderIconAlignHoriz;
		int m_iTextAlignHoriz;

		int m_iHeaderTextAlignVert;
		int m_iHeaderIconAlignVert;
		int m_iTextAlignVert;

		Offset m_headerTextPositionOffset;
		Offset m_headerIconPositionOffset;
		Offset m_textPositionOffset;

		int m_iItemMarginHorizontal;
		int m_iItemMarginVertical;
		int m_iItemColumns;
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
			int iRed, int iGreen, int iBlue, int iAlpha);
		bool SetCustomHeaderIconColor(
			int iRed, int iGreen, int iBlue, int iAlpha);
		bool SetCustomHeaderTextColor(
			int iRed, int iGreen, int iBlue, int iAlpha);
		bool SetCustomTextColor(
			int iRed, int iGreen, int iBlue, int iAlpha);

		void SetTeamColor();

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
		bool HasText() const { return m_wText.length() && m_bUseToggleText; }

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
		MESSAGE_FUNC_INT_INT(OnScreenSizeChanged, "OnScreenSizeChanged", iOldWide, iOldTall);
	};
}

#endif