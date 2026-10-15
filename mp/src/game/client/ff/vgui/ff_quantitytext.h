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


#include "string"

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
		bool m_bFontDirty;
		bool m_bPositionDirty;
		bool m_bParentTrigeredInvalidation;

		Size m_sizeScaled;
		Size m_maxSizeScaled;

		Position m_positionScaled;
		FFQuantityHelper::optional<Position> m_maxPositionScaled;

		Offset m_positionOffset;

		AnchorPosition m_anchorPosition;
		Size m_anchorSizeScaled;

		HorizontalAlignment m_horizontalAlignment;
		VerticalAlignment m_verticalAlignment;

		bool m_bShow;
		bool m_bTextShadow;
		bool m_bSticky;

		int m_iSize;

		int m_iColorMode;
		Color m_clrCustom;
		Color m_clrText;
		Color m_clrTeam;
		Color m_clrFaded;
		Color m_clrStepped;

		HFont m_hfText;
		HFont* m_hfFamily;

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

		MESSAGE_FUNC_INT_INT(OnScreenSizeChanged, "OnScreenSizeChanged", oldwide, oldtall);

	private:
		// These are all set in ApplyStyleData
		void SetCustomColor(Color clrCustom);

		void PerformLayout() override;
		void Paint() override;

		void OnSizeChanged(int wide, int tall) override;

	public:
		FFQuantityText(Panel* parent, const char* pElementName);

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

		bool SetFontFamily(HFont* font);

		bool SetAnchorSize(Size anchorSize);

		void SetIntensityFadedColor(Color clrFaded);
		void SetIntensitySteppedColor(Color clrStepped);
		void SetTeamColor(Color clrTeam);

		bool ApplyStyleData(
			KeyValues* kvStyleData,
			KeyValues* kvDefaultStyleData);
	};
}
#endif