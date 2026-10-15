#ifndef FF_CUSTOMHUDOPTIONS_ASSIGNPRESETS_H
#define FF_CUSTOMHUDOPTIONS_ASSIGNPRESETS_H

#include "ff_optionspage.h"
#include "ff_customhudoptions_panelstylepresets.h"
#include "ff_customhudoptions_itemstylepresets.h"
#include "ff_customhudoptions_positionpresets.h"

#include "KeyValues.h"
#include "filesystem.h"
#include "ff_quantitypanel.h"
#include "ff_colorpicker.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ScrollBar.h>
#include <vgui_controls/PropertySheet.h>

#define	PRESETCOMOBO_ROW_HEIGHT 30
#define	PRESETCOMOBO_X 180
#define	PRESETCOMOBO_Y 80

using namespace vgui;

class CFFCustomHudAssignPresets : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFCustomHudAssignPresets, CFFOptionsPage);

public:
	CFFCustomHudAssignPresets(
		Panel* parent,
		char const* panelName,
		CFFCustomHudItemStylePresets* stylePresetsClass,
		CFFCustomHudPanelStylePresets* arrangementPresetsClass,
		CFFCustomHudPositionPresets* positionPresetsClass);
	~CFFCustomHudAssignPresets();

	virtual void Load();
	virtual void Reset();
	virtual void Apply();

	virtual void OnMouseWheeled(int delta);

	void UpdateNumericOption(void* pHudPanel, const char* pszName, int iValue);
	bool IsReady();
	void ApplyAssignmentToControls(KeyValues* kvAssignment);
	void UpdateSecondaryComboFromParentKey(KeyValues* kvPrimary);

	void ItemStylePresetPreviewUpdated(const char* pszPresetName, KeyValues* kvPresetPreview);
	void ItemStylePresetUpdated(const char* pszPresetName);
	void ItemStylePresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName);
	void ItemStylePresetDeleted(const char* pszDeletedPresetName, bool bPreviewOnly = false);
	void ItemStylePresetAdded(const char* pszPresetName, KeyValues* kvPreset);

	void PanelStylePresetPreviewUpdated(const char* pszPresetName, KeyValues* kvPresetPreview);
	void PanelStylePresetUpdated(const char* pszPresetName);
	void PanelStylePresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName);
	void PanelStylePresetDeleted(const char* pszDeletedPresetName, bool bPreviewOnly = false);
	void PanelStylePresetAdded(const char* pszPresetName, KeyValues* kvPreset);

	void PositionPresetPreviewUpdated(const char* pszPresetName, KeyValues* kvPresetPreview);
	void PositionPresetUpdated(const char* pszPresetName);
	void PositionPresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName);
	void PositionPresetDeleted(const char* pszDeletedPresetName, bool bPreviewOnly = false);
	void PositionPresetAdded(const char* pszPresetName, KeyValues* kvPreset);

	void OnItemStylePresetsClassLoaded();
	void OnPanelStylePresetsClassLoaded();
	void OnPositionPresetsClassLoaded();

	KeyValues* GetPresetAssignments();
	void RegisterAssignmentChange(const char* keyName, ComboBox* comboBox);

	KeyValues* GetStyleDataForAssignment(KeyValues* kvAssignment);
	void SendStyleDataToAssignment(KeyValues* kvAssignment);
	void SendPresetPreviewDataToAssignment(KeyValues* kvAssignment, KeyValues* kvPresetData);
	void CreatePresetFromPanelDefault(KeyValues* kvPresetData, void* pHudPanel);
	void AssignPresetToPanel(void* pHudPanel, const char* pszPresetType, const char* pszPresetName);

private:
	void AddAssignmentOption(ComboBox* pCombo, const char* pszText, KeyValues* kvAssignment);
	void UpdateAssignmentForDeletedPreset(
		KeyValues* kvAssignment,
		const char* pszPresetType,
		const char* pszDefault,
		bool bPreviewOnly);

	void SetColorOptionValues(KeyValues* kvAssignment, FFColorPicker* pPicker);

	//-----------------------------------------------------------------------------
	// Purpose: Catch quantity panels requesting to be added for
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnAddQuantityPanel, "AddQuantityPanel", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch the checkboxes changing their state
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCheckButton, "CheckButtonChecked", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch the "Copy Default" buttons
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch changes to custom color and alert options
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnColorOptionChanged, "ColorChanged", data);
	MESSAGE_FUNC_PARAMS(OnAlertSettingsChanged, "AlertSettingsChanged", data);

	//-----------------------------------------------------------------------------
	// Purpose: Scroll the panel-specific options
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnSpecificOptionsScrolled, "ScrollBarSliderMoved", data);

	MESSAGE_FUNC(OnSpecificOptionsPageChanged, "PageChanged");

	void CreatePanelSpecificControls(KeyValues* kvAssignment);

	PropertySheet* m_pSpecificOptionsTabs;
	Panel* m_pGeneralOptionsPage;
	Panel* m_pItemOptionsPage;
	KeyValues* m_kvSpecificAssignment = nullptr;
	int m_iSpecificTabScroll[2] = { 0, 0 };

	void SetPanelSpecificValues(KeyValues* kvAssignment, KeyValues* kvValues);
	void RestoreAssignment(KeyValues* kvAssignment, KeyValues* kvSavedAssignment);
	void RestoreAssignments();

	Panel* m_pPanelSpecificOptions;
	ScrollBar* m_pSpecificOptionsScroll;
	int m_iSpecificOptionsScroll = 0;

	ComboBox* m_pPrimary, * m_pSecondary;
	ComboBox* m_pItemStylePresets, * m_pPanelStylePresets, * m_pPositionPresets;

	CheckButton* m_pPreviewMode;

	Button* m_pCopyDefaultItemStyle, * m_pCopyDefaultPanelStyle, * m_pCopyDefaultPosition;
	Button* m_pResetCustomOptions;

	KeyValues* m_kvLoadedAssignments;
	KeyValues* m_kvRequiredUpdates;

	CFFCustomHudPanelStylePresets* m_pPanelStylePresetsClass;
	CFFCustomHudItemStylePresets* m_pItemStylePresetsClass;
	CFFCustomHudPositionPresets* m_pPositionPresetsClass;

	bool m_bCopyDefaultPosition;
	bool m_bCopyDefaultPanelStyle;
	bool m_bCopyDefaultItemStyle;

	bool m_bLoaded;
	int iYCoords;

	void RegisterAssignmentChange(KeyValues* kvSelf, KeyValues* kvParent = NULL);
};

extern CFFCustomHudAssignPresets* g_AP;

#endif
