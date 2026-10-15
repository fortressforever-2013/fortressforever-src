/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_quantityitem.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_quantityitem
	file ext:	h
	author:		Elmo

	purpose:	Customisable Quanitity indicator
*********************************************************************/

#ifndef FF_QUANTITYTEXT_H
#define FF_QUANTITYTEXT_H


#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#include <string>
#pragma pop_macro("max")
#pragma pop_macro("min")

#include "vgui_controls/Panel.h"

#include "ff_quantityhelper.h"

namespace vgui
{
	class FFQuantityItem;

	class FFQuantityText : public Panel
	{
		using AnchorPosition = FFQuantityHelper::AnchorPosition;
		using HorizontalAlignment = FFQuantityHelper::HorizontalAlignment;
		using VerticalAlignment = FFQuantityHelper::VerticalAlignment;
		using Bounds = FFQuantityHelper::Bounds;
		using Position = FFQuantityHelper::Position;
		using Offset = FFQuantityHelper::Offset;
		using Scale = FFQuantityHelper::Scale;
		using Size = FFQuantityHelper::Size;

	private:
		DECLARE_CLASS_SIMPLE(FFQuantityText, Panel);

		Scale m_scale;
		bool m_bFontDirty = true;
		bool m_bPositionDirty = true;
		bool m_bParentTrigeredInvalidation = false;

		Size m_sizeScaled = Size(0, 0);
		Size m_maxSizeScaled = Size(0, 0);

		Position m_positionScaled = Position(0, 0);
		FFQuantityHelper::optional<Position> m_maxPositionScaled;

		Offset m_positionOffset = Offset(0, 0);

		AnchorPosition m_anchorPosition = FFQuantityHelper::ANCHORPOS_TOPRIGHT;
		Size m_anchorSize = Size(0, 0);

		HorizontalAlignment m_horizontalAlignment = FFQuantityHelper::ALIGN_RIGHT;
		VerticalAlignment m_verticalAlignment = FFQuantityHelper::ALIGN_BOTTOM;

		bool m_bShow = true;
		bool m_bTextShadow = false;
		bool m_bSticky = true;

		int m_iSize = 2;

		int m_iColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		Color m_clrCustom = Color(255, 255, 255, 255);
		Color m_clrText = Color(255, 255, 255, 255);
		Color m_clrTeam;
		Color m_clrTeamBright;
		Color m_clrFaded;
		Color m_clrStepped;

		HFont m_hfText = 0;
		HFont* m_hfFamily = nullptr;
		int m_iFontSizeCount;

		std::wstring m_wText;
		std::wstring m_wTextMax;

		FFQuantityHelper::Bounds GetRelativeBounds();

		FFQuantityHelper::Position RecalculatePositionCommon(
			const std::wstring& text,
			FFQuantityHelper::Size& sizeScaled);

		void RecalculateColor();

		Color GetRgbColor(
			int iColorMode,
			Color& clrCustom);

		MESSAGE_FUNC_INT_INT(
			OnScreenSizeChanged,
			"OnScreenSizeChanged",
			oldwide,
			oldtall);

	private:
		// These are all set in ApplyStyleData
		void SetCustomColor(Color clrCustom);

		void PerformLayout() override;
		void Paint() override;

		void OnSizeChanged(int wide, int tall) override;

	public:
		FFQuantityText(
			Panel* parent,
			const char* pElementName,
			int iFontSizeCount);

		bool SetAnchorOffset(
			Offset offset);
		bool SetAnchorPosition(
			AnchorPosition iAnchorPosition);
		bool SetAnchorAlignment(
			HorizontalAlignment iAlignHorizontally,
			VerticalAlignment iAlignVertically);

		void SetText(const std::wstring& szText);

		void SetTextMax(const std::wstring& szTextMax);
		void ClearTextMax();

		bool SetFontFamily(HFont* font, bool bForceRefresh = false);

		bool SetAnchorSize(Size anchorSize);

		void SetIntensityFadedColor(Color clrFaded);
		void SetIntensitySteppedColor(Color clrStepped);
		void SetTeamColor(Color clrTeam, Color clrTeamBright);

		bool ApplyStyleData(
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData);
	};
}

#endif
