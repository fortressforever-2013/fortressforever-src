//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_quantityitem.cpp
//

#include "cbase.h"
#include "ff_quantityitem.h"
#include "ff_utils.h"

#include "vprof.h"

using namespace FFQuantityHelper;

namespace vgui
{
	template <typename T>
	T minOf(const T& a, const T& b, const T& c, const T& d) {
		return min(min(a, b), min(c, d));
	}

	template <typename T>
	T maxOf(const T& a, const T& b, const T& c, const T& d) {
		return max(max(a, b), max(c, d));
	}

	FFQuantityItem::FFQuantityItem(
		Panel* parent,
		const char* pElementName)
		: BaseClass(
			parent,
			pElementName)
	{
		// Children
		m_pAmountText = new FFQuantityText(this, "AmountText");
		m_pIconText = new FFQuantityText(this, "IconText");
		m_pLabelText = new FFQuantityText(this, "LabelText");

		// Defaults already in header member initializers; just confirm style-related ones
		m_pAmountText->SetAnchorAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
		m_pIconText->SetAnchorAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
		m_pLabelText->SetAnchorAlignment(ALIGN_LEFT, ALIGN_BOTTOM);

		m_pAmountText->SetAnchorOffset(Offset(0, 0));
		m_pIconText->SetAnchorOffset(Offset(5, 0));
		m_pLabelText->SetAnchorOffset(Offset(-5, 0));

		m_pAmountText->SetAnchorPosition(ANCHORPOS_TOPRIGHT);
		m_pIconText->SetAnchorPosition(ANCHORPOS_MIDDLELEFT);
		m_pLabelText->SetAnchorPosition(ANCHORPOS_TOPLEFT);

		// Initial bar colours (custom defaults)
		const Color clrWhite(255, 255, 255, 255);
		const Color clrGrayFaded(192, 192, 192, 80);

		m_clrBarCustom = clrWhite;
		m_clrBarBorderCustom = clrWhite;
		m_clrBarBackgroundCustom = clrGrayFaded;

		m_clrBar = m_clrBarCustom;
		m_clrBarBorder = m_clrBarBorderCustom;
		m_clrBarBackground = m_clrBarBackgroundCustom;

		// Compute initial quantity-dependent colours once so first paint is correct
		RecalculateQuantity();

		// Fonts arrays init
		for (int i = 0; i < QUANTITYITEMTEXTSIZES * 3; ++i) {
			m_hfTextFamily[i] = NULL;
			m_hfTahomaTextFamily[i] = NULL;
		}
		for (int i = 0; i < QUANTITYITEMICONSIZES * 3; ++i) {
			m_hfIconFamily[i] = NULL;
		}

		// Initialize scale once so first layout is correct
		m_scale = GetScaleFromScreenSize();

		// Ensure content offset starts defined
		m_contentOffset = Offset(0, 0);
	}

	void FFQuantityItem::ApplySchemeSettings(IScheme* pScheme)
	{
		HScheme QuantityItemScheme
			= scheme()->LoadSchemeFromFile(
				"resource/QuantityPanelScheme.res",
				"QuantityPanelScheme");

		IScheme* qbScheme
			= scheme()->GetIScheme(QuantityItemScheme);

		// Non-scaled fonts for measurement (everything measured as if 640x480)
		for (int i = 0; i < QUANTITYITEMTEXTSIZES; ++i)
		{
			m_hfTextFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityItem%d", i), true);
			m_hfTextFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityItemShadow%d", i), true);
			m_hfTextFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityItem%d", i), false);

			m_hfTahomaTextFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityItemTahoma%d", i), true);
			m_hfTahomaTextFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityItemTahomaShadow%d", i), true);
			m_hfTahomaTextFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityItemTahoma%d", i), false);
		}

		for (int i = 0; i < QUANTITYITEMICONSIZES; ++i)
		{
			m_hfIconFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityItemIcon%d", i), true);
			m_hfIconFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityItemIconShadow%d", i), true);
			m_hfIconFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityItemIcon%d", i), false);
		}

		// Assign font families to child texts
		m_pAmountText->SetFontFamily(m_hfTextFamily);
		m_pLabelText->SetFontFamily(m_hfTextFamily);
		m_pIconText->SetFontFamily(m_hfIconFamily);

		BaseClass::ApplySchemeSettings(pScheme);
	}

	void FFQuantityItem::OnScreenSizeChanged(
		int iOldWide,
		int iOldTall)
	{
		Panel::OnScreenSizeChanged(
			iOldWide,
			iOldTall);

		Scale newScale
			= GetScaleFromScreenSize();

		if (m_scale == newScale)
			return;

		m_scale = newScale;

		InvalidateLayout();
	}

	bool FFQuantityItem::IsVisible()
	{
		if (m_bDisabled)
			return false;

		return Panel::IsVisible();
	}

	void FFQuantityItem::Paint()
	{
		VPROF_BUDGET("FFQuantityItem::Paint", "QuantityItem");

		BaseClass::Paint();

		if (m_bShowBarBorder)
		{
			surface()->DrawSetColor(m_clrBarBorder);

			const int scaledBorderWidth
				= ScaleX(m_iBarBorderWidth);

			for (int i = 1; i <= scaledBorderWidth; ++i)
			{
				surface()->DrawOutlinedRect(
					m_contentOffset.X - i,
					m_contentOffset.Y - i,
					m_contentOffset.X + ScaleX(m_barSize.Width) + i,
					m_contentOffset.Y + ScaleY(m_barSize.Height) + i);
			}
		}

		if (m_bShowBarBackground)
		{
			surface()->DrawSetColor(m_clrBarBackground);
			surface()->DrawFilledRect(
				m_contentOffset.X,
				m_contentOffset.Y,
				m_contentOffset.X + ScaleX(m_barSize.Width),
				m_contentOffset.Y + ScaleY(m_barSize.Height));
		}

		if (m_bShowBar)
		{
			surface()->DrawSetColor(m_clrBar);
			surface()->DrawFilledRect(
				m_contentOffset.X + ScaleX(m_iBarX0QuantityOffset),
				m_contentOffset.Y + ScaleY(m_iBarY0QuantityOffset),
				m_contentOffset.X + ScaleX(m_barSize.Width + m_iBarX1QuantityOffset),
				m_contentOffset.Y + ScaleY(m_barSize.Height + m_iBarY1QuantityOffset));
		}
	}

	void FFQuantityItem::SetDisabled(
		bool bState)
	{
		m_bDisabled = bState;
	}

	bool FFQuantityItem::IsDisabled()
	{
		return m_bDisabled;
	}

	void FFQuantityItem::SetAmount(
		float iAmount)
	{
		bool bHasChanged
			= Change(m_flAmount, iAmount);

		if (bHasChanged)
		{
			RecalculateAmountDisplay();
			RecalculateQuantity();
		}
	}

	void FFQuantityItem::SetAmountMax(
		float iAmountMax)
	{
		if (iAmountMax == 0)
			return; // avoid divide-by-zero

		bool bHasChanged
			= Change(m_flMaxAmount, iAmountMax);

		if (bHasChanged)
		{
			RecalculateAmountMaxDisplay();
			RecalculateAmountDisplay();
			RecalculateQuantity();
		}
	}

	void FFQuantityItem::SetIcon(
		char cIcon)
	{
		m_pIconText->SetText({
			static_cast<wchar_t>(cIcon),
			L'\0' }); // null-terminate
	}

	void FFQuantityItem::SetLabel(
		const char* szLabel)
	{
		if (wchar_t* pszLocalized = g_pVGuiLocalize->Find(szLabel))
		{
			m_pLabelText->SetText(std::wstring(pszLocalized));
			return;
		}

		const size_t iLength
			= strlen(szLabel) + 1;

		std::wstring wLabel(
			iLength,
			L'\0');

		g_pVGuiLocalize->ConvertANSIToUnicode(
			szLabel,
			wLabel.data(),
			iLength * sizeof(wchar_t));

		m_pLabelText->SetText(wLabel);
	}

	bool FFQuantityItem::SetAmountDisplay(
		AmountDisplay amountDisplay)
	{
		bool bHasChanged
			= Change(m_amountDisplay, amountDisplay);

		if (bHasChanged)
		{
			RecalculateAmountDisplay();
			RecalculateAmountMaxDisplay();
		}

		return bHasChanged;
	}

	void FFQuantityItem::SetBarColorMode(
		const int iColorModeBar)
	{
		SetColorMode(
			m_iBarColorMode,
			m_clrBar,
			m_clrBarCustom,
			iColorModeBar);
	}

	void FFQuantityItem::SetBarBorderColorMode(
		const int iColorModeBarBorder)
	{
		SetColorMode(
			m_iBarBorderColorMode,
			m_clrBarBorder,
			m_clrBarBorderCustom,
			iColorModeBarBorder);
	}

	void FFQuantityItem::SetBarBackgroundColorMode(
		const int iColorModeBarBackground)
	{
		SetColorMode(
			m_iBarBackgroundColorMode,
			m_clrBarBackground,
			m_clrBarBackgroundCustom,
			iColorModeBarBackground);
	}

	void FFQuantityItem::SetColorMode(
		int& iColorMode,
		Color& clrFinal,
		const Color& clrCustom,
		const int iColorModeNew)
	{
		bool bHasChanged
			= Change(iColorMode, iColorModeNew);

		if (bHasChanged)
		{
			RecalculateColor(
				clrFinal,
				iColorMode,
				clrCustom);
		}
	}

	void FFQuantityItem::SetCustomBarColor(
		const int r, const int g, const int b, const int a)
	{
		SetCustomColor(
			m_clrBarCustom,
			m_clrBar,
			m_iBarColorMode,
			r, g, b, a);
	}

	void FFQuantityItem::SetCustomBarBorderColor(
		const int r, const int g, const int b, const int a)
	{
		SetCustomColor(
			m_clrBarBorderCustom,
			m_clrBarBorder,
			m_iBarBorderColorMode,
			r, g, b, a);
	}

	void FFQuantityItem::SetCustomBarBackgroundColor(
		const int r, const int g, const int b, const int a)
	{
		SetCustomColor(
			m_clrBarBackgroundCustom,
			m_clrBarBackground,
			m_iBarBackgroundColorMode,
			r, g, b, a);
	}

	void FFQuantityItem::SetCustomColor(
		Color& clrCustom,
		Color& clrFinal,
		int iColorMode,
		const int r, const int g, const int b, const int a)
	{
		const Color clrNew(r, g, b, a);

		bool bHasChanged
			= Change(clrCustom, clrNew);

		if (bHasChanged)
		{
			RecalculateColor(
				clrFinal,
				iColorMode,
				clrCustom);
		}
	}

	bool FFQuantityItem::SetTeamColor(
		Color clrTeam)
	{
		bool bHasChanged
			= Change(m_clrTeam, clrTeam);

		if (!bHasChanged)
			return false;

		if (m_iBarColorMode == COLOR_MODE_TEAM)
			RecalculateColor(m_clrBar, m_iBarColorMode, m_clrBarCustom);
		if (m_iBarBorderColorMode == COLOR_MODE_TEAM)
			RecalculateColor(m_clrBarBorder, m_iBarBorderColorMode, m_clrBarBorderCustom);
		if (m_iBarBackgroundColorMode == COLOR_MODE_TEAM)
			RecalculateColor(m_clrBarBackground, m_iBarBackgroundColorMode, m_clrBarBackgroundCustom);

		m_pAmountText->SetTeamColor(clrTeam);
		m_pIconText->SetTeamColor(clrTeam);
		m_pLabelText->SetTeamColor(clrTeam);
		return true;
	}

	float FFQuantityItem::GetAmount() { return m_flAmount; }
	float FFQuantityItem::GetAmountMax() { return m_flMaxAmount; }
	int FFQuantityItem::GetAmountAsInt() { return static_cast<int>(m_flAmount); }

	void FFQuantityItem::SetIntensityLimits(int r, int o, int y, int g)
	{
		m_iIntensityRed = r; m_iIntensityOrange = o; m_iIntensityYellow = y; m_iIntensityGreen = g;
		RecalculateQuantity();
	}

	void FFQuantityItem::SetIntensityAmountScaled(bool bScaled)
	{
		m_bIntensityAmountScaled = bScaled;
		RecalculateQuantity();
	}

	bool FFQuantityItem::ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData)
	{
		bool bRecalculateQuantity = false;
		bool bInvalidateLayout = false;

		// Read scalar bar settings
		const int iBarWidth = GetInt("barWidth", kvStyleData, kvDefaultStyleData);
		const int iBarHeight = GetInt("barHeight", kvStyleData, kvDefaultStyleData);
		const int iBarBorderWidth = GetInt("barBorderWidth", kvStyleData, kvDefaultStyleData);
		const int iBarOrientation = GetInt("barOrientation", kvStyleData, kvDefaultStyleData);

		if (iBarWidth != -1 && iBarHeight != -1
			&& Change(m_barSize, Size(iBarWidth, iBarHeight)))
		{
			m_pAmountText->SetAnchorSize(m_barSize);
			m_pIconText->SetAnchorSize(m_barSize);
			m_pLabelText->SetAnchorSize(m_barSize);

			bRecalculateQuantity = true;
			bInvalidateLayout = true;
		}

		if (iBarBorderWidth != -1
			&& Change(m_iBarBorderWidth, iBarBorderWidth))
		{
			bInvalidateLayout = true;
		}

		if (iBarOrientation != -1
			&& Change(m_iBarOrientation, iBarOrientation))
		{
			bRecalculateQuantity = true;
		}

		// Bar components
		if (KeyValues* kvBar = GetData("Bar", kvStyleData, kvDefaultStyleData))
		{
			const int iShow = GetInt("show", kvBar);
			const int iRed = GetInt("red", kvBar);
			const int iGreen = GetInt("green", kvBar);
			const int iBlue = GetInt("blue", kvBar);
			const int iAlpha = GetInt("alpha", kvBar);
			const int iColorMode = GetInt("colorMode", kvBar);

			if (iShow != -1)
			{
				m_bShowBar = iShow == 1;

				// Invalidation not required as bar size is always
				// maintained in calculation, irrespective of visibility
			}

			if (iColorMode != -1)
			{
				SetBarColorMode(iColorMode);
			}

			if (iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomBarColor(iRed, iGreen, iBlue, iAlpha);
			}
		}

		if (KeyValues* kvBarBorder = GetData("BarBorder", kvStyleData, kvDefaultStyleData))
		{
			const int iShow = GetInt("show", kvBarBorder);
			const int iRed = GetInt("red", kvBarBorder);
			const int iGreen = GetInt("green", kvBarBorder);
			const int iBlue = GetInt("blue", kvBarBorder);
			const int iAlpha = GetInt("alpha", kvBarBorder);
			const int iColorMode = GetInt("colorMode", kvBarBorder);

			if (iShow != -1
				&& Change(m_bShowBarBorder, iShow == 1))
			{
				// Show/hide border affects layout due to thickness
				bInvalidateLayout = true;
			}

			if (iColorMode != -1)
			{
				SetBarBorderColorMode(iColorMode);
			}

			if (iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomBarBorderColor(iRed, iGreen, iBlue, iAlpha);
			}
		}

		if (KeyValues* kvBarBg = GetData("BarBackground", kvStyleData, kvDefaultStyleData))
		{
			const int iShow = GetInt("show", kvBarBg);
			const int iRed = GetInt("red", kvBarBg);
			const int iGreen = GetInt("green", kvBarBg);
			const int iBlue = GetInt("blue", kvBarBg);
			const int iAlpha = GetInt("alpha", kvBarBg);
			const int iColorMode = GetInt("colorMode", kvBarBg);

			if (iShow != -1)
			{
				m_bShowBarBackground = iShow == 1;

				// Invalidation not required as bar size is always
				// maintained in calculation, irrespective of visibility
			}

			if (iColorMode != -1)
			{
				SetBarBackgroundColorMode(iColorMode);
			}

			if (iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomBarBackgroundColor(iRed, iGreen, iBlue, iAlpha);
			}
		}

		// Text components
		if (KeyValues* kvIcon = GetData("Icon", kvStyleData, kvDefaultStyleData)) {
			if (m_pIconText->ApplyStyleData(kvIcon, kvDefaultStyleData)) {
				bInvalidateLayout = true;
			}
		}

		if (KeyValues* kvLabel = GetData("Label", kvStyleData, kvDefaultStyleData)) {
			if (m_pLabelText->ApplyStyleData(kvLabel, kvDefaultStyleData)) {
				bInvalidateLayout = true;
			}
			const int iFontTahoma = GetInt("fontTahoma", kvLabel);
			if (iFontTahoma != -1
				&& m_pLabelText->SetFontFamily(
					iFontTahoma == 1
					? m_hfTahomaTextFamily
					: m_hfTextFamily))
			{
				bInvalidateLayout = true;
			}
		}

		if (KeyValues* kvAmount = GetData("Amount", kvStyleData, kvDefaultStyleData)) {
			if (m_pAmountText->ApplyStyleData(kvAmount, kvDefaultStyleData)) {
				bInvalidateLayout = true;
			}
			const int iFontTahoma = GetInt("fontTahoma", kvAmount);
			if (iFontTahoma != -1
				&& m_pAmountText->SetFontFamily(
					iFontTahoma == 1
					? m_hfTahomaTextFamily
					: m_hfTextFamily))
			{
				bInvalidateLayout = true;
			}
		}

		if (bInvalidateLayout) {
			m_bParentTrigeredInvalidation = true;
			InvalidateLayout(true);
		}

		if (bRecalculateQuantity) {
			RecalculateQuantity();
		}

		return bInvalidateLayout;
	}

	void FFQuantityItem::PerformLayout()
	{
		// Let base do any housekeeping (safe/no-op in many cases)
		BaseClass::PerformLayout();

		// Layout strategy:
		// - Measure bar rect (possibly includes border thickness)
		// - Fetch each text's bounds relative to the bar
		// - Compute normalization offset (to avoid negative coords)
		// - Position texts with that offset
		// - Set our own size as the union of bar + texts

		const float scaledBorderThickness
			= m_bShowBarBorder
			? static_cast<float>(ScaleX(m_iBarBorderWidth))
			: 0.0f;

		// Bar extents in item-local space (before normalization)
		const float barX0 = -scaledBorderThickness;
		const float barY0 = -scaledBorderThickness;

		const float barX1
			= ScaleX(m_barSize.Width) + scaledBorderThickness;
		const float barY1
			= ScaleY(m_barSize.Height) + scaledBorderThickness;

		Bounds amountBounds = m_pAmountText->GetRelativeBounds();
		Bounds iconBounds = m_pIconText->GetRelativeBounds();
		Bounds labelBounds = m_pLabelText->GetRelativeBounds();

		const float minX0 = minOf(barX0, amountBounds.X0, iconBounds.X0, labelBounds.X0);
		const float minY0 = minOf(barY0, amountBounds.Y0, iconBounds.Y0, labelBounds.Y0);

		// Normalization offset (contentOffset)
		const float offsetX = -minX0;
		const float offsetY = -minY0;

		m_contentOffset = Offset(
			static_cast<int>(offsetX),
			static_cast<int>(offsetY));

		// Place texts (item owns child positions; texts paint at their local 0,0)
		m_pAmountText->SetPos(
			static_cast<int>(offsetX + amountBounds.X0),
			static_cast<int>(offsetY + amountBounds.Y0));
		m_pIconText->SetPos(
			static_cast<int>(offsetX + iconBounds.X0),
			static_cast<int>(offsetY + iconBounds.Y0));
		m_pLabelText->SetPos(
			static_cast<int>(offsetX + labelBounds.X0),
			static_cast<int>(offsetY + labelBounds.Y0));

		const float maxX1 = maxOf(barX1, amountBounds.X1, iconBounds.X1, labelBounds.X1);
		const float maxY1 = maxOf(barY1, amountBounds.Y1, iconBounds.Y1, labelBounds.Y1);


		Size currentSize = Size();

		GetSize(currentSize.Width, currentSize.Height);

		Size desiredSize
			= Size(
				static_cast<int>(maxX1 - minX0),
				static_cast<int>(maxY1 - minY0));

		// When the size changes, we invalidate the parent.
		// When an invalidation is triggered by the parent,
		// we prevent an invalidation loop using a flag
		// within OnSizeChanged.
		//
		// If the size hasn't changed  but the parent-triggered
		// invalidation flag is set, we clear it here.
		if (currentSize != desiredSize)
		{
			SetSize(desiredSize.Width, desiredSize.Height);
		}
		else if (m_bParentTrigeredInvalidation)
		{
			m_bParentTrigeredInvalidation = false;
		}
	}

	void FFQuantityItem::SetAmountDecimalPlaces(
		int iDecimalPlaces)
	{
		m_iAmountDecimalPlaces
			= clamp(iDecimalPlaces, 0, 5);

		RecalculateAmountDisplay();
		RecalculateAmountMaxDisplay();
	}

	inline int CeilToInt(float x)
	{
		return static_cast<int>(ceilf(x));
	}

	void FFQuantityItem::RecalculateAmountDisplay()
	{
		std::wstring text = L"";
		float flAmount = m_flAmount;

		switch (m_amountDisplay)
		{
		default:
		case DISPLAY_RAW:
			// When showing 0 decimals, round up to nearest whole.
			// Consider timers like detpack, rounding up makes sense
			// to ensure zero means zero (boom).
			if (m_iAmountDecimalPlaces == 0) {
				flAmount = ceilf(flAmount);
			}

			wchar_t buffer[32];
			swprintf(
				buffer,
				sizeof(buffer) / sizeof(wchar_t),
				L"%.*f",
				m_iAmountDecimalPlaces,
				flAmount);

			text = std::wstring(buffer);
			break;

		case DISPLAY_PERCENTAGE:
			text
				= std::to_wstring(CeilToInt((flAmount * 100) / m_flMaxAmount))
				+ L"%";
			break;

		case DISPLAY_MAX:
			text
				= std::to_wstring(CeilToInt(flAmount))
				+ L"/"
				+ std::to_wstring(CeilToInt(m_flMaxAmount));
			break;
		}

		m_pAmountText->SetText(text);
	}

	void FFQuantityItem::RecalculateAmountMaxDisplay()
	{
		switch (m_amountDisplay)
		{
		default:
		case DISPLAY_RAW:
			m_pAmountText->ClearTextMax();
			break;

		case DISPLAY_PERCENTAGE:
			m_pAmountText->SetTextMax(L"100%");
			break;

		case DISPLAY_MAX:
			m_pAmountText->SetTextMax(
				std::to_wstring(static_cast<int>(m_flMaxAmount))
				+ L"/"
				+ std::to_wstring(static_cast<int>(m_flMaxAmount)));
			break;
		}
	}

	Color FFQuantityItem::GetIntensityColor(
		int iAmount,
		int iMaxAmount,
		int iColorMode)
	{
		return ::GetIntensityColor(
			iAmount,
			iColorMode,
			255,
			m_iIntensityRed,
			m_iIntensityOrange,
			m_iIntensityYellow,
			m_iIntensityGreen,
			m_bIntensityInvertScale);
	}

	void FFQuantityItem::RecalculateQuantity()
	{
		const int iAmountPercent = (m_flAmount * 100) / m_flMaxAmount;

		Color clrIntensityStepped, clrIntensityFaded;
		if (m_bIntensityAmountScaled) {
			clrIntensityFaded = GetIntensityColor(iAmountPercent, 100, 2);
			clrIntensityStepped = GetIntensityColor(iAmountPercent, 100, 1);
		}
		else {
			clrIntensityFaded = GetIntensityColor(m_flAmount, m_flMaxAmount, 2);
			clrIntensityStepped = GetIntensityColor(m_flAmount, m_flMaxAmount, 1);
		}

		const bool bFadedChanged = Change(m_clrIntensityFaded, clrIntensityFaded);
		const bool bSteppedChanged = Change(m_clrIntensityStepped, clrIntensityStepped);

		if (bFadedChanged || bSteppedChanged)
		{
			SetIntensityColor(m_clrBarBorder, m_iBarBorderColorMode, m_clrBarBorderCustom);
			SetIntensityColor(m_clrBarBackground, m_iBarBackgroundColorMode, m_clrBarBackgroundCustom);
			SetIntensityColor(m_clrBar, m_iBarColorMode, m_clrBarCustom);

			if (bFadedChanged) {
				m_pAmountText->SetIntensityFadedColor(m_clrIntensityFaded);
				m_pIconText->SetIntensityFadedColor(m_clrIntensityFaded);
				m_pLabelText->SetIntensityFadedColor(m_clrIntensityFaded);
			}
			if (bSteppedChanged) {
				m_pAmountText->SetIntensitySteppedColor(m_clrIntensityStepped);
				m_pIconText->SetIntensitySteppedColor(m_clrIntensityStepped);
				m_pLabelText->SetIntensitySteppedColor(m_clrIntensityStepped);
			}
		}

		// Quantity fill offsets relative to bar
		m_iBarX0QuantityOffset = 0;
		m_iBarX1QuantityOffset = 0;
		m_iBarY0QuantityOffset = 0;
		m_iBarY1QuantityOffset = 0;

		switch (m_iBarOrientation)
		{
		case ORIENTATION_HORIZONTAL_INVERTED:
			m_iBarX0QuantityOffset = m_barSize.Width - (m_barSize.Width * m_flAmount / m_flMaxAmount);
			break;
		case ORIENTATION_VERTICAL:
			m_iBarY0QuantityOffset = m_barSize.Height - (m_barSize.Height * m_flAmount / m_flMaxAmount);
			break;
		case ORIENTATION_VERTICAL_INVERTED:
			m_iBarY1QuantityOffset = (m_barSize.Height * m_flAmount / m_flMaxAmount) - m_barSize.Height;
			break;
		case ORIENTATION_HORIZONTAL:
		default:
			m_iBarX1QuantityOffset = (m_barSize.Width * m_flAmount / m_flMaxAmount) - m_barSize.Width;
			break;
		}
	}

	void FFQuantityItem::SetIntensityColor(
		Color& color,
		int iColorMode,
		Color& alphaColor)
	{
		if (iColorMode == COLOR_MODE_FADED)
			SetColor(color, m_clrIntensityFaded, alphaColor);
		else if (iColorMode == COLOR_MODE_STEPPED)
			SetColor(color, m_clrIntensityStepped, alphaColor);
	}

	void FFQuantityItem::RecalculateColor(
		Color& clrFinal,
		const int iColorMode,
		const Color& clrCustom)
	{
		const Color rgb
			= GetRgbColor(
				iColorMode,
				clrCustom);

		SetColor(
			clrFinal,
			rgb,
			clrCustom);
	}

	Color FFQuantityItem::GetRgbColor(
		const int iColorMode,
		const Color& colorCustom)
	{
		switch (iColorMode)
		{
		case COLOR_MODE_STEPPED:
			return m_clrIntensityStepped;
		case COLOR_MODE_FADED:
			return m_clrIntensityFaded;
		case COLOR_MODE_TEAM:
			return m_clrTeam;
		case COLOR_MODE_CUSTOM:
		default:
			return colorCustom;
		}
	}

	void FFQuantityItem::SetColor(
		Color& color,
		const Color& rgb,
		const Color& alphaColor)
	{
		color.SetColor(
			rgb.r(),
			rgb.g(),
			rgb.b(),
			alphaColor.a());
	}

	void FFQuantityItem::OnSizeChanged(
		int wide,
		int tall)
	{
		Panel::OnSizeChanged(wide, tall);

		if (m_bParentTrigeredInvalidation)
		{
			m_bParentTrigeredInvalidation = false;
			return;
		}

		GetParent()->InvalidateLayout(true);
	}

	void FFQuantityItem::GetContentMetrics(
		Size& size,
		Offset& contentOffset)
	{
		GetSize(size.Width, size.Height);
		contentOffset = m_contentOffset;
	}
}
