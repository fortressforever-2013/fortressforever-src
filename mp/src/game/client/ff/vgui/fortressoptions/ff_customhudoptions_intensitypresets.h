#ifndef FF_CUSTOMHUDOPTIONS_INTENSITYPRESETS_H
#define FF_CUSTOMHUDOPTIONS_INTENSITYPRESETS_H

#include "ff_optionspresetpage.h"

#include "ff_colorpicker.h"

namespace vgui
{
	class CFFCustomHudItemStylePresets;

	class CFFCustomHudIntensityPresets : public CFFOptionsPresetPage
	{
		DECLARE_CLASS_SIMPLE(CFFCustomHudIntensityPresets, CFFOptionsPresetPage);

	public:
		CFFCustomHudIntensityPresets(
			Panel* parent,
			CFFCustomHudItemStylePresets* pItemStylePresetsClass);

		void ActivatePresetPage() override;
		void SetControlsEnabled(bool bEnabled) override;

		void UpdatePresetFromControls(KeyValues* kvPreset) override;
		void ApplyPresetToControls(KeyValues* kvPreset) override;
		void RegisterSelfForPresetAssignment() override;
		KeyValues* RemoveNonEssentialValues(KeyValues* kvPreset) override;

		void Apply() override;

		void SendUpdatedPresetPreviewToPresetAssignment(
			const char* pszPresetName,
			KeyValues* kvPresetPreview) override;
		void SendUpdatedPresetToPresetAssignment(const char* pszPresetName) override;
		void SendRenamedPresetToPresetAssignment(
			const char* pszOldPresetName,
			const char* pszNewPresetName) override;
		void SendDeletedPresetToPresetAssignment(
			const char* pszDeletedPresetName,
			bool bPreviewOnly = false) override;
		void SendNewPresetToPresetAssignment(
			const char* pszPresetName,
			KeyValues* kvPreset) override;

	private:
		void PublishPalettes(bool bPreviewOnly = true);
		void QueueRefresh();

		//-----------------------------------------------------------------------------
		// Purpose: Catch the colour picker changing the selected palette definition
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnColorChanged, "ColorChanged", data);

		//-----------------------------------------------------------------------------
		// Purpose: Refresh palette references after a preset change has finished
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC(OnRefreshPalettes, "RefreshPalettes");

		FFColorPicker* m_pColor = nullptr;
		CFFCustomHudItemStylePresets* m_pItemStylePresetsClass = nullptr;
		bool m_bApplying = false;
	};
}

#endif
