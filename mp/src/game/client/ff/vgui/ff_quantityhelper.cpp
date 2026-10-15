#include "cbase.h"
#include "ff_quantityhelper.h"
#include "ff_shareddefs.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>

namespace FFQuantityHelper
{
	static KeyValues* s_kvIntensityPresets = NULL;

	void IntensityPalette::SetPresets(KeyValues* kvPresets)
	{
		if (s_kvIntensityPresets)
			s_kvIntensityPresets->deleteThis();

		s_kvIntensityPresets = kvPresets->MakeCopy();
	}

	IntensityPalette::IntensityPalette()
	{
		Reset();
	}

	const char* IntensityPalette::GetLevelName(int iLevel)
	{
		static const char* const names[LEVEL_COUNT] = { "Min", "Low", "Mid", "Max", "OverMax" };
		Assert(iLevel >= 0 && iLevel < LEVEL_COUNT);
		return names[iLevel];
	}

	void IntensityPalette::Reset(int iAlpha)
	{
		colors[0] = INTENSITYSCALE_COLOR_RED;
		colors[1] = INTENSITYSCALE_COLOR_ORANGE;
		colors[2] = INTENSITYSCALE_COLOR_YELLOW;
		colors[3] = INTENSITYSCALE_COLOR_GREEN;
		colors[4] = INTENSITYSCALE_COLOR_DEFAULT;

		for (Color& color : colors)
			color.setA(clamp(iAlpha, 0, 255));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Resolve a palette reference or definition while retaining component-owned alpha and partial updates
	//-----------------------------------------------------------------------------
	void IntensityPalette::Apply(
		KeyValues* kvComponent,
		KeyValues* kvDefaults,
		int iAlpha)
	{
		// Read the component's palette reference, falling back to its default style.
		KeyValues* kvPalette = kvComponent ? kvComponent->FindKey("IntensityPalette") : NULL;
		if (!kvPalette && kvDefaults)
			kvPalette = kvDefaults->FindKey("IntensityPalette");

		// The palette editor supplies a palette definition; item components supply a palette reference.
		KeyValues* kvIntensity = kvComponent ? kvComponent->FindKey("Intensity") : NULL;
		KeyValues* kvDefaultIntensity = kvDefaults ? kvDefaults->FindKey("Intensity") : NULL;
		const int iDefaultAlpha = kvDefaults ? kvDefaults->GetInt("alpha", iAlpha) : iAlpha;
		const int iComponentAlpha = kvComponent ? kvComponent->GetInt("alpha", iDefaultAlpha) : iDefaultAlpha;

		// Opacity belongs to the component, including during alpha-only previews.
		for (Color& color : colors)
			color.setA(clamp(iComponentAlpha, 0, 255));

		// Without a palette reference or definition, retain the current RGB colours.
		if (!kvPalette && !kvIntensity && !kvDefaultIntensity)
			return;

		// Reset before loading a palette definition so omitted channels cannot retain previous colours.
		Reset(iComponentAlpha);

		if (kvPalette)
		{
			// Resolve the palette reference instead of using the palette definitions from these styles.
			kvIntensity = NULL;
			kvDefaultIntensity = NULL;
			const char* pszPalette = kvPalette->GetString();

			// Default or an unavailable palette reference uses the default palette loaded by Reset().
			if (s_kvIntensityPresets && Q_stricmp(pszPalette, "Default") != 0)
			{
				// Compare palette names literally: FindKey would treat a slash as a path.
				for (KeyValues* kvPreset = s_kvIntensityPresets->GetFirstTrueSubKey();
					kvPreset;
					kvPreset = kvPreset->GetNextTrueSubKey())
				{
					if (Q_stricmp(kvPreset->GetName(), pszPalette) != 0)
						continue;

					kvIntensity = kvPreset->FindKey("Intensity");
					break;
				}
			}
		}

		// Read RGB channels from the palette definition, falling back to the default palette definition.
		// Channels absent from both definitions retain the default palette colour; alpha belongs to the component.
		const char* const channels[3] = { "Red", "Green", "Blue" };

		for (int i = 0; i < LEVEL_COUNT; ++i)
		{
			KeyValues* kvColor = kvIntensity ? kvIntensity->FindKey(GetLevelName(i)) : NULL;
			KeyValues* kvDefaultColor = kvDefaultIntensity ? kvDefaultIntensity->FindKey(GetLevelName(i)) : NULL;

			for (int channel = 0; channel < 3; ++channel)
			{
				int value = kvDefaultColor ? kvDefaultColor->GetInt(channels[channel], colors[i][channel]) : colors[i][channel];
				if (kvColor)
					value = kvColor->GetInt(channels[channel], value);

				colors[i][channel] = clamp(value, 0, 255);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Write the five RGB colour definitions without storing component opacity
	//-----------------------------------------------------------------------------
	void IntensityPalette::Save(KeyValues* kvIntensity) const
	{
		kvIntensity->Clear();
		for (int i = 0; i < LEVEL_COUNT; ++i)
		{
			KeyValues* kvColor = kvIntensity->FindKey(GetLevelName(i), true);
			kvColor->SetInt("Red", colors[i].r());
			kvColor->SetInt("Green", colors[i].g());
			kvColor->SetInt("Blue", colors[i].b());
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Choose a stepped colour or interpolate between thresholds, reaching Over Max beyond the normal maximum
	//-----------------------------------------------------------------------------
	Color IntensityPalette::Evaluate(
		int iValue,
		bool bStepped,
		int iRed,
		int iOrange,
		int iYellow,
		int iGreen,
		bool bInvert) const
	{
		// Give the final faded colour a finite endpoint: 20% beyond normal maximum.
		const int iOverMaxRange = Max(1, static_cast<int>(fabsf(static_cast<float>(iGreen)) * 0.2f));
		const int iOverMax = iGreen + (bInvert ? -iOverMaxRange : iOverMaxRange);
		const int thresholds[LEVEL_COUNT] = { iRed, iOrange, iYellow, iGreen, iOverMax };
		for (int i = 0; i < LEVEL_COUNT; ++i)
		{
			if (bInvert ? iValue < thresholds[i] : iValue > thresholds[i])
				continue;

			if (i == 0 || bStepped || thresholds[i] == thresholds[i - 1])
				return colors[i];

			const float fraction = clamp(
				static_cast<float>(iValue - thresholds[i - 1]) / (thresholds[i] - thresholds[i - 1]),
				0.0f,
				1.0f);
			Color color = colors[i - 1];
			for (int channel = 0; channel < 3; ++channel)
				color[channel] = static_cast<unsigned char>(
					colors[i - 1][channel] + fraction * (colors[i][channel] - colors[i - 1][channel]));

			return color;
		}

		return colors[4];
	}

	vgui::HFont GetFont(
		const vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier,
		int iSizeCount)
	{
		if (!hfFamily || iSizeCount <= 0)
			return 0;

		iSize = clamp(iSize, 0, iSizeCount - 1);

		const int iModifier
			= bUseModifier ? 1 : 0;

		const int iFontIndex
			= iSize * 3 + iModifier;

		return hfFamily[iFontIndex];
	}

	int GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		int iDefaultValue)
	{
		return kvStyleData
			? kvStyleData->GetInt(keyName, iDefaultValue)
			: iDefaultValue;
	}

	int GetInt(
		const char* keyName,
		KeyValues* kvStyleData,
		KeyValues* kvDefaultStyleData,
		int iDefaultValue)
	{
		iDefaultValue = GetInt(keyName, kvDefaultStyleData, iDefaultValue);
		return GetInt(keyName, kvStyleData, iDefaultValue);
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