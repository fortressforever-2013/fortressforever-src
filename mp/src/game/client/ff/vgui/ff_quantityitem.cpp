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
#include "ff_quantitypanel.h"
#include "ff_utils.h"

#include "vprof.h"

using namespace FFQuantityHelper;

namespace vgui
{
	template <typename T>
	T minOf(
		const T& a,
		const T& b,
		const T& c,
		const T& d) {
		return min(min(a, b), min(c, d));
	}

	template <typename T>
	T maxOf(
		const T& a,
		const T& b,
		const T& c,
		const T& d) {
		return max(max(a, b), max(c, d));
	}

	FFQuantityItem::FFQuantityItem(
		FFQuantityPanel* parent,
		const char* pElementName)
		: BaseClass(
			parent,
			pElementName),
		m_parentQuantityPanel(parent)
	{
		// Children
		m_pAmountText = new FFQuantityText(this, "AmountText", QUANTITYITEMTEXTSIZES);
		m_pIconText = new FFQuantityText(this, "IconText", QUANTITYITEMICONSIZES);
		m_pLabelText = new FFQuantityText(this, "LabelText", QUANTITYITEMTEXTSIZES);

		m_pAmountText->SetAnchorSize(m_barSize);
		m_pIconText->SetAnchorSize(m_barSize);
		m_pLabelText->SetAnchorSize(m_barSize);

		// Configure the child text anchors.
		m_pAmountText->SetAnchorAlignment(ALIGN_RIGHT, ALIGN_BOTTOM);
		m_pIconText->SetAnchorAlignment(ALIGN_RIGHT, ALIGN_MIDDLE);
		m_pLabelText->SetAnchorAlignment(ALIGN_LEFT, ALIGN_BOTTOM);

		m_pAmountText->SetAnchorOffset(Offset(0, 0));
		m_pIconText->SetAnchorOffset(Offset(5, 0));
		m_pLabelText->SetAnchorOffset(Offset(-5, 0));

		m_pAmountText->SetAnchorPosition(ANCHORPOS_TOPRIGHT);
		m_pIconText->SetAnchorPosition(ANCHORPOS_MIDDLELEFT);
		m_pLabelText->SetAnchorPosition(ANCHORPOS_TOPLEFT);

		// Compute initial quantity-dependent colours once so first paint is correct
		RecalculateQuantity();

		// Initialize scale once so first layout is correct
		m_scale = GetScaleFromScreenSize();
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

			m_hfVerdanaTextFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityItemVerdana%d", i), true);
			m_hfVerdanaTextFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityItemVerdanaShadow%d", i), true);
			m_hfVerdanaTextFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityItemVerdana%d", i), false);
		}

		for (int i = 0; i < QUANTITYITEMICONSIZES; ++i)
		{
			m_hfIconFamily[i * 3 + 0] = qbScheme->GetFont(VarArgs("QuantityItemIcon%d", i), true);
			m_hfIconFamily[i * 3 + 1] = qbScheme->GetFont(VarArgs("QuantityItemIconShadow%d", i), true);
			m_hfIconFamily[i * 3 + 2] = qbScheme->GetFont(VarArgs("QuantityItemIcon%d", i), false);
		}

		// Assign font families to child texts
		m_pAmountText->SetFontFamily(
			m_bAmountFontVerdana ? m_hfVerdanaTextFamily : m_hfTextFamily,
			true);
		m_pLabelText->SetFontFamily(
			m_bLabelFontVerdana ? m_hfVerdanaTextFamily : m_hfTextFamily,
			true);
		m_pIconText->SetFontFamily(m_hfIconFamily, true);

		BaseClass::ApplySchemeSettings(pScheme);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Rescale the bar and text components after a resolution change
	//-----------------------------------------------------------------------------
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

	//-----------------------------------------------------------------------------
	// Purpose: Apply temporary opacity to selected components without changing their style colours
	//-----------------------------------------------------------------------------
	void FFQuantityItem::SetOpacity(float flOpacity, OpacityTarget target)
	{
		m_flOpacity = flOpacity;
		m_opacityTarget = target;
		m_pIconText->SetAlpha(static_cast<int>(255 * GetOpacity(OPACITY_ICON)));
		m_pLabelText->SetAlpha(static_cast<int>(255 * GetOpacity(OPACITY_LABEL)));
		m_pAmountText->SetAlpha(static_cast<int>(255 * GetOpacity(OPACITY_AMOUNT)));
	}

	float FFQuantityItem::GetOpacity(OpacityTarget target) const
	{
		if (m_opacityTarget == OPACITY_ALL || m_opacityTarget == target)
			return m_flOpacity;

		return 1.0f;
	}

	Color FFQuantityItem::GetOpacityColor(Color color, OpacityTarget target) const
	{
		color.SetColor(
			color.r(),
			color.g(),
			color.b(),
			static_cast<int>(color.a() * GetOpacity(target)));
		return color;
	}

	void FFQuantityItem::Paint()
	{
		VPROF_BUDGET("FFQuantityItem::Paint", "QuantityItem");

		BaseClass::Paint();

		if (m_bShowBarBorder)
		{
			surface()->DrawSetColor(GetOpacityColor(m_clrBarBorder, OPACITY_BAR_BORDER));

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
			surface()->DrawSetColor(GetOpacityColor(m_clrBarBackground, OPACITY_BAR_BACKGROUND));
			surface()->DrawFilledRect(
				m_contentOffset.X,
				m_contentOffset.Y,
				m_contentOffset.X + ScaleX(m_barSize.Width),
				m_contentOffset.Y + ScaleY(m_barSize.Height));
		}

		if (m_bShowBar)
		{
			surface()->DrawSetColor(GetOpacityColor(m_clrBar, OPACITY_BAR));
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

	bool FFQuantityItem::IsDisabled() const
	{
		return m_bDisabled;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Refresh text, fill and colours while retaining the gameplay amount during preview
	//-----------------------------------------------------------------------------
	void FFQuantityItem::SetAmount(
		float iAmount)
	{
		if (!m_bUpdatingPreviewAmount)
			m_flNonPreviewAmount = iAmount;

		const bool bHasChanged
			= Change(m_flAmount, iAmount);

		if (bHasChanged)
		{
			RecalculateAmountFormat();
			RecalculateQuantity();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Animate a sample quantity with a per-item time offset without replacing the gameplay amount
	//-----------------------------------------------------------------------------
	void FFQuantityItem::AnimatePreviewAmount(float flTime)
	{
		if (m_flPreviewTimeOffset < 0.0f)
			m_flPreviewTimeOffset = RandomFloat(0.0f, PREVIEW_CYCLE_DURATION);

		const float flPhase = (flTime + m_flPreviewTimeOffset) * (2.0f * M_PI / PREVIEW_CYCLE_DURATION);
		const float flFraction = 0.5f - 0.5f * cosf(flPhase);
		m_bUpdatingPreviewAmount = true;
		SetAmount(flFraction * m_flMaxAmount);
		m_bUpdatingPreviewAmount = false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Restore the gameplay amount and discard the previous preview animation offset
	//-----------------------------------------------------------------------------
	void FFQuantityItem::RestorePreviewAmount()
	{
		m_flPreviewTimeOffset = -1.0f;
		SetAmount(m_flNonPreviewAmount);
	}

	void FFQuantityItem::SetAmountMax(
		float iAmountMax)
	{
		const bool bHasChanged
			= Change(m_flMaxAmount, iAmountMax);

		if (bHasChanged)
		{
			RecalculateAmountMaxDisplay();
			RecalculateAmountFormat();
			RecalculateQuantity();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reserve extra digits above maximum without changing bar fill or intensity thresholds
	//-----------------------------------------------------------------------------
	void FFQuantityItem::SetAmountOverMax(float flAmountOverMax)
	{
		if (Change(m_flAmountOverMax, Max(0.0f, flAmountOverMax)))
			RecalculateAmountMaxDisplay();
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

	bool FFQuantityItem::SetAmountFormat(
		AmountFormat amountDisplay)
	{
		const bool bHasChanged
			= Change(m_amountFormat, amountDisplay);

		if (bHasChanged)
		{
			RecalculateAmountFormat();
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
			iColorModeBar,
			m_barIntensity);
	}

	void FFQuantityItem::SetBarBorderColorMode(
		const int iColorModeBarBorder)
	{
		SetColorMode(
			m_iBarBorderColorMode,
			m_clrBarBorder,
			m_clrBarBorderCustom,
			iColorModeBarBorder,
			m_borderIntensity);
	}

	void FFQuantityItem::SetBarBackgroundColorMode(
		const int iColorModeBarBackground)
	{
		SetColorMode(
			m_iBarBackgroundColorMode,
			m_clrBarBackground,
			m_clrBarBackgroundCustom,
			iColorModeBarBackground,
			m_backgroundIntensity);
	}

	void FFQuantityItem::SetColorMode(
		int& iColorMode,
		Color& clrFinal,
		const Color& clrCustom,
		const int iColorModeNew,
		const IntensityPalette& palette)
	{
		const bool bHasChanged
			= Change(iColorMode, iColorModeNew);

		if (bHasChanged)
		{
			RecalculateColor(
				clrFinal,
				iColorMode,
				clrCustom,
				palette);
		}
	}

	void FFQuantityItem::SetCustomBarColor(
		const int r,
		const int g,
		const int b,
		const int a)
	{
		SetCustomColor(
			m_clrBarCustom,
			m_clrBar,
			m_iBarColorMode,
			r,
			g,
			b,
			a,
			m_barIntensity);
	}

	void FFQuantityItem::SetCustomBarBorderColor(
		const int r,
		const int g,
		const int b,
		const int a)
	{
		SetCustomColor(
			m_clrBarBorderCustom,
			m_clrBarBorder,
			m_iBarBorderColorMode,
			r,
			g,
			b,
			a,
			m_borderIntensity);
	}

	void FFQuantityItem::SetCustomBarBackgroundColor(
		const int r,
		const int g,
		const int b,
		const int a)
	{
		SetCustomColor(
			m_clrBarBackgroundCustom,
			m_clrBarBackground,
			m_iBarBackgroundColorMode,
			r,
			g,
			b,
			a,
			m_backgroundIntensity);
	}

	void FFQuantityItem::SetCustomColor(
		Color& clrCustom,
		Color& clrFinal,
		int iColorMode,
		const int r,
		const int g,
		const int b,
		const int a,
		const IntensityPalette& palette)
	{
		const Color clrNew(
			r,
			g,
			b,
			a);

		const bool bHasChanged
			= Change(clrCustom, clrNew);

		if (bHasChanged)
		{
			RecalculateColor(
				clrFinal,
				iColorMode,
				clrCustom,
				palette);
		}
	}

	bool FFQuantityItem::SetTeamColor(
		Color clrTeam,
		Color clrTeamBright)
	{
		bool bHasChanged
			= Change(m_clrTeam, clrTeam);
		bHasChanged |= Change(m_clrTeamBright, clrTeamBright);

		if (!bHasChanged)
			return false;

		if (m_iBarColorMode == COLOR_MODE_TEAM || m_iBarColorMode == COLOR_MODE_TEAM_BRIGHT)
			RecalculateColor(
				m_clrBar,
				m_iBarColorMode,
				m_clrBarCustom,
				m_barIntensity);

		if (m_iBarBorderColorMode == COLOR_MODE_TEAM || m_iBarBorderColorMode == COLOR_MODE_TEAM_BRIGHT)
			RecalculateColor(
				m_clrBarBorder,
				m_iBarBorderColorMode,
				m_clrBarBorderCustom,
				m_borderIntensity);

		if (m_iBarBackgroundColorMode == COLOR_MODE_TEAM || m_iBarBackgroundColorMode == COLOR_MODE_TEAM_BRIGHT)
			RecalculateColor(
				m_clrBarBackground,
				m_iBarBackgroundColorMode,
				m_clrBarBackgroundCustom,
				m_backgroundIntensity);

		m_pAmountText->SetTeamColor(clrTeam, clrTeamBright);
		m_pIconText->SetTeamColor(clrTeam, clrTeamBright);
		m_pLabelText->SetTeamColor(clrTeam, clrTeamBright);
		return true;
	}

	float FFQuantityItem::GetAmount() const { return m_flAmount; }
	float FFQuantityItem::GetAmountMax() const { return m_flMaxAmount; }
	int FFQuantityItem::GetAmountAsInt() const { return static_cast<int>(m_flAmount); }

	void FFQuantityItem::SetIntensityLimits(
		int r,
		int o,
		int y,
		int g)
	{
		m_iIntensityRed = r; m_iIntensityOrange = o; m_iIntensityYellow = y; m_iIntensityGreen = g;
		RecalculateQuantity();
	}

	void FFQuantityItem::SetIntensityAmountScaled(bool bScaled)
	{
		m_bIntensityAmountScaled = bScaled;
		RecalculateQuantity();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Apply bar and text settings and report whether the parent must reposition its items
	//-----------------------------------------------------------------------------
	bool FFQuantityItem::ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData)
	{
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

			bInvalidateLayout = true;
		}

		if (iBarBorderWidth != -1
			&& Change(m_iBarBorderWidth, iBarBorderWidth))
		{
			bInvalidateLayout = true;
		}

		if (iBarOrientation != -1)
			m_iBarOrientation = iBarOrientation;

		// Bar components
		if (KeyValues* kvBar = GetData("Bar", kvStyleData, kvDefaultStyleData))
		{
			m_barIntensity.Apply(
				kvBar,
				kvDefaultStyleData->FindKey("Bar"),
				GetInt("alpha", kvBar, m_clrBarCustom.a()));

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
				SetCustomBarColor(
					iRed,
					iGreen,
					iBlue,
					iAlpha);
			}
		}

		if (KeyValues* kvBarBorder = GetData("BarBorder", kvStyleData, kvDefaultStyleData))
		{
			m_borderIntensity.Apply(
				kvBarBorder,
				kvDefaultStyleData->FindKey("BarBorder"),
				GetInt("alpha", kvBarBorder, m_clrBarBorderCustom.a()));

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
				SetCustomBarBorderColor(
					iRed,
					iGreen,
					iBlue,
					iAlpha);
			}
		}

		if (KeyValues* kvBarBg = GetData("BarBackground", kvStyleData, kvDefaultStyleData))
		{
			m_backgroundIntensity.Apply(
				kvBarBg,
				kvDefaultStyleData->FindKey("BarBackground"),
				GetInt("alpha", kvBarBg, m_clrBarBackgroundCustom.a()));

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
				SetCustomBarBackgroundColor(
					iRed,
					iGreen,
					iBlue,
					iAlpha);
			}
		}

		// Text components
		if (KeyValues* kvIcon = GetData("Icon", kvStyleData, kvDefaultStyleData)) {
			KeyValues* kvDefaultIcon = kvDefaultStyleData->FindKey("Icon");

			if (m_pIconText->ApplyStyleData(
				kvIcon,
				kvDefaultIcon))
			{
				bInvalidateLayout = true;
			}
		}

		if (KeyValues* kvLabel = GetData("Label", kvStyleData, kvDefaultStyleData)) {
			KeyValues* kvDefaultLabel = kvDefaultStyleData->FindKey("Label");

			if (m_pLabelText->ApplyStyleData(
				kvLabel,
				kvDefaultLabel))
			{
				bInvalidateLayout = true;
			}

			const int iFontVerdana = GetInt(
				"fontVerdana",
				kvLabel,
				kvDefaultLabel);
			if (iFontVerdana != -1)
			{
				m_bLabelFontVerdana = iFontVerdana == 1;
				if (m_pLabelText->SetFontFamily(
					m_bLabelFontVerdana ? m_hfVerdanaTextFamily : m_hfTextFamily))
				{
					bInvalidateLayout = true;
				}
			}
		}

		if (KeyValues* kvAmount = GetData("Amount", kvStyleData, kvDefaultStyleData)) {
			KeyValues* kvDefaultAmount = kvDefaultStyleData->FindKey("Amount");

			if (m_pAmountText->ApplyStyleData(
				kvAmount,
				kvDefaultAmount))
			{
				bInvalidateLayout = true;
			}

			const int iFontVerdana = GetInt(
				"fontVerdana",
				kvAmount,
				kvDefaultAmount);
			if (iFontVerdana != -1)
			{
				m_bAmountFontVerdana = iFontVerdana == 1;
				if (m_pAmountText->SetFontFamily(
					m_bAmountFontVerdana ? m_hfVerdanaTextFamily : m_hfTextFamily))
				{
					bInvalidateLayout = true;
				}
			}
		}

		if (bInvalidateLayout) {
			// Suppress only this immediate layout. If the scheme defers it,
			// the later size change must still notify the parent.
			m_bParentTrigeredInvalidation = true;
			InvalidateLayout(true);
			m_bParentTrigeredInvalidation = false;
		}

		// Palette edits can change the colour without changing the amount or layout.
		RecalculateQuantity();

		return bInvalidateLayout;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Enclose the bar and text bounds and shift their positions into non-negative item coordinates
	//-----------------------------------------------------------------------------
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

		const float minX0 = minOf(
			barX0,
			amountBounds.X0,
			iconBounds.X0,
			labelBounds.X0);
		const float minY0 = minOf(
			barY0,
			amountBounds.Y0,
			iconBounds.Y0,
			labelBounds.Y0);

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

		const float maxX1 = maxOf(
			barX1,
			amountBounds.X1,
			iconBounds.X1,
			labelBounds.X1);
		const float maxY1 = maxOf(
			barY1,
			amountBounds.Y1,
			iconBounds.Y1,
			labelBounds.Y1);

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

		RecalculateAmountFormat();
		RecalculateAmountMaxDisplay();
	}

	inline int CeilToInt(float x)
	{
		return static_cast<int>(ceilf(x));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Format the displayed amount as a raw value, percentage or amount over maximum
	//-----------------------------------------------------------------------------
	void FFQuantityItem::RecalculateAmountFormat()
	{
		std::wstring text = L"";
		float flAmount = m_flAmount;

		switch (m_amountFormat)
		{
		default:
		case FORMAT_RAW:
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

		case FORMAT_PERCENTAGE:
			AssertMsg(m_flMaxAmount != 0, "Cannot calculate a quantity percentage with a zero maximum.");
			if (m_flMaxAmount == 0)
				break; // Leave percentage text empty when the calculation is undefined.

			text
				= std::to_wstring(CeilToInt((flAmount * 100) / m_flMaxAmount))
				+ L"%";
			break;

		case FORMAT_OVER_MAX:
			text
				= std::to_wstring(CeilToInt(flAmount))
				+ L"/"
				+ std::to_wstring(CeilToInt(m_flMaxAmount));
			break;
		}

		m_pAmountText->SetText(text);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reserve text bounds for the selected format and permitted over-maximum value
	//-----------------------------------------------------------------------------
	void FFQuantityItem::RecalculateAmountMaxDisplay()
	{
		// OverMax reserves text space without changing the bar maximum or intensity scale.
		const float flDisplayMax = Max(m_flMaxAmount, m_flAmountOverMax);

		switch (m_amountFormat)
		{
		default:
		case FORMAT_RAW:
			if (m_flAmountOverMax > 0.0f)
			{
				wchar_t buffer[32];
				swprintf(
					buffer,
					ARRAYSIZE(buffer),
					L"%.*f",
					m_iAmountDecimalPlaces,
					flDisplayMax);
				m_pAmountText->SetTextMax(buffer);
			}
			else
				m_pAmountText->ClearTextMax();

			break;

		case FORMAT_PERCENTAGE:
			m_pAmountText->SetTextMax(
				std::to_wstring(m_flMaxAmount > 0.0f
					? CeilToInt(flDisplayMax * 100.0f / m_flMaxAmount) : 100)
				+ L"%");
			break;

		case FORMAT_OVER_MAX:
			m_pAmountText->SetTextMax(
				std::to_wstring(CeilToInt(flDisplayMax))
				+ L"/"
				+ std::to_wstring(static_cast<int>(m_flMaxAmount)));
			break;
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update bar fill and component intensity colours from the current amount
	//-----------------------------------------------------------------------------
	void FFQuantityItem::RecalculateQuantity()
	{
		AssertMsg(m_flMaxAmount != 0, "Cannot calculate quantity colours or bar fill with a zero maximum.");
		if (m_flMaxAmount == 0)
			return;

		const int iAmountPercent = (m_flAmount * 100) / m_flMaxAmount;

		SetIntensityColor(
			m_clrBarBorder,
			m_iBarBorderColorMode,
			m_borderIntensity);
		SetIntensityColor(
			m_clrBarBackground,
			m_iBarBackgroundColorMode,
			m_backgroundIntensity);
		SetIntensityColor(
			m_clrBar,
			m_iBarColorMode,
			m_barIntensity);

		FFQuantityText* texts[] = { m_pAmountText, m_pIconText, m_pLabelText };
		for (FFQuantityText* text : texts)
		{
			text->SetIntensityColors(
				m_bIntensityAmountScaled ? iAmountPercent : static_cast<int>(m_flAmount),
				m_iIntensityRed,
				m_iIntensityOrange,
				m_iIntensityYellow,
				m_iIntensityGreen,
				m_bIntensityInvertScale);
		}

		// Quantity fill offsets relative to bar
		m_iBarX0QuantityOffset = 0;
		m_iBarX1QuantityOffset = 0;
		m_iBarY0QuantityOffset = 0;
		m_iBarY1QuantityOffset = 0;

		// Over-limit amounts still display in full, but the fill stays inside the bar.
		const float flBarFraction = clamp(m_flAmount / m_flMaxAmount, 0.0f, 1.0f);

		switch (m_iBarOrientation)
		{
		case ORIENTATION_HORIZONTAL_INVERTED:
			m_iBarX0QuantityOffset = m_barSize.Width - (m_barSize.Width * flBarFraction);
			break;
		case ORIENTATION_VERTICAL:
			m_iBarY0QuantityOffset = m_barSize.Height - (m_barSize.Height * flBarFraction);
			break;
		case ORIENTATION_VERTICAL_INVERTED:
			m_iBarY1QuantityOffset = (m_barSize.Height * flBarFraction) - m_barSize.Height;
			break;
		case ORIENTATION_HORIZONTAL:
		default:
			m_iBarX1QuantityOffset = (m_barSize.Width * flBarFraction) - m_barSize.Width;
			break;
		}
	}

	Color FFQuantityItem::GetIntensityColor(int iColorMode, const IntensityPalette& palette) const
	{
		const float flIntensity = m_bIntensityAmountScaled && m_flMaxAmount > 0.0f
			? m_flAmount * 100 / m_flMaxAmount : m_flAmount;

		return palette.Evaluate(
			static_cast<int>(flIntensity),
			iColorMode == COLOR_MODE_STEPPED,
			m_iIntensityRed,
			m_iIntensityOrange,
			m_iIntensityYellow,
			m_iIntensityGreen,
			m_bIntensityInvertScale);
	}

	void FFQuantityItem::SetIntensityColor(
		Color& color,
		int iColorMode,
		const IntensityPalette& palette)
	{
		if (iColorMode == COLOR_MODE_FADED || iColorMode == COLOR_MODE_STEPPED)
			color = GetIntensityColor(iColorMode, palette);
	}

	void FFQuantityItem::RecalculateColor(
		Color& clrFinal,
		const int iColorMode,
		const Color& clrCustom,
		const IntensityPalette& palette)
	{
		const Color rgb
			= GetRgbColor(
				iColorMode,
				clrCustom,
				palette);

		if (iColorMode == COLOR_MODE_FADED || iColorMode == COLOR_MODE_STEPPED)
			clrFinal = rgb;
		else
			SetColor(
				clrFinal,
				rgb,
				clrCustom);
	}

	Color FFQuantityItem::GetRgbColor(
		const int iColorMode,
		const Color& colorCustom,
		const IntensityPalette& palette) const
	{
		switch (iColorMode)
		{
		case COLOR_MODE_STEPPED:
		case COLOR_MODE_FADED:
			return GetIntensityColor(iColorMode, palette);
		case COLOR_MODE_TEAM_BRIGHT:
			return m_clrTeamBright;

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

	//-----------------------------------------------------------------------------
	// Purpose: Notify the parent panel about item size changes without repeating parent-triggered layout
	//-----------------------------------------------------------------------------
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

		m_parentQuantityPanel->InvalidateItemLayout();
	}

	void FFQuantityItem::GetContentMetrics(
		Size& size,
		Offset& contentOffset)
	{
		GetSize(size.Width, size.Height);
		contentOffset = m_contentOffset;
	}
}
