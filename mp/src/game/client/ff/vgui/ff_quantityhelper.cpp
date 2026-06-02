#include "cbase.h"
#include "ff_quantityhelper.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>

namespace FFQuantityHelper
{
	vgui::HFont GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier)
	{
		int iModifier
			= bUseModifier ? 1 : 0;

		int iFontIndex
			= iSize * 3 + iModifier;

		return hfFamily[iFontIndex];
	}

	int GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		int iDefaultValue)
	{
		return kvStyleData->GetInt(keyName, iDefaultValue);
	}

	int GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData,
		int iDefaultValue)
	{
		iDefaultValue = kvDefaultStyleData->GetInt(keyName, iDefaultValue);
		return kvStyleData->GetInt(keyName, iDefaultValue);
	}

	KeyValues* GetData(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData)
	{
		kvStyleData = kvStyleData->FindKey(keyName);

		if (kvStyleData)
		{
			return kvStyleData;
		}

		return kvDefaultStyleData->FindKey(keyName);
	}

	void CombineHash(int& iHash, int iValue)
	{
		iHash = iHash * 23 + iValue;
	}

	Offset CalculateAnchorOffset(
		Size anchorSize,
		AnchorPosition anchorPosition)
	{
		int iAlignHorizontally, iAlignVertically;

		ConvertToAlignment(
			anchorPosition,
			iAlignHorizontally,
			iAlignVertically);

		return CalculatePositionOffset(
			anchorSize,
			iAlignHorizontally,
			iAlignVertically);
	}

	Position CalculatePosition(
		AnchorPosition anchorPosition,
		Size anchorSize,
		Size size,
		int horizontalAlignment,
		int verticalAlignment,
		Offset offset)
	{
		// Calculate the anchor offset
		Offset anchorOffset
			= CalculateAnchorOffset(
				anchorSize,
				anchorPosition);

		// Calculate the alignment offset
		Offset alignmentOffset
			= CalculatePositionOffset(
				size,
				horizontalAlignment,
				verticalAlignment);

		// Return the computed scaled position
		return Position(
			offset.X
			+ anchorOffset.X
			- alignmentOffset.X,
			offset.Y
			+ anchorOffset.Y
			- alignmentOffset.Y);
	}

	void ConvertToAlignment(
		AnchorPosition anchorPosition,
		int& iAlignHoriz,
		int& iAlignVert)
	{
		switch (anchorPosition)
		{
		case ANCHORPOS_TOPLEFT:
			iAlignVert = ALIGN_TOP;
			iAlignHoriz = ALIGN_LEFT;
			break;
		case ANCHORPOS_TOPCENTER:
			iAlignVert = ALIGN_TOP;
			iAlignHoriz = ALIGN_CENTER;
			break;
		case ANCHORPOS_TOPRIGHT:
			iAlignVert = ALIGN_TOP;
			iAlignHoriz = ALIGN_RIGHT;
			break;
		case ANCHORPOS_MIDDLELEFT:
			iAlignVert = ALIGN_MIDDLE;
			iAlignHoriz = ALIGN_LEFT;
			break;
		case ANCHORPOS_MIDDLECENTER:
			iAlignVert = ALIGN_MIDDLE;
			iAlignHoriz = ALIGN_CENTER;
			break;
		case ANCHORPOS_MIDDLERIGHT:
			iAlignVert = ALIGN_MIDDLE;
			iAlignHoriz = ALIGN_RIGHT;
			break;
		case ANCHORPOS_BOTTOMLEFT:
			iAlignVert = ALIGN_BOTTOM;
			iAlignHoriz = ALIGN_LEFT;
			break;
		case ANCHORPOS_BOTTOMCENTER:
			iAlignVert = ALIGN_BOTTOM;
			iAlignHoriz = ALIGN_CENTER;
			break;
		case ANCHORPOS_BOTTOMRIGHT:
			iAlignVert = ALIGN_BOTTOM;
			iAlignHoriz = ALIGN_RIGHT;
			break;
		}
	}

	Offset CalculatePositionOffset(
		Size size,
		int iAlignHoriz,
		int iAlignVert)
	{
		int iX, iY;

		switch (iAlignHoriz)
		{
		case ALIGN_CENTER:
			iX = size.Width / 2;
			break;
		case ALIGN_RIGHT:
			iX = size.Width;
			break;
		case ALIGN_LEFT:
		default:
			iX = 0;
			break;
		}

		switch (iAlignVert)
		{
		case ALIGN_MIDDLE:
			iY = size.Height / 2;
			break;
		case ALIGN_BOTTOM:
			iY = size.Height;
			break;
		case ALIGN_TOP:
		default:
			iY = 0;
			break;
		}

		return Offset(iX, iY);
	}

	const Scale GetScaleFromScreenSize()
	{
		int iScreenWide = 0, iScreenTall = 0;

		vgui::surface()->GetScreenSize(
			iScreenWide,
			iScreenTall);

		const Scale scale(
			iScreenWide / 640.0f,
			iScreenTall / 480.0f);

		return scale;
	}
}