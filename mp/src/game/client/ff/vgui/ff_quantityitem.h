/********************************************************************
	created:	2006/02/04
	filename:   ff_hud_quantityitem.h
	purpose:	Customisable Quantity indicator
*********************************************************************/

#ifndef FF_QUANTITYITEM_H
#define FF_QUANTITYITEM_H

#define QUANTITYITEMTEXTSIZES 15
#define QUANTITYITEMICONSIZES 20

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "ff_quantityhelper.h"
#include "ff_quantitytext.h"

namespace vgui
{
	class FFQuantityPanel; // Forward declaration

	class FFQuantityItem : public Panel
	{
		using AmountDisplay = FFQuantityHelper::AmountDisplay;
		using Offset = FFQuantityHelper::Offset;
		using Scale = FFQuantityHelper::Scale;
		using Size = FFQuantityHelper::Size;

	private:
		DECLARE_CLASS_SIMPLE(FFQuantityItem, Panel);

		bool m_bParentTrigeredInvalidation;

		FFQuantityPanel* m_parentQuantityPanel = nullptr;

		FFQuantityText* m_pAmountText = nullptr;
		FFQuantityText* m_pIconText = nullptr;
		FFQuantityText* m_pLabelText = nullptr;

		HFont m_hfTextFamily[QUANTITYITEMTEXTSIZES * 3];
		HFont m_hfTahomaTextFamily[QUANTITYITEMTEXTSIZES * 3];
		HFont m_hfIconFamily[QUANTITYITEMICONSIZES * 3];

		bool  m_bDisabled = false;

		Scale  m_scale{ 1.0f, 1.0f };

		// Offset that normalizes all sub-elements (bar, texts) into +ve coords
		// (e.g., bar at 0,0 but icon measures at -12,-12 => contentOffset 12,12)
		Offset m_contentOffset{ 0, 0 };

		// *** Calculated quantity fill offsets (relative to bar) ::
		int m_iBarX0QuantityOffset = 0;
		int m_iBarY0QuantityOffset = 0;
		int m_iBarX1QuantityOffset = 0;
		int m_iBarY1QuantityOffset = 0;

		Color m_clrBarBorder;
		Color m_clrBarBackground;
		Color m_clrBar;
		// *** Calculated ^

		float m_flAmount = 100.0f;
		float m_flMaxAmount = 100.0f;

		// *** Style Settings ::
		Size  m_barSize{ 60, 10 };
		int   m_iBarBorderWidth = 1;
		int   m_iBarOrientation = 0;

		int   m_iAmountDecimalPlaces = 0;

		AmountDisplay m_amountDisplay = FFQuantityHelper::DISPLAY_RAW;

		bool  m_bShowBar = true;
		bool  m_bShowBarBackground = true;
		bool  m_bShowBarBorder = true;

		// Intensity calc inputs
		bool  m_bIntensityAmountScaled = true;
		int   m_iIntensityRed = 20;
		int   m_iIntensityOrange = 50;
		int   m_iIntensityYellow = 80;
		int   m_iIntensityGreen = 100;
		int   m_bIntensityInvertScale = false;

		Color m_clrBarCustom{ 255,255,255,255 };
		Color m_clrBarBorderCustom{ 255,255,255,255 };
		Color m_clrBarBackgroundCustom{ 192,192,192,80 };

		int   m_iBarColorMode = FFQuantityHelper::COLOR_MODE_STEPPED;
		int   m_iBarBorderColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		int   m_iBarBackgroundColorMode = FFQuantityHelper::COLOR_MODE_STEPPED;
		// *** Style Settings ^^

		Color m_clrTeam;
		Color m_clrIntensityFaded;
		Color m_clrIntensityStepped;

		HFont GetFont(HFont* hfFamily, int iSize, bool bUseModifier);

		bool SetAmountDisplay(AmountDisplay amountDisplayType);

		void SetCustomBarColor(
			const int iRed, const int iGreen, const int iBlue, const int iAlpha);
		void SetCustomBarBorderColor(
			const int iRed, const int iGreen, const int iBlue, const int iAlpha);
		void SetCustomBarBackgroundColor(
			const int iRed, const int iGreen, const int iBlue, const int iAlpha);
		void SetCustomColor(
			Color& clrCustom,
			Color& clrFinal,
			const int iColorMode,
			const int iRed, const int iGreen, const int iBlue, const int iAlpha);
		bool SetTeamColor(Color clrTeam);

		void SetBarColorMode(int iBarColorMode);
		void SetBarBorderColorMode(int iBarBorderColorMode);
		void SetBarBackgroundColorMode(int iBarBackgroundColorMode);
		void SetColorMode(
			int& iColorMode,
			Color& clrFinal,
			const Color& clrCustom,
			const int iColorModeNew);

		Color GetIntensityColor(int iAmount, int iMaxAmount, int iColorSetting);

		void  RecalculateQuantity();

		void RecalculateAmountDisplay();
		void RecalculateAmountMaxDisplay();

		void RecalculateColor(
			Color& clrFinal,
			const int iColorMode,
			const Color& colorCustom);
		Color GetRgbColor(
			const int iColorMode,
			const Color& colorCustom);
		void  SetColor(
			Color& color,
			const Color& rgbColor,
			const Color& alphaColor);
		void  SetIntensityColor(
			Color& color,
			int iColorMode,
			Color& alphaColor);

		void  DrawText(wchar_t* wszText, HFont font, Color color, int iXPosition, int iYPosition);

		int   ScaleX(int value) const { return static_cast<int>(value * m_scale.X); }
		int   ScaleY(int value) const { return static_cast<int>(value * m_scale.Y); }

	public:
		FFQuantityItem(Panel* parent, const char* pElementName);

		enum Orientation {
			ORIENTATION_HORIZONTAL = 0,
			ORIENTATION_VERTICAL,
			ORIENTATION_HORIZONTAL_INVERTED,
			ORIENTATION_VERTICAL_INVERTED
		};

		bool ApplyStyleData(
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData);

		void SetAmount(float iAmount);
		void SetAmountMax(float iAmountMax);
		void SetAmountDecimalPlaces(int iDecimalPlaces);

		void SetIcon(char cIcon);
		void SetLabel(const char* szLabel);

		void SetIntensityLimits(int iRed, int iOrange, int iYellow, int iGreen);
		void SetIntensityAmountScaled(bool bAmountScaled);

		float GetAmount();
		float GetAmountMax();

		int GetAmountAsInt();

		void SetDisabled(bool bState);
		bool IsDisabled();

		// Panel-facing measurement contract: item reports tight size and content normalization offset.
		void GetContentMetrics(Size& size, Offset& contentOffset);

	protected:
		void ApplySchemeSettings(IScheme* pScheme) override;

		bool IsVisible() override;
		void Paint() override;
		void PerformLayout() override;

		void OnSizeChanged(int wide, int tall) override;

		MESSAGE_FUNC_INT_INT(OnScreenSizeChanged, "OnScreenSizeChanged", oldwide, oldtall);
	};
}
#endif
