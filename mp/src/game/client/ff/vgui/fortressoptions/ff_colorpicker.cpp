#include "cbase.h"
#include "ff_utils.h"
#include "ff_colorpicker.h"
#include "ff_quantityhelper.h"
#include "ff_shareddefs.h"
#include "KeyValues.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>

using namespace FFQuantityHelper;

namespace vgui
{
	class FFColorPicker::ColorSwatchButton : public Button
	{
		DECLARE_CLASS_SIMPLE(ColorSwatchButton, Button);

	public:
		ColorSwatchButton(Panel* parent, const char* name, FFColorPicker* pPicker)
			: BaseClass(
				parent,
				name,
				"",
				pPicker,
				"OpenColorPicker"), m_pPicker(pPicker)
		{
		}

	protected:
		void PaintBackground() override
		{
			surface()->DrawSetColor(m_pPicker->GetBgColor());
			surface()->DrawFilledRect(
				0,
				0,
				GetWide(),
				GetTall());
		}

	private:
		FFColorPicker* m_pPicker;
	};


	FFColorPicker::FFColorPicker(
		Panel* parent,
		const char* panelName,
		Panel* pActionSignalTarget,
		bool bColorModeIntensity,
		Mode mode)
		: BaseClass(parent, panelName)
	{
		if (mode == Mode::POPUP)
		{
			// Popup pickers display their colour through the sibling button only.
			SetPaintBackgroundEnabled(false);
			SetPaintBorderEnabled(false);
			SetSize(0, 0);

			char dialogButtonName[128];
			Q_snprintf(
				dialogButtonName,
				sizeof(dialogButtonName),
				"%sDialogButton",
				panelName);
			m_pDialogButton = new ColorSwatchButton(parent, dialogButtonName, this);

			m_pDialog = new FFColorPickerDialog(this, "FFColorPicker", bColorModeIntensity);
			m_pDialog->AddActionSignalTarget(this);

			if (pActionSignalTarget)
				AddActionSignalTarget(pActionSignalTarget);

			return;
		}

		m_pDialogButton = NULL;

		char inputSliderName[128];
		char inputSliderInputName[128];

		Q_strncpy(inputSliderName, panelName, 127);
		Q_strncpy(inputSliderInputName, panelName, 127);
		Q_strncat(
			inputSliderName,
			"Red",
			127,
			COPY_ALL_CHARACTERS);
		Q_strncat(
			inputSliderInputName,
			"RedInput",
			127,
			COPY_ALL_CHARACTERS);
		m_pRed = new CFFInputSlider(
			parent,
			inputSliderName,
			inputSliderInputName,
			this);
		m_pRed->SetRange(0, 255);
		m_pRed->SetValue(255);

		Q_strncpy(inputSliderName, panelName, 127);
		Q_strncpy(inputSliderInputName, panelName, 127);
		Q_strncat(
			inputSliderName,
			"Green",
			127,
			COPY_ALL_CHARACTERS);
		Q_strncat(
			inputSliderInputName,
			"GreenInput",
			127,
			COPY_ALL_CHARACTERS);
		m_pGreen = new CFFInputSlider(
			parent,
			inputSliderName,
			inputSliderInputName,
			this);
		m_pGreen->SetRange(0, 255);
		m_pGreen->SetValue(255);

		Q_strncpy(inputSliderName, panelName, 127);
		Q_strncpy(inputSliderInputName, panelName, 127);
		Q_strncat(
			inputSliderName,
			"Blue",
			127,
			COPY_ALL_CHARACTERS);
		Q_strncat(
			inputSliderInputName,
			"BlueInput",
			127,
			COPY_ALL_CHARACTERS);
		m_pBlue = new CFFInputSlider(
			parent,
			inputSliderName,
			inputSliderInputName,
			this);
		m_pBlue->SetRange(0, 255);
		m_pBlue->SetValue(255);

		Q_strncpy(inputSliderName, panelName, 127);
		Q_strncpy(inputSliderInputName, panelName, 127);
		Q_strncat(
			inputSliderName,
			"Alpha",
			127,
			COPY_ALL_CHARACTERS);
		Q_strncat(
			inputSliderInputName,
			"AlphaInput",
			127,
			COPY_ALL_CHARACTERS);
		m_pAlpha = new CFFInputSlider(
			parent,
			inputSliderName,
			inputSliderInputName,
			this);
		m_pAlpha->SetRange(0, 255);
		m_pAlpha->SetValue(255);

		char colorModeComboName[128];
		Q_strncpy(colorModeComboName, panelName, 127);
		Q_strncat(
			colorModeComboName,
			"ColorMode",
			127,
			COPY_ALL_CHARACTERS);
		m_pColorMode = new ComboBox(
			parent,
			colorModeComboName,
			4,
			false);
		m_pColorMode->RemoveActionSignalTarget(parent);
		if (bColorModeIntensity)
		{
			KeyValues* kv = new KeyValues("Custom");
			kv->SetInt("Value", COLOR_MODE_CUSTOM);
			m_pColorMode->AddItem("#GameUI_Custom", kv);
			kv->deleteThis();
			kv = new KeyValues("Stepped");
			kv->SetInt("Value", COLOR_MODE_STEPPED);
			m_pColorMode->AddItem("#GameUI_Stepped", kv);
			kv->deleteThis();
			kv = new KeyValues("Faded");
			kv->SetInt("Value", COLOR_MODE_FADED);
			m_pColorMode->AddItem("#GameUI_Faded", kv);
			kv->deleteThis();
			kv = new KeyValues("TeamColored");
			kv->SetInt("Value", COLOR_MODE_TEAM);
			m_pColorMode->AddItem("#GameUI_TeamColored", kv);
			kv->deleteThis();
			kv = new KeyValues("TeamColoredBright");
			kv->SetInt("Value", COLOR_MODE_TEAM_BRIGHT);
			m_pColorMode->AddItem("#GameUI_TeamColoredBright", kv);
			kv->deleteThis();
		}
		else
		{
			KeyValues* kv = new KeyValues("Custom");
			kv->SetInt("Value", COLOR_MODE_CUSTOM);
			m_pColorMode->AddItem("#GameUI_Custom", kv);
			kv->deleteThis();
			kv = new KeyValues("TeamColored");
			kv->SetInt("Value", COLOR_MODE_TEAM);
			m_pColorMode->AddItem("#GameUI_TeamColored", kv);
			kv->deleteThis();
			kv = new KeyValues("TeamColoredBright");
			kv->SetInt("Value", COLOR_MODE_TEAM_BRIGHT);
			m_pColorMode->AddItem("#GameUI_TeamColoredBright", kv);
			kv->deleteThis();
		}

		m_pColorMode->ActivateItemByRow(0);
		m_pColorMode->AddActionSignalTarget(this);
		m_iColorMode = 0;

		char imagePanelName[128];
		Q_strncpy(imagePanelName, panelName, 127);
		Q_strncat(
			imagePanelName,
			"Background",
			127,
			COPY_ALL_CHARACTERS);
		m_pColorBackground = new ImagePanel(parent, imagePanelName);
		m_pColorBackground->SetImage("crosshairbg");
		m_pColorBackground->SetZPos(-1);

		m_iTeamColorPreview = FF_TEAM_BLUE;

		if (pActionSignalTarget)
		{
			this->AddActionSignalTarget(pActionSignalTarget);
		}
	}

	void FFColorPicker::SetColorPickerDialog(FFColorPickerDialog* newDialog)
	{
		Assert(m_pDialogButton && newDialog);
		if (!m_pDialogButton || !newDialog || newDialog == m_pDialog)
			return;

		m_pDialog->RemoveActionSignalTarget(this);
		m_pDialog->MarkForDeletion();
		m_pDialog = newDialog;
		m_pDialog->SetParent(this);
		m_pDialog->AddActionSignalTarget(this);
	}

	void FFColorPicker::SetColorModeSelectionEnabled(bool bEnabled)
	{
		m_bColorModeSelectionEnabled = bEnabled;

		if (m_pDialog)
			m_pDialog->SetColorModeSelectionEnabled(bEnabled);
		else
			m_pColorMode->SetEnabled(bEnabled && IsEnabled());
	}

	void FFColorPicker::SetEnabled(bool bState)
	{
		BaseClass::SetEnabled(bState);

		if (m_pDialogButton)
		{
			m_pDialogButton->SetEnabled(bState);
			if (!bState && m_pDialog)
				m_pDialog->SetVisible(false);

			return;
		}

		if (!m_pColorMode)
			return;

		m_pColorMode->SetEnabled(bState && m_bColorModeSelectionEnabled);
		SetRGBComponentsEnabled(bState && m_iColorMode == COLOR_MODE_CUSTOM);
		m_pAlpha->SetEnabled(bState);
	}

	void FFColorPicker::OnDialogButtonClick(KeyValues* data)
	{
		if (!IsEnabled())
			return;

		m_pDialog->SetValue(
			m_iRed,
			m_iGreen,
			m_iBlue,
			m_iAlpha);
		m_pDialog->SetColorMode(m_iColorMode);
		m_pDialog->SetVisible(true);
	}

	void FFColorPicker::ApplySchemeSettings(
		IScheme* pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);
		RecalculateColorPreview();
	}

	void FFColorPicker::UpdateColor()
	{
		RecalculateColorPreview();

		KeyValues* msg = new KeyValues("ColorChanged");
		msg->SetPtr("panel", this);
		PostActionSignal(msg);
	}

	int FFColorPicker::GetRedComponentValue()
	{
		return m_iRed;
	}

	int FFColorPicker::GetGreenComponentValue()
	{
		return m_iGreen;
	}

	int FFColorPicker::GetBlueComponentValue()
	{
		return m_iBlue;
	}

	int FFColorPicker::GetAlphaComponentValue()
	{
		return m_iAlpha;
	}

	int FFColorPicker::GetColorMode()
	{
		return m_iColorMode;
	}

	void FFColorPicker::SetColorMode(
		int iColorMode)
	{
		m_iColorMode = iColorMode;
		if (!m_pDialogButton)
		{
			m_pColorMode->RemoveActionSignalTarget(this);
			for (int i = 0; i < m_pColorMode->GetItemCount(); ++i)
			{
				KeyValues* kvColorMode = m_pColorMode->GetItemUserData(i);
				if (kvColorMode->GetInt("Value") == iColorMode)
				{
					m_pColorMode->ActivateItemByRow(i);
					break;
				}
			}

			m_pColorMode->AddActionSignalTarget(this);
		}
	}

	void FFColorPicker::SetRedComponentValue(int iRed)
	{
		if (!m_pDialogButton)
		{
			m_pRed->RemoveActionSignalTarget(this);
			m_pRed->SetValue(iRed);
			m_pRed->AddActionSignalTarget(this);
		}

		m_iRed = iRed;
		RecalculateColorPreview();
	}

	void FFColorPicker::SetGreenComponentValue(int iGreen)
	{
		if (!m_pDialogButton)
		{
			m_pGreen->RemoveActionSignalTarget(this);
			m_pGreen->SetValue(iGreen);
			m_pGreen->AddActionSignalTarget(this);
		}

		m_iGreen = iGreen;
		RecalculateColorPreview();
	}

	void FFColorPicker::SetBlueComponentValue(int iBlue)
	{
		if (!m_pDialogButton)
		{
			m_pBlue->RemoveActionSignalTarget(this);
			m_pBlue->SetValue(iBlue);
			m_pBlue->AddActionSignalTarget(this);
		}

		m_iBlue = iBlue;
		RecalculateColorPreview();
	}

	void FFColorPicker::SetAlphaComponentValue(int iAlpha)
	{
		if (!m_pDialogButton)
		{
			m_pAlpha->RemoveActionSignalTarget(this);
			m_pAlpha->SetValue(iAlpha);
			m_pAlpha->AddActionSignalTarget(this);
		}

		m_iAlpha = iAlpha;
		RecalculateColorPreview();
	}

	void FFColorPicker::SetValue(
		int iRed,
		int iGreen,
		int iBlue,
		int iAlpha)
	{
		m_iRed = iRed;
		m_iGreen = iGreen;
		m_iBlue = iBlue;
		m_iAlpha = iAlpha;
		if (!m_pDialogButton)
		{
			m_pRed->RemoveActionSignalTarget(this);
			m_pRed->SetValue(iRed);
			m_pRed->AddActionSignalTarget(this);
			m_pGreen->RemoveActionSignalTarget(this);
			m_pGreen->SetValue(iGreen);
			m_pGreen->AddActionSignalTarget(this);
			m_pBlue->RemoveActionSignalTarget(this);
			m_pBlue->SetValue(iBlue);
			m_pBlue->AddActionSignalTarget(this);
			m_pAlpha->RemoveActionSignalTarget(this);
			m_pAlpha->SetValue(iAlpha);
			m_pAlpha->AddActionSignalTarget(this);
		}

		SetBgColor(Color(
			m_iRed,
			m_iGreen,
			m_iBlue,
			m_iAlpha));
	}

	void FFColorPicker::GetValue(
		int& iRed,
		int& iGreen,
		int& iBlue,
		int& iAlpha)
	{
		iRed = m_iRed;
		iGreen = m_iGreen;
		iBlue = m_iBlue;
		iAlpha = m_iAlpha;
	}

	//dialog callbacks
	void FFColorPicker::OnRedComponentChanged(KeyValues* data)
	{
		m_iRed = data->GetInt("Value");
		UpdateColor();
	}

	void FFColorPicker::OnGreenComponentChanged(KeyValues* data)
	{
		m_iGreen = data->GetInt("Value");
		UpdateColor();
	}

	void FFColorPicker::OnBlueComponentChanged(KeyValues* data)
	{
		m_iBlue = data->GetInt("Value");
		UpdateColor();
	}

	void FFColorPicker::OnAlphaComponentChanged(KeyValues* data)
	{
		m_iAlpha = data->GetInt("Value");
		UpdateColor();
	}

	void FFColorPicker::OnColorModeChanged(KeyValues* data)
	{
		m_iColorMode = data->GetInt("Value");
		UpdateColor();
	}

	//no-dialog callbacks
	void FFColorPicker::OnUpdateSliders(KeyValues* data)
	{
		if (data->GetPtr("panel") == m_pRed)
		{
			m_iRed = m_pRed->GetValue();
			UpdateColor();
		}
		else if (data->GetPtr("panel") == m_pGreen)
		{
			m_iGreen = m_pGreen->GetValue();
			UpdateColor();
		}
		else if (data->GetPtr("panel") == m_pBlue)
		{
			m_iBlue = m_pBlue->GetValue();
			UpdateColor();
		}
		else if (data->GetPtr("panel") == m_pAlpha)
		{
			m_iAlpha = m_pAlpha->GetValue();
			UpdateColor();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxes changing their selection
	//-----------------------------------------------------------------------------
	void FFColorPicker::OnUpdateCombos(KeyValues* data)
	{
		if (data->GetPtr("panel") == m_pColorMode)
		{
			KeyValues* kvSelectedColorMode = m_pColorMode->GetActiveItemUserData();

			m_iColorMode = kvSelectedColorMode->GetInt("Value");

			KeyValues* msg = new KeyValues("ColorModeChanged");
			msg->SetInt("Value", m_iColorMode);
			PostActionSignal(msg);

			if (m_iColorMode == COLOR_MODE_CUSTOM)
			{
				SetRGBComponentsEnabled(true);
				ivgui()->RemoveTickSignal(GetVPanel());
			}
			else if (m_iColorMode == COLOR_MODE_TEAM || m_iColorMode == COLOR_MODE_TEAM_BRIGHT)
			{
				SetRGBComponentsEnabled(false);
				ivgui()->AddTickSignal(GetVPanel(), 1250);
			}
			else
			{
				SetRGBComponentsEnabled(false);
				ivgui()->RemoveTickSignal(GetVPanel());
			}

			RecalculateColorPreview();
		}
	}

	void FFColorPicker::SetRGBComponentsEnabled(
		bool bState)
	{
		m_pRed->SetEnabled(bState && IsEnabled());
		m_pGreen->SetEnabled(bState && IsEnabled());
		m_pBlue->SetEnabled(bState && IsEnabled());
	}

	void FFColorPicker::OnTick()
	{
		if (m_iTeamColorPreview == FF_TEAM_GREEN)
		{
			m_iTeamColorPreview = FF_TEAM_BLUE;
		}
		else
		{
			++m_iTeamColorPreview;
		}

		RecalculateColorPreview();
	}

	void FFColorPicker::RecalculateColorPreview()
	{
		switch (m_iColorMode)
		{
		case COLOR_MODE_CUSTOM:
			SetBgColor(
				Color(
					m_iRed,
					m_iGreen,
					m_iBlue,
					m_iAlpha));
			break;
		case COLOR_MODE_TEAM_BRIGHT:
		case COLOR_MODE_TEAM:
			Color teamColor;
			switch (m_iTeamColorPreview)
			{
			case TEAM_SPECTATOR:
			default:
				teamColor = TEAM_COLOR_SPECTATOR;
				break;
			case FF_TEAM_BLUE:
				teamColor = TEAM_COLOR_BLUE;
				break;
			case FF_TEAM_RED:
				teamColor = TEAM_COLOR_RED;
				break;
			case FF_TEAM_YELLOW:
				teamColor = TEAM_COLOR_YELLOW;
				break;
			case FF_TEAM_GREEN:
				teamColor = TEAM_COLOR_GREEN;
				break;
			}

			if (m_iColorMode == COLOR_MODE_TEAM_BRIGHT)
				SetColorByTeam(m_iTeamColorPreview, teamColor);

			SetBgColor(
				Color(
					teamColor.r(),
					teamColor.g(),
					teamColor.b(),
					m_iAlpha));
			break;
		}
	}
};
