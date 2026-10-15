#include "cbase.h"
#include "ff_customhudoptions_intensitypresets.h"
#include "ff_customhudoptions_itemstylepresets.h"
#include "ff_customhudoptions_assignpresets.h"

#define INTENSITYPRESET_FILE "HudIntensityPalettes.vdf"

#include "tier0/memdbgon.h"

extern CFFCustomHudAssignPresets* g_AP;

namespace vgui
{
	CFFCustomHudIntensityPresets::CFFCustomHudIntensityPresets(
		Panel* parent,
		CFFCustomHudItemStylePresets* pItemStylePresetsClass)
		: BaseClass(
			parent,
			"CustomHudIntensityPresets",
			"Intensity",
			INTENSITYPRESET_FILE),
		m_pItemStylePresetsClass(pItemStylePresetsClass)
	{
		m_pColor = new FFColorPicker(
			this,
			"Palette",
			this,
			true,
			FFColorPicker::Mode::EMBEDDED,
			false);
		m_pColor->ConfigureIntensityEditor();
		m_pColor->SetAlphaComponentValue(100);
		LoadControlSettings("resource/ui/FFCustomHudOptionsIntensityPresets.res");
	}

	void CFFCustomHudIntensityPresets::ActivatePresetPage()
	{
		PostActionSignal(new KeyValues("ActivateIntensityPage"));
	}

	void CFFCustomHudIntensityPresets::SetControlsEnabled(bool bEnabled)
	{
		m_pColor->SetEnabled(bEnabled);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the selected palette definition from the colour controls
	//-----------------------------------------------------------------------------
	void CFFCustomHudIntensityPresets::UpdatePresetFromControls(KeyValues* kvPreset)
	{
		if (!kvPreset || m_bApplying)
			return;

		m_pColor->SaveIntensity(kvPreset);
		BaseClass::UpdatePresetFromControls(kvPreset);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the selected palette definition without recording a user edit
	//-----------------------------------------------------------------------------
	void CFFCustomHudIntensityPresets::ApplyPresetToControls(KeyValues* kvPreset)
	{
		if (!kvPreset)
			return;

		m_bApplying = true;
		m_pColor->LoadIntensity(kvPreset);
		m_bApplying = false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Make loaded palette definitions available to item styles and HUD panels
	//-----------------------------------------------------------------------------
	void CFFCustomHudIntensityPresets::RegisterSelfForPresetAssignment()
	{
		PublishPalettes(false);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Keep only the palette definition and supply defaults for missing RGB channels
	//-----------------------------------------------------------------------------
	KeyValues* CFFCustomHudIntensityPresets::RemoveNonEssentialValues(KeyValues* kvPreset)
	{
		KeyValues* kvCleaned = new KeyValues(kvPreset->GetName());
		FFQuantityHelper::IntensityPalette palette;
		palette.Apply(kvPreset, NULL, 255);
		palette.Save(kvCleaned->FindKey("Intensity", true));
		return kvCleaned;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Save palette definitions and refresh all HUD panels
	//-----------------------------------------------------------------------------
	void CFFCustomHudIntensityPresets::Apply()
	{
		BaseClass::Apply();
		PublishPalettes(false);
	}

	void CFFCustomHudIntensityPresets::SendUpdatedPresetPreviewToPresetAssignment(
		const char* pszPresetName,
		KeyValues* kvPresetPreview)
	{
		QueueRefresh();
	}

	void CFFCustomHudIntensityPresets::SendUpdatedPresetToPresetAssignment(const char* pszPresetName)
	{
		QueueRefresh();
	}

	void CFFCustomHudIntensityPresets::SendRenamedPresetToPresetAssignment(
		const char* pszOldPresetName,
		const char* pszNewPresetName)
	{
		m_pItemStylePresetsClass->RenamePaletteReferences(
			pszOldPresetName,
			pszNewPresetName);
		QueueRefresh();
	}

	void CFFCustomHudIntensityPresets::SendDeletedPresetToPresetAssignment(
		const char* pszDeletedPresetName,
		bool bPreviewOnly)
	{
		if (!bPreviewOnly)
			m_pItemStylePresetsClass->RenamePaletteReferences(
				pszDeletedPresetName,
				"Default");

		QueueRefresh();
	}

	void CFFCustomHudIntensityPresets::SendNewPresetToPresetAssignment(
		const char* pszPresetName,
		KeyValues* kvPreset)
	{
		kvPreset->deleteThis();
		QueueRefresh();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Refresh palette definitions, item-style choices and affected HUD panels
	//-----------------------------------------------------------------------------
	void CFFCustomHudIntensityPresets::PublishPalettes(bool bPreviewOnly)
	{
		KeyValues* kvPresets = GetPresetData();
		FFQuantityHelper::IntensityPalette::SetPresets(kvPresets);
		m_pItemStylePresetsClass->RefreshPaletteChoices(kvPresets);
		kvPresets->deleteThis();

		if (g_AP)
			g_AP->RefreshIntensityPalettes(bPreviewOnly);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Defer refresh until the preset collection has finished changing
	//-----------------------------------------------------------------------------
	void CFFCustomHudIntensityPresets::QueueRefresh()
	{
		// Preset creation/deletion finishes after its notification callback returns.
		PostMessage(this, new KeyValues("RefreshPalettes"));
	}

	void CFFCustomHudIntensityPresets::OnColorChanged(KeyValues* data)
	{
		if (m_bLoaded && !m_bApplying)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	void CFFCustomHudIntensityPresets::OnRefreshPalettes()
	{
		PublishPalettes();
	}
}
