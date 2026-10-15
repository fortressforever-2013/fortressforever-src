#ifndef FF_COLORPICKER_H
#define FF_COLORPICKER_H

#pragma push_macro("min")
#pragma push_macro("max")
#undef min
#undef max
#include <string>
#pragma pop_macro("max")
#pragma pop_macro("min")

#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>
#include "ff_inputslider.h"
#include "ff_colorpickerdialog.h"
#include "ff_quantityhelper.h"
#include <vgui_controls/Label.h>

namespace vgui
{
	class FFColorPicker : public Panel
	{
		DECLARE_CLASS_SIMPLE(FFColorPicker, Panel);
	public:
		enum class Mode
		{
			EMBEDDED,
			POPUP
		};

		FFColorPicker(
			Panel* parent,
			const char* name,
			Panel* pActionSignalTarget = NULL,
			bool bColorModeIntensity = false,
			Mode mode = Mode::EMBEDDED,
			bool bPaletteSelection = true);

		void SetColorPickerDialog(FFColorPickerDialog* newDialog);
		void SetColorModeSelectionEnabled(bool bEnabled);
		void SetEnabled(bool bState) override;

		int GetRedComponentValue() const;
		int GetGreenComponentValue() const;
		int GetBlueComponentValue() const;
		int GetAlphaComponentValue() const;
		int GetColorMode() const;

		void SetRedComponentValue(int iRed);
		void SetGreenComponentValue(int iGreen);
		void SetBlueComponentValue(int iBlue);
		void SetAlphaComponentValue(int iAlpha);
		void SetColorMode(int iColorMode);
		void SetPalettePresets(KeyValues* kvPresets);
		void ConfigureIntensityEditor();
		void LoadIntensity(KeyValues* kvComponent);
		void SaveIntensity(KeyValues* kvComponent) const;

		void SetValue(
			int iRed,
			int iGreen,
			int iBlue,
			int iAlpha);
		void GetValue(
			int& iRed,
			int& iGreen,
			int& iBlue,
			int& iAlpha) const;

	private:
		class ColorSwatchButton;
		class CroppedImagePanel;
		class IntensityBarPreview;

		FFQuantityHelper::IntensityPalette m_intensity;
		ComboBox* m_pIntensityPalette = NULL;
		Label* m_pIntensityPaletteLabel = NULL;
		std::string m_paletteName = "Default";
		FFColorPicker* m_pIntensityPickers[FFQuantityHelper::IntensityPalette::LEVEL_COUNT]{};
		Label* m_pIntensityLabels[FFQuantityHelper::IntensityPalette::LEVEL_COUNT]{};
		void UpdateIntensityControls();
		MESSAGE_FUNC_PARAMS(OnIntensityColorChanged, "ColorChanged", data);

		ImagePanel* m_pColorBackground = NULL;
		Button* m_pDialogButton = NULL;
		FFColorPickerDialog* m_pDialog = NULL;

		CFFInputSlider* m_pRed = NULL;
		CFFInputSlider* m_pGreen = NULL;
		CFFInputSlider* m_pBlue = NULL;
		CFFInputSlider* m_pAlpha = NULL;
		ComboBox* m_pColorMode = NULL;
		Panel* m_pSteppedPreview = nullptr;
		IntensityBarPreview* m_pFadedBarPreview = nullptr;
		IntensityBarPreview* m_pSteppedBarPreview = nullptr;

		int m_iTeamColorPreview = 0;
		int m_iRed = 255;
		int m_iGreen = 255;
		int m_iBlue = 255;
		int m_iAlpha = 255;
		int m_iColorMode = 0;
		bool m_bColorModeSelectionEnabled = true;

		virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

		void UpdateColor();
		void RecalculateColorPreview();
		void SetRGBComponentsEnabled(bool bState);

		void OnTick(void);
		void OnThink() override;

		MESSAGE_FUNC_PARAMS(OnDialogButtonClick, "Command", data);
		MESSAGE_FUNC_PARAMS(OnRedComponentChanged, "RedComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnGreenComponentChanged, "GreenComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnBlueComponentChanged, "BlueComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnAlphaComponentChanged, "AlphaComponentChanged", data);
		MESSAGE_FUNC_PARAMS(OnColorModeChanged, "ColorModeChanged", data);
		MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);
	};
};
#endif