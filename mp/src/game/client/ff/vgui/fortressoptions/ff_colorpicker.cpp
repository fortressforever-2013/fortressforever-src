#include "cbase.h"
#include "ff_utils.h"
#include "ff_colorpicker.h"
#include "ff_quantityhelper.h"
#include "ff_shareddefs.h"
#include "KeyValues.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/IImage.h>
#include <vgui/IScheme.h>

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

	class FFColorPicker::CroppedImagePanel : public ImagePanel
	{
		DECLARE_CLASS_SIMPLE(CroppedImagePanel, ImagePanel);

	public:
		CroppedImagePanel(
			Panel* parent,
			const char* name)
			: BaseClass(parent, name)
		{
			SetMouseInputEnabled(false);
			SetKeyBoardInputEnabled(false);
		}

		void ApplySettings(KeyValues* kvSettings) override
		{
			BaseClass::ApplySettings(kvSettings);

			m_iCropOffsetX = kvSettings->GetInt("cropOffsetX", 0);
			m_iCropOffsetY = kvSettings->GetInt("cropOffsetY", 0);
		}

	protected:
		void GetSettings(KeyValues* kvSettings) override
		{
			BaseClass::GetSettings(kvSettings);

			kvSettings->SetInt("cropOffsetX", m_iCropOffsetX);
			kvSettings->SetInt("cropOffsetY", m_iCropOffsetY);
		}

		//-----------------------------------------------------------------------------
		// Purpose: Fill the background without stretching, cropping at the resource offset
		//-----------------------------------------------------------------------------
		void PaintBackground() override
		{
			IImage* pImage = GetImage();
			if (!pImage)
				return;

			int iImageWide, iImageTall;
			pImage->GetContentSize(iImageWide, iImageTall);
			if (iImageWide <= 0 || iImageTall <= 0)
				return;

			const float flScale = Max(
				GetWide() / float(iImageWide),
				GetTall() / float(iImageTall));
			const int iWide = static_cast<int>(ceilf(iImageWide * flScale));
			const int iTall = static_cast<int>(ceilf(iImageTall * flScale));

			const int iOffsetX = IsProportional()
				? scheme()->GetProportionalScaledValueEx(GetScheme(), m_iCropOffsetX)
				: m_iCropOffsetX;
			const int iOffsetY = IsProportional()
				? scheme()->GetProportionalScaledValueEx(GetScheme(), m_iCropOffsetY)
				: m_iCropOffsetY;

			int iOriginalWide, iOriginalTall;
			pImage->GetSize(iOriginalWide, iOriginalTall);
			pImage->SetSize(iWide, iTall);
			pImage->SetPos(
				-clamp(iOffsetX, 0, Max(0, iWide - GetWide())),
				-clamp(iOffsetY, 0, Max(0, iTall - GetTall())));
			pImage->SetColor(GetDrawColor());
			pImage->Paint();

			// Other previews share this image, so restore its drawing dimensions.
			pImage->SetSize(iOriginalWide, iOriginalTall);
			pImage->SetPos(0, 0);
		}

	private:
		int m_iCropOffsetX = 0;
		int m_iCropOffsetY = 0;
	};

	class FFColorPicker::IntensityBarPreview : public Panel
	{
		DECLARE_CLASS_SIMPLE(IntensityBarPreview, Panel);

	public:
		IntensityBarPreview(
			Panel* parent,
			const char* name)
			: BaseClass(parent, name)
		{
			SetMouseInputEnabled(false);
			SetKeyBoardInputEnabled(false);
		}

		void SetValue(
			int iIntensity,
			const Color& color)
		{
			m_flFraction = clamp(iIntensity / 100.0f, 0.0f, 1.0f);
			SetBgColor(color);
		}

	protected:
		void PaintBackground() override
		{
			const int iHeight = static_cast<int>(GetTall() * m_flFraction);
			surface()->DrawSetColor(GetBgColor());
			surface()->DrawFilledRect(
				0,
				GetTall() - iHeight,
				GetWide(),
				GetTall());
		}

	private:
		float m_flFraction = 0.0f;
	};

	FFColorPicker::FFColorPicker(
		Panel* parent,
		const char* panelName,
		Panel* pActionSignalTarget,
		bool bColorModeIntensity,
		Mode mode,
		bool bPaletteSelection)
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

		if (bColorModeIntensity && bPaletteSelection)
		{
			m_pIntensityPalette = new ComboBox(
				parent,
				VarArgs("%sIntensityPalette", panelName),
				8,
				false);
			m_pIntensityPalette->RemoveActionSignalTarget(parent);
			m_pIntensityPalette->AddActionSignalTarget(this);
			m_pIntensityPaletteLabel = new Label(
				parent,
				VarArgs("%sIntensityPaletteLabel", panelName),
				"#HudPalette_Selection");
			m_pIntensityPalette->SetVisible(false);
			m_pIntensityPaletteLabel->SetVisible(false);
		}

		if (bColorModeIntensity && !bPaletteSelection)
		{
			const char* const labels[IntensityPalette::LEVEL_COUNT] = {
				"#HudIntensity_Min", "#HudIntensity_Low", "#HudIntensity_Mid", "#HudIntensity_Max", "#HudIntensity_OverMax"
			};

			for (int i = 0; i < IntensityPalette::LEVEL_COUNT; ++i)
			{
				char name[128];
				Q_snprintf(
					name,
					sizeof(name),
					"%sIntensity%s",
					panelName,
					IntensityPalette::GetLevelName(i));
				m_pIntensityPickers[i] = new FFColorPicker(
					parent,
					name,
					this,
					false,
					Mode::POPUP);
				m_pIntensityPickers[i]->SetColorModeSelectionEnabled(false);
				m_pIntensityPickers[i]->m_pDialog->SetAlphaVisible(false);
				m_pIntensityPickers[i]->m_pDialogButton->SetVisible(false);

				Q_snprintf(
					name,
					sizeof(name),
					"%sIntensity%sLabel",
					panelName,
					IntensityPalette::GetLevelName(i));
				m_pIntensityLabels[i] = new Label(
					parent,
					name,
					labels[i]);
				m_pIntensityLabels[i]->SetContentAlignment(Label::a_west);
				m_pIntensityLabels[i]->SetVisible(false);
			}
		}

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
		UpdateIntensityControls();
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

	int FFColorPicker::GetRedComponentValue() const
	{
		return m_iRed;
	}

	int FFColorPicker::GetGreenComponentValue() const
	{
		return m_iGreen;
	}

	int FFColorPicker::GetBlueComponentValue() const
	{
		return m_iBlue;
	}

	int FFColorPicker::GetAlphaComponentValue() const
	{
		return m_iAlpha;
	}

	int FFColorPicker::GetColorMode() const
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

		RecalculateColorPreview();
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
		int& iAlpha) const
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

			if (m_pSteppedPreview)
				RecalculateColorPreview();
			else
				UpdateColor();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxes changing their selection
	//-----------------------------------------------------------------------------
	void FFColorPicker::OnUpdateCombos(KeyValues* data)
	{
		if (data->GetPtr("panel") == m_pIntensityPalette && m_pIntensityPalette)
		{
			KeyValues* kvChoice = m_pIntensityPalette->GetActiveItemUserData();
			if (!kvChoice)
				return;

			KeyValues* kvComponent = new KeyValues("Component");
			kvComponent->SetInt("alpha", m_iAlpha);
			kvComponent->SetString("IntensityPalette", kvChoice->GetString("Name"));
			LoadIntensity(kvComponent);
			kvComponent->deleteThis();
			UpdateColor();
			return;
		}

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

	//-----------------------------------------------------------------------------
	// Purpose: Set up simultaneous Faded and Stepped previews for editing a palette definition
	//-----------------------------------------------------------------------------
	void FFColorPicker::ConfigureIntensityEditor()
	{
		m_pColorMode->SetVisible(false);
		m_pColorMode->SetEnabled(false);

		m_pSteppedPreview = new Panel(
			GetParent(),
			VarArgs("%sSteppedPreview", GetName()));
		m_pSteppedPreview->SetMouseInputEnabled(false);
		m_pSteppedPreview->SetKeyBoardInputEnabled(false);

		new CroppedImagePanel(
			GetParent(),
			VarArgs("%sFadedBarBackground", GetName()));
		new CroppedImagePanel(
			GetParent(),
			VarArgs("%sSteppedBarBackground", GetName()));

		m_pFadedBarPreview = new IntensityBarPreview(
			GetParent(),
			VarArgs("%sFadedBarPreview", GetName()));
		m_pSteppedBarPreview = new IntensityBarPreview(
			GetParent(),
			VarArgs("%sSteppedBarPreview", GetName()));

		SetColorMode(COLOR_MODE_FADED);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Refresh named palette choices without changing the component palette reference
	//-----------------------------------------------------------------------------
	void FFColorPicker::SetPalettePresets(KeyValues* kvPresets)
	{
		if (!m_pIntensityPalette)
			return;

		m_pIntensityPalette->RemoveActionSignalTarget(this);
		m_pIntensityPalette->RemoveAll();
		KeyValues* kvDefault = new KeyValues("Palette");
		kvDefault->SetString("Name", "Default");
		m_pIntensityPalette->AddItem("#GameUI_Default", kvDefault);
		kvDefault->deleteThis();

		for (KeyValues* kvPreset = kvPresets->GetFirstTrueSubKey(); kvPreset; kvPreset = kvPreset->GetNextTrueSubKey())
		{
			KeyValues* kvChoice = new KeyValues("Palette");
			kvChoice->SetString("Name", kvPreset->GetName());
			m_pIntensityPalette->AddItem(kvPreset->GetName(), kvChoice);
			kvChoice->deleteThis();
		}

		KeyValues* kvComponent = new KeyValues("Component");
		kvComponent->SetInt("alpha", m_iAlpha);
		kvComponent->SetString("IntensityPalette", m_paletteName.c_str());
		LoadIntensity(kvComponent);
		kvComponent->deleteThis();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Resolve a palette reference or definition and refresh the colour controls
	//-----------------------------------------------------------------------------
	void FFColorPicker::LoadIntensity(KeyValues* kvComponent)
	{
		m_intensity.Reset(m_iAlpha);
		m_intensity.Apply(kvComponent, NULL, m_iAlpha);

		if (m_pIntensityPalette)
		{
			m_paletteName = kvComponent->GetString("IntensityPalette", "Default");
			m_pIntensityPalette->RemoveActionSignalTarget(this);
			int selected = 0;
			for (int row = 0; row < m_pIntensityPalette->GetItemCount(); ++row)
			{
				const int item = m_pIntensityPalette->GetItemIDFromRow(row);
				if (Q_stricmp(m_pIntensityPalette->GetItemUserData(item)->GetString("Name"), m_paletteName.c_str()) == 0)
					selected = row;
			}

			if (m_pIntensityPalette->GetItemCount() > 0)
				m_pIntensityPalette->ActivateItemByRow(selected);

			m_pIntensityPalette->AddActionSignalTarget(this);
		}

		for (int i = 0; i < IntensityPalette::LEVEL_COUNT; ++i)
		{
			if (!m_pIntensityPickers[i])
				continue;

			const Color& color = m_intensity.colors[i];
			m_pIntensityPickers[i]->m_pDialog->SetVisible(false);
			m_pIntensityPickers[i]->SetValue(
				color.r(),
				color.g(),
				color.b(),
				255);
		}

		RecalculateColorPreview();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Save a component palette reference or the palette editor RGB definition
	//-----------------------------------------------------------------------------
	void FFColorPicker::SaveIntensity(KeyValues* kvComponent) const
	{
		if (m_pIntensityPalette)
		{
			kvComponent->SetString("IntensityPalette", m_paletteName.c_str());
			KeyValues* kvEmbedded = kvComponent->FindKey("Intensity");
			if (kvEmbedded)
			{
				kvComponent->RemoveSubKey(kvEmbedded);
				kvEmbedded->deleteThis();
			}

			return;
		}

		m_intensity.Save(kvComponent->FindKey("Intensity", true));
	}

	void FFColorPicker::OnIntensityColorChanged(KeyValues* data)
	{
		for (int i = 0; i < IntensityPalette::LEVEL_COUNT; ++i)
		{
			if (data->GetPtr("panel") != m_pIntensityPickers[i])
				continue;

			int iRed, iGreen, iBlue, iAlpha;
			m_pIntensityPickers[i]->GetValue(
				iRed,
				iGreen,
				iBlue,
				iAlpha);
			m_intensity.colors[i] = Color(iRed, iGreen, iBlue, 255);
			UpdateColor();
			return;
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Show palette controls in intensity modes while keeping component alpha available
	//-----------------------------------------------------------------------------
	void FFColorPicker::UpdateIntensityControls()
	{
		if (!m_pIntensityPickers[0] && !m_pIntensityPalette)
			return;

		const bool bIntensity = m_iColorMode == COLOR_MODE_STEPPED || m_iColorMode == COLOR_MODE_FADED;
		CFFInputSlider* sliders[] = { m_pRed, m_pGreen, m_pBlue };
		for (CFFInputSlider* slider : sliders)
		{
			slider->SetVisible(!bIntensity);
			Panel* label = GetParent()->FindChildByName(VarArgs("%sLabel", slider->GetName()));
			if (label)
				label->SetVisible(!bIntensity);
		}

		m_pAlpha->SetVisible(true);
		Panel* pAlphaLabel = GetParent()->FindChildByName(VarArgs("%sLabel", m_pAlpha->GetName()));
		if (pAlphaLabel)
			pAlphaLabel->SetVisible(true);

		if (m_pIntensityPalette)
		{
			Label* pRedLabel = dynamic_cast<Label*>(GetParent()->FindChildByName(VarArgs("%sLabel", m_pRed->GetName())));
			if (pRedLabel)
				m_pIntensityPaletteLabel->SetFont(pRedLabel->GetFont());

			m_pIntensityPalette->SetVisible(bIntensity);
			m_pIntensityPalette->SetEnabled(IsEnabled());
			m_pIntensityPaletteLabel->SetVisible(bIntensity);
			m_pIntensityPaletteLabel->SetEnabled(IsEnabled());
			return;
		}

		Label* pRedLabel = dynamic_cast<Label*>(GetParent()->FindChildByName(VarArgs("%sLabel", m_pRed->GetName())));

		for (int i = 0; i < IntensityPalette::LEVEL_COUNT; ++i)
		{
			FFColorPicker* picker = m_pIntensityPickers[i];
			picker->SetEnabled(IsEnabled() && bIntensity);
			picker->m_pDialogButton->SetVisible(bIntensity);
			m_pIntensityLabels[i]->SetVisible(bIntensity);
			m_pIntensityLabels[i]->SetEnabled(IsEnabled());
			if (pRedLabel && m_pIntensityLabels[i]->GetFont() != pRedLabel->GetFont())
				m_pIntensityLabels[i]->SetFont(pRedLabel->GetFont());
		}
	}

	void FFColorPicker::OnThink()
	{
		BaseClass::OnThink();

		if (IsVisible() && (m_iColorMode == COLOR_MODE_STEPPED || m_iColorMode == COLOR_MODE_FADED))
			RecalculateColorPreview();
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

	//-----------------------------------------------------------------------------
	// Purpose: Update the colour sample and palette bar previews without changing saved colour settings
	//-----------------------------------------------------------------------------
	void FFColorPicker::RecalculateColorPreview()
	{
		UpdateIntensityControls();

		switch (m_iColorMode)
		{
		case COLOR_MODE_STEPPED:
		case COLOR_MODE_FADED:
		{
			// Sweep up and down over eight seconds, without editing the saved colour.
			const float flPhase = fmodf(gpGlobals->realtime, 8.0f) / 4.0f;
			const int iIntensity = static_cast<int>((flPhase <= 1.0f ? flPhase : 2.0f - flPhase) * 120.0f);
			Color faded = m_intensity.Evaluate(iIntensity, false);
			Color stepped = m_intensity.Evaluate(iIntensity, true);
			faded.setA(m_iAlpha);
			stepped.setA(m_iAlpha);

			SetBgColor(m_iColorMode == COLOR_MODE_STEPPED ? stepped : faded);

			if (m_pSteppedPreview)
				m_pSteppedPreview->SetBgColor(stepped);

			if (m_pFadedBarPreview)
			{
				m_pFadedBarPreview->SetValue(
					iIntensity,
					faded);
				m_pSteppedBarPreview->SetValue(
					iIntensity,
					stepped);
			}
			break;
		}
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
