/********************************************************************
	created:	2020/11
	filename: 	cl_dll\ff\vgui\ff_quantitytext.cpp
	file path:	cl_dll\ff\vgui
	file base:	ff_quantitytext
	file ext:	cpp
	author:		Elmo

	purpose:	Quantity panel component to render customized text
*********************************************************************/

#include "cbase.h"

#include "ff_quantitytext.h"
#include "vgui/ISurface.h"

#include "vprof.h"

using namespace FFQuantityHelper;

namespace vgui
{
	FFQuantityText::FFQuantityText(
			Panel* parent,
			const char* panelName,
			int iFontSizeCount)
		: BaseClass(parent, panelName),
		m_iFontSizeCount(iFontSizeCount)
	{
		m_scale = GetScaleFromScreenSize();
	}

	void FFQuantityText::Paint()
	{
		VPROF_BUDGET("FFQuantityText::Paint", "QuantityText");

		BaseClass::Paint();

		if (!m_bShow || m_wText.empty())
			return;

		if (m_bSticky && m_maxPositionScaled.has_value())
		{
			surface()->DrawSetTextPos(
				m_positionScaled.X - m_maxPositionScaled->X,
				m_positionScaled.Y - m_maxPositionScaled->Y);
		}
		else
		{
			surface()->DrawSetTextPos(0, 0);
		}

		surface()->DrawSetTextColor(m_clrText);
		surface()->DrawSetTextFont(m_hfText);
		surface()->DrawUnicodeString(m_wText.c_str());
	}

	void FFQuantityText::OnScreenSizeChanged(
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
		m_bFontDirty = true;
		m_bPositionDirty = true;
		InvalidateLayout();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Report the reserved text bounds used to size and position the parent item
	//-----------------------------------------------------------------------------
	Bounds FFQuantityText::GetRelativeBounds() const
	{
		if (!m_bShow)
			return Bounds();

		// If textMax exists and we are sticky, use the max rect so layout never shrinks
		if (m_bSticky && m_maxPositionScaled.has_value())
		{
			return Bounds(
				m_maxPositionScaled.value(),
				m_maxSizeScaled);
		}

		// Otherwise use the current text rect
		return Bounds(
			m_positionScaled,
			m_sizeScaled);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Resolve dirty font and alignment settings, then update actual and reserved text bounds
	//-----------------------------------------------------------------------------
	void FFQuantityText::PerformLayout()
	{
		// Let base do any housekeeping (safe/no-op in many cases)
		BaseClass::PerformLayout();

		// Can only calculate font if we have a font family
		if (m_bFontDirty && m_hfFamily)
		{
			m_hfText
				= GetFont(
					m_hfFamily,
					m_iSize,
					m_bTextShadow,
					m_iFontSizeCount);

			m_bPositionDirty = true;
		}

		// We've calculated all we can at this point
		m_bFontDirty = false;

		if (!m_bPositionDirty)
			return;

		//If font is not valid, we can't do anything
		if (m_wText.empty() || !m_hfText)
		{
			m_positionScaled = Position();
			m_sizeScaled = Size();
			m_maxSizeScaled = Size();
			m_maxPositionScaled.reset();
		}
		else
		{
			m_positionScaled
				= RecalculatePositionCommon(
					m_wText,
					m_sizeScaled);

			if (m_wTextMax.empty())
			{
				m_maxPositionScaled.reset();
			}
			else
			{
				m_maxPositionScaled.set(
					RecalculatePositionCommon(
						m_wTextMax,
						m_maxSizeScaled));
			}
		}

		m_bPositionDirty = false;

		Size currentSize = Size();

		GetSize(currentSize.Width, currentSize.Height);

		Size desiredSize
			= m_bSticky && m_maxPositionScaled.has_value()
			? m_maxSizeScaled
			: m_sizeScaled;

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

	//-----------------------------------------------------------------------------
	// Purpose: Measure a string and align it to the scaled anchor using the current text settings
	//-----------------------------------------------------------------------------
	Position FFQuantityText::RecalculatePositionCommon(
		const std::wstring& text,
		Size& sizeScaled)
	{
		// Get the text size based on the provided string
		surface()->GetTextSize(
			m_hfText,
			text.c_str(),
			sizeScaled.Width,
			sizeScaled.Height);

		return CalculatePosition(
			m_anchorPosition,
			m_anchorSize.Scaled(m_scale),
			sizeScaled,
			m_horizontalAlignment,
			m_verticalAlignment,
			m_positionOffset.Scaled(m_scale));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the displayed string while preserving reserved maximum bounds when possible
	//-----------------------------------------------------------------------------
	void FFQuantityText::SetText(
		const std::wstring& wText)
	{
		if (m_wText == wText)
			return;

		m_wText = wText;
		if (m_bSticky && m_maxPositionScaled.has_value() && !m_wText.empty()
			&& !m_bFontDirty && !m_bPositionDirty)
		{
			// Only the text's offset changes inside the already reserved maximum bounds.
			m_positionScaled = RecalculatePositionCommon(m_wText, m_sizeScaled);
			return;
		}

		m_bPositionDirty = true;
		InvalidateLayout();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reserve space for the widest expected amount so changing values do not resize the item
	//-----------------------------------------------------------------------------
	void FFQuantityText::SetTextMax(
		const std::wstring& wTextMax)
	{
		if (m_wTextMax == wTextMax)
			return;

		m_wTextMax = wTextMax;
		m_bPositionDirty = true;
		InvalidateLayout();
	}

	void FFQuantityText::ClearTextMax()
	{
		m_wTextMax.clear();
		m_bPositionDirty = true;
		InvalidateLayout();
	}

	bool FFQuantityText::SetFontFamily(
		const HFont* hfFamily,
		bool bForceRefresh)
	{
		const bool bHasChanged
			= Change(m_hfFamily, hfFamily);

		if (bHasChanged || bForceRefresh)
		{
			m_hfFamily = hfFamily;
			m_bFontDirty = true;
			InvalidateLayout();
		}

		return bHasChanged || bForceRefresh;
	}

	bool FFQuantityText::SetAnchorAlignment(
		HorizontalAlignment horizontalAlignment,
		VerticalAlignment vertialAlignment)
	{
		const bool bHorizChanged
			= Change(m_horizontalAlignment, horizontalAlignment);

		const bool bVertChanged
			= Change(m_verticalAlignment, vertialAlignment);

		if (bHorizChanged || bVertChanged)
		{
			m_bPositionDirty = true;
			InvalidateLayout();
		}

		return bHorizChanged || bVertChanged;
	}

	bool FFQuantityText::SetAnchorOffset(
		Offset offset)
	{
		const bool bHasChanged
			= Change(m_positionOffset, offset);

		if (bHasChanged)
		{
			m_bPositionDirty = true;
			InvalidateLayout();
		}

		return bHasChanged;
	}

	bool FFQuantityText::SetAnchorPosition(
		AnchorPosition anchorPosition)
	{
		const bool bHasChanged
			= Change(m_anchorPosition, anchorPosition);

		if (bHasChanged)
		{
			m_bPositionDirty = true;
			InvalidateLayout();
		}

		return bHasChanged;
	}

	bool FFQuantityText::SetAnchorSize(
		Size anchorSize)
	{
		const bool bHasChanged
			= Change(m_anchorSize, anchorSize);

		if (bHasChanged)
		{
			m_bPositionDirty = true;
			InvalidateLayout();
		}

		return bHasChanged;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Apply supplied text settings and propagate font or position changes to the parent item
	//-----------------------------------------------------------------------------
	bool FFQuantityText::ApplyStyleData(
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData)
	{
		m_intensity.Apply(
			kvStyleData,
			kvDefaultStyleData,
			m_clrCustom.a());

		const int iShow = GetInt("show", kvStyleData, kvDefaultStyleData);
		const int iShadow = GetInt("shadow", kvStyleData, kvDefaultStyleData);
		const int iSize = GetInt("size", kvStyleData, kvDefaultStyleData);
		const int iSticky = GetInt("sticky", kvStyleData, kvDefaultStyleData);
		const int iAnchorPosition = GetInt("anchorPosition", kvStyleData, kvDefaultStyleData);
		const int iAlignHorizontally = GetInt("alignH", kvStyleData, kvDefaultStyleData);
		const int iAlignVertically = GetInt("alignV", kvStyleData, kvDefaultStyleData);
		const int iX = GetInt(
			"offsetX",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		const int iY = GetInt(
			"offsetY",
			kvStyleData,
			kvDefaultStyleData,
			-9999);
		const int iColorMode = GetInt("colorMode", kvStyleData, kvDefaultStyleData);
		const int iRed = GetInt("red", kvStyleData, kvDefaultStyleData);
		const int iGreen = GetInt("green", kvStyleData, kvDefaultStyleData);
		const int iBlue = GetInt("blue", kvStyleData, kvDefaultStyleData);
		const int iAlpha = GetInt("alpha", kvStyleData, kvDefaultStyleData);

		if (iShow != -1
			&& Change(m_bShow, iShow == 1))
		{
			m_bPositionDirty = true;
		}

		if (iShadow != -1
			&& Change(m_bTextShadow, iShadow == 1))
		{
			m_bFontDirty = true;
		}

		if (iSize != -1
			&& Change(m_iSize, iSize))
		{
			m_bFontDirty = true;
		}

		if (iSticky != -1
			&& Change(m_bSticky, iSticky == 1))
		{
			m_bPositionDirty = true;
		}

		if (iAnchorPosition != -1)
		{
			// public method already sets position dirty
			SetAnchorPosition(
				static_cast<AnchorPosition>(iAnchorPosition));
		}

		if (iAlignHorizontally != -1 && iAlignVertically != -1)
		{
			// public method already sets position dirty
			SetAnchorAlignment(
				static_cast<HorizontalAlignment>(iAlignHorizontally),
				static_cast<VerticalAlignment>(iAlignVertically));
		}

		if (iX != -9999 && iY != -9999)
		{
			// public method already sets position dirty
			SetAnchorOffset(
				Offset(iX, iY));
		}

		bool bRecalculateColor = false;

		if (iColorMode != -1
			&& Change(
				m_iColorMode,
				iColorMode))
		{
			bRecalculateColor = true;
		}

		if (iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1
			&& Change(
				m_clrCustom,
				Color(
					iRed,
					iGreen,
					iBlue,
					iAlpha)))
		{
			bRecalculateColor = true;
		}

		if (bRecalculateColor)
			RecalculateColor();

		if (!m_bFontDirty && !m_bPositionDirty)
			return false;

		// A layout deferred until the scheme is ready must notify the item later.
		m_bParentTrigeredInvalidation = true;
		InvalidateLayout(true);
		m_bParentTrigeredInvalidation = false;
		return true;
	}

	void FFQuantityText::RecalculateColor()
	{
		Color clrRgb;

		switch (m_iColorMode)
		{
		case COLOR_MODE_TEAM_BRIGHT:
			clrRgb = m_clrTeamBright;
			break;

		case COLOR_MODE_TEAM:
			clrRgb = m_clrTeam;
			break;

		case COLOR_MODE_FADED:
			clrRgb = m_clrFaded;
			break;

		case COLOR_MODE_STEPPED:
			clrRgb = m_clrStepped;
			break;

		case COLOR_MODE_CUSTOM:
		default:
			clrRgb = m_clrCustom;
		}

		if (m_iColorMode != COLOR_MODE_FADED && m_iColorMode != COLOR_MODE_STEPPED)
			clrRgb.setA(m_clrCustom.a());

		m_clrText = clrRgb;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Calculate text colours from the current quantity and its intensity thresholds
	//-----------------------------------------------------------------------------
	void FFQuantityText::SetIntensityColors(
		int iValue,
		int iRed,
		int iOrange,
		int iYellow,
		int iGreen,
		bool bInvert)
	{
		SetIntensityFadedColor(m_intensity.Evaluate(
			iValue,
			false,
			iRed,
			iOrange,
			iYellow,
			iGreen,
			bInvert));
		SetIntensitySteppedColor(m_intensity.Evaluate(
			iValue,
			true,
			iRed,
			iOrange,
			iYellow,
			iGreen,
			bInvert));
	}

	void FFQuantityText::SetIntensityFadedColor(
		Color clrIntensityFaded)
	{
		const bool bHasChanged
			= Change(m_clrFaded, clrIntensityFaded);

		if (bHasChanged)
		{
			RecalculateColor();
		}
	}

	void FFQuantityText::SetIntensitySteppedColor(
		Color clrIntensityStepped)
	{
		const bool bHasChanged
			= Change(m_clrStepped, clrIntensityStepped);

		if (bHasChanged)
		{
			RecalculateColor();
		}
	}

	void FFQuantityText::SetTeamColor(
		Color clrTeam,
		Color clrTeamBright)
	{
		bool bHasChanged
			= Change(m_clrTeam, clrTeam);
		bHasChanged |= Change(m_clrTeamBright, clrTeamBright);

		if (bHasChanged)
		{
			RecalculateColor();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Notify the parent item about text size changes without repeating parent-triggered layout
	//-----------------------------------------------------------------------------
	void FFQuantityText::OnSizeChanged(
		int wide,
		int tall)
	{
		Panel::OnSizeChanged(wide, tall);

		if (m_bParentTrigeredInvalidation)
		{
			m_bParentTrigeredInvalidation = false;
			return;
		}

		GetParent()->InvalidateLayout();
	}
}
