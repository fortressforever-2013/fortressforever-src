#ifndef FF_COLORPICKER_H
#define FF_COLORPICKER_H

#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Panel.h>
#include "ff_inputslider.h"
#include "ff_colorpickerdialog.h"

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
			Mode mode = Mode::EMBEDDED);

		void SetColorPickerDialog(FFColorPickerDialog* newDialog);
		void SetColorModeSelectionEnabled(bool bEnabled);
		void SetEnabled(bool bState) override;

		int GetRedComponentValue();
		int GetGreenComponentValue();
		int GetBlueComponentValue();
		int GetAlphaComponentValue();
		int GetColorMode();

		void SetRedComponentValue(int iRed);
		void SetGreenComponentValue(int iGreen);
		void SetBlueComponentValue(int iBlue);
		void SetAlphaComponentValue(int iAlpha);
		void SetColorMode(int iColorMode);

		void SetValue(
			int iRed,
			int iGreen,
			int iBlue,
			int iAlpha);
		void GetValue(
			int& iRed,
			int& iGreen,
			int& iBlue,
			int& iAlpha);

	private:
		class ColorSwatchButton;

		ImagePanel* m_pColorBackground = NULL;
		Button* m_pDialogButton = NULL;
		FFColorPickerDialog* m_pDialog = NULL;

		CFFInputSlider* m_pRed = NULL;
		CFFInputSlider* m_pGreen = NULL;
		CFFInputSlider* m_pBlue = NULL;
		CFFInputSlider* m_pAlpha = NULL;
		ComboBox* m_pColorMode = NULL;

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