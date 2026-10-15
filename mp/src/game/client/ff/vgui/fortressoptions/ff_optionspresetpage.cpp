/********************************************************************
	created:	2010/09/02
	filename: 	cl_dll\ff\ff_optionspresetpage.cpp
	file path:	cl_dll\ff
	file base:	ff_optionspresetpage
	file ext:	cpp
	author:		Elmo

	purpose:	Preset editor panel (for cusomtisable hud stuff)
*********************************************************************/

#include "cbase.h"
#include "ff_customhudoptions.h"
#include "ff_optionspresetpage.h"
#include "ff_hudpresetnameinputdialog.h"

#include "filesystem.h"
extern IFileSystem** pFilesystem;

#include "ff_customhudoptions_assignpresets.h"
extern CFFCustomHudAssignPresets* g_AP;

namespace vgui {
	CFFOptionsPresetPage::CFFOptionsPresetPage(
		Panel* parent,
		const char* panelName,
		char const* pszPresetType,
		const char* pszSourceFile) : BaseClass(parent, panelName) {
		Q_strncpy(m_szSourceFile, pszSourceFile, 127);
		Q_strncpy(m_szPresetTypeName, pszPresetType, 127);

		//preset changes (new delete rename)
		m_kvChanges = new KeyValues("Changes");
		//updates to changes which need to be sent to assignment
		m_kvUpdates = new KeyValues("Updates");

		m_bLoaded = false;
		m_bIsLoading = false;

		m_bCopy = false;
		m_bRename = false;

		char comboBoxName[128];
		Q_strncpy(comboBoxName, m_szPresetTypeName, 127);
		Q_strncat(
			comboBoxName,
			"PresetSelectionCombo",
			127,
			COPY_ALL_CHARACTERS);
		m_pPresets = new ComboBox(
			this,
			comboBoxName,
			6,
			false);
		m_pNewPreset = new Button(
			this,
			"NewPresetButton",
			"",
			this,
			"NewPreset");
		m_pCopyPreset = new Button(
			this,
			"CopyPresetButton",
			"",
			this,
			"CopyPreset");
		m_pDeletePreset = new Button(
			this,
			"DeletePresetButton",
			"",
			this,
			"DeletePreset");
		m_pRenamePreset = new Button(
			this,
			"RenamePresetButton",
			"",
			this,
			"RenamePreset");

		m_kvPanelDefaultCopy = NULL;
	}

	CFFOptionsPresetPage::~CFFOptionsPresetPage()
	{
		m_kvChanges->deleteThis();
		m_kvChanges = NULL;
		m_kvUpdates->deleteThis();
		m_kvUpdates = NULL;
	}

	void CFFOptionsPresetPage::SetPresetControlsEnabled(bool bEnabled)
	{
		m_pDeletePreset->SetEnabled(bEnabled);
		m_pRenamePreset->SetEnabled(bEnabled);
		m_pCopyPreset->SetEnabled(bEnabled);
		m_pPresets->SetEnabled(bEnabled);
		SetControlsEnabled(bEnabled);
	}

	void CFFOptionsPresetPage::Load()
	{
		if (m_bLoaded || m_bIsLoading)
			return;

		m_bIsLoading = true;
		m_pPresets->RemoveActionSignalTarget(this);

		m_pPresets->RemoveAll();
		KeyValues* kvPresets = new KeyValues("Presets");
		kvPresets->LoadFromFile(*pFilesystem, m_szSourceFile);

		if (kvPresets->GetFirstTrueSubKey() == NULL)
			//if no presets were in the file (or file not found)
		{
			SetPresetControlsEnabled(false);
		}
		else
			//if presets exist
		{
			for (KeyValues* kvPreset = kvPresets->GetFirstTrueSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextTrueSubKey())
			{
				KeyValues* kvCleanedPreset = RemoveNonEssentialValues(kvPreset);
				kvCleanedPreset->SetInt(
					"version",
					kvPreset->GetInt("version", FFQuantityHelper::PRESET_DATA_VERSION));
				//use RemoveNonEssentialValues to remove/add keys using defaults
				m_pPresets->AddItem(kvPreset->GetName(), kvCleanedPreset);

				kvCleanedPreset->deleteThis();
				kvCleanedPreset = NULL;
			}

			SetPresetControlsEnabled(true);

			//load the first one to the controls by activating it
			m_pPresets->ActivateItemByRow(0);

			ApplyPresetToControls(m_pPresets->GetActiveItemUserData());
		}

		kvPresets->deleteThis();
		kvPresets = NULL;

		RegisterSelfForPresetAssignment();

		m_pPresets->AddActionSignalTarget(this);
		m_bLoaded = true;
		m_bIsLoading = false;
	}

	void CFFOptionsPresetPage::Reset()
	{
		m_kvChanges->Clear();
		m_kvUpdates->Clear();
		m_bCopy = false;
		m_bRename = false;

		if (m_kvPanelDefaultCopy)
		{
			m_kvPanelDefaultCopy->deleteThis();
			m_kvPanelDefaultCopy = NULL;
		}

		m_bLoaded = false;
		Load();
	}

	void CFFOptionsPresetPage::Apply()
	{
		for (int i = 0; i < m_kvChanges->GetInt("Count", 0); ++i)
		{
			char item[16];
			Q_snprintf(
				item,
				sizeof(item),
				"%i",
				i);
			KeyValues* kvCommand = m_kvChanges->FindKey(item);

			Assert(kvCommand);

			if (!kvCommand)
			{
				continue;
			}

			if (Q_stricmp(kvCommand->GetString("Delete"), "") != 0)
			{
				const char* pszDeletedName = kvCommand->GetString("Delete");
				KeyValues* kvReplacement = GetPresetDataByName(pszDeletedName);

				// A recreated preset owns this name now; keep its choices and assignments.
				if (kvReplacement)
				{
					kvReplacement->deleteThis();
					SendUpdatedPresetToPresetAssignment(pszDeletedName);
					continue;
				}

				SendDeletedPresetToPresetAssignment(pszDeletedName);
			}
		}

		m_kvChanges->deleteThis();
		m_kvChanges = NULL;
		m_kvChanges = new KeyValues("Changes");

		for (KeyValues* kvPreset = m_kvUpdates->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
		{
			SendUpdatedPresetToPresetAssignment(kvPreset->GetName());
		}

		m_kvUpdates->deleteThis();
		m_kvUpdates = NULL;
		m_kvUpdates = new KeyValues("Updates");

		KeyValues* kvPresets = GetPresetData();
		kvPresets->SaveToFile(*pFilesystem, m_szSourceFile);
		kvPresets->deleteThis();
	}

	bool CFFOptionsPresetPage::HasLoaded()
	{
		return m_bLoaded;
	}

	KeyValues* CFFOptionsPresetPage::FindPendingPresetUpdate(const char* pszPresetName)
	{
		// Preset names are literal strings; FindKey would interpret '/' as a path.
		for (KeyValues* kvUpdate = m_kvUpdates->GetFirstTrueSubKey();
			kvUpdate;
			kvUpdate = kvUpdate->GetNextTrueSubKey())
		{
			if (Q_stricmp(kvUpdate->GetName(), pszPresetName) == 0)
				return kvUpdate;
		}

		return NULL;
	}

	void CFFOptionsPresetPage::UpdatePresetFromControls(KeyValues* kvPreset)
	{
		if (!FindPendingPresetUpdate(kvPreset->GetName()))
		{
			m_kvUpdates->AddSubKey(new KeyValues(kvPreset->GetName()));
		}

		SendUpdatedPresetPreviewToPresetAssignment(kvPreset->GetName(), kvPreset);
	}

	bool CFFOptionsPresetPage::HasPreset(const char* pszPresetName)
	{
		if (!pszPresetName || !pszPresetName[0])
			return false;

		for (int row = 0; row < m_pPresets->GetItemCount(); ++row)
		{
			KeyValues* kvPreset = m_pPresets->GetItemUserData(m_pPresets->GetItemIDFromRow(row));
			if (Q_stricmp(pszPresetName, kvPreset->GetName()) == 0)
				return true;
		}

		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Give the keyvalue data associated with the given name
	//-----------------------------------------------------------------------------
	KeyValues* CFFOptionsPresetPage::GetPresetDataByName(char const* styleName)
	{
		if (Q_strcmp(styleName, "") == 0)
			return NULL;

		for (int i = 0; i < m_pPresets->GetItemCount(); ++i)
		{
			//if name exists
			if (Q_stricmp(styleName, m_pPresets->GetItemUserData(i)->GetName()) == 0)
			{
				return m_pPresets->GetItemUserData(i)->MakeCopy();
			}
		}

		//if we got here then it wasn't found so return null
		return NULL;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Give the keyvalue data associated with the given name
	//-----------------------------------------------------------------------------
	KeyValues* CFFOptionsPresetPage::GetPresetData()
	{
		KeyValues* kvPresets = new KeyValues("Presets");

		for (int i = 0; i < m_pPresets->GetItemCount(); ++i)
		{
			KeyValues* kvPreset = m_pPresets->GetItemUserData(i);

			KeyValues* kvPresetToSave = new KeyValues(kvPreset->GetName());
			kvPreset->CopySubkeys(kvPresetToSave);
			kvPresetToSave->SetInt(
				"version",
				kvPreset->GetInt("version", FFQuantityHelper::PRESET_DATA_VERSION));
			kvPresets->AddSubKey(kvPresetToSave);
		}

		return kvPresets;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void CFFOptionsPresetPage::OnUpdateCombos(KeyValues* data)
	{
		if (m_bLoaded && data->GetPtr("panel") == m_pPresets)
		{
			ApplyPresetToControls(m_pPresets->GetActiveItemUserData());
		}
	}

	void CFFOptionsPresetPage::OnShowNameInputCommand(KeyValues* data)
	{
		ShowNameInput(data->GetString("defaultName", NULL));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch name input dialog's okay button and process the text
	//-----------------------------------------------------------------------------
	void CFFOptionsPresetPage::OnInputDialogCommand(KeyValues* data)
	{
		KeyValues* kvContext = data->GetFirstTrueSubKey();
		if (m_pCopyDefaultPanel && kvContext)
			m_bAssignCopiedPreset = kvContext->GetBool("assign", true);

		//if name empty
		if (Q_stricmp(data->GetString("text"), "") == 0)
		{
			//tell the user what's wrong
			KeyValues* context 
				= data->GetFirstTrueSubKey();

			const char* defaultName
				= context
				? context->GetString("name", NULL)
				: NULL;

			ShowNameInputError(
				"#GameUI_ErrorPresetNameEmpty", 
				defaultName);

			return;
		}

		//get localisation of default
		wchar_t* wszDefault = g_pVGuiLocalize->Find("#GameUI_Default");
		char szDefault[32];
		g_pVGuiLocalize->ConvertUnicodeToANSI(wszDefault, szDefault, sizeof(szDefault));

		//if name is default (a reserved name) or equal to the localization of GameUI_Default
		if (Q_stricmp(data->GetString("text"), "Default") == 0 || Q_stricmp(data->GetString("text"), szDefault) == 0)
		{
			//tell the user what's wrong
			KeyValues* context
				= data->GetFirstTrueSubKey();

			const char* defaultName
				= context
				? context->GetString("name", NULL)
				: NULL;

			ShowNameInputError(
				"#GameUI_ErrorPresetNameReserved",
				defaultName);

			return;
		}

		for (int i = 0; i < m_pPresets->GetItemCount(); ++i)
		{
			//if name exists
			if (Q_stricmp(data->GetString("text"), m_pPresets->GetItemUserData(i)->GetName()) == 0)
			{
				//if renaming to the same name as it was then just continue without any error
				if (m_bRename && m_pPresets->GetActiveItem() == i)
				{
					m_bRename = false;
					return;
				}

				//tell the user what's wrong
				ShowNameInputError(
					"#GameUI_ErrorPresetNameDuplicate",
					data->GetString("text"));

				return;
			}
		}

		if (m_bRename)
		{
			const char* szOldName = m_pPresets->GetActiveItemUserData()->GetName();

			KeyValues* kvPreset = new KeyValues(data->GetString("text"));
			m_pPresets->GetActiveItemUserData()->CopySubkeys(kvPreset);
			m_pPresets->UpdateItem(m_pPresets->GetActiveItem(), data->GetString("text"), kvPreset);
			//makes the name update in the combobox
			m_pPresets->ActivateItem(m_pPresets->GetActiveItem());
			kvPreset->deleteThis();

			KeyValues* kvUpdate = FindPendingPresetUpdate(szOldName);
			if (kvUpdate)
			{
				const char* pszNewName = data->GetString("text");
				if (FindPendingPresetUpdate(pszNewName))
				{
					m_kvUpdates->RemoveSubKey(kvUpdate);
					kvUpdate->deleteThis();
				}
				else
				{
					kvUpdate->SetName(pszNewName);
				}
			}

			SendRenamedPresetToPresetAssignment(
				szOldName,
				data->GetString("text"));
			m_bRename = false;
		}
		else
		{
			KeyValues* kvPreset = new KeyValues(data->GetString("text"));
			if (m_bCopy)
				//copy
			{
				//copy from current
				m_pPresets->GetActiveItemUserData()->CopySubkeys(kvPreset);
				m_bCopy = false;
			}
			else
				//new
			{
				if (m_kvPanelDefaultCopy)
				{
					//copy from panel
					m_kvPanelDefaultCopy->CopySubkeys(kvPreset);
					m_kvPanelDefaultCopy->deleteThis();
					m_kvPanelDefaultCopy = NULL;
				}
				else
				{
					//get the control defaults and make a preset out of them
					KeyValues* kvCleanedPreset = RemoveNonEssentialValues(kvPreset);
					kvPreset->deleteThis();
					kvPreset = kvCleanedPreset;
				}
			}

			AddPreset(kvPreset);

			if (m_pCopyDefaultPanel && m_bAssignCopiedPreset)
			{
				g_AP->AssignPresetToPanel(
					m_pCopyDefaultPanel,
					m_szPresetTypeName,
					kvPreset->GetName());
			}

			m_pCopyDefaultPanel = NULL;
			kvPreset->deleteThis();
			kvPreset = NULL;

			if (!m_pPresets->IsEnabled())
			{
				SetPresetControlsEnabled(true);
			}
		}
	}

	/*
	If the function calling this is finished with kvPreset then it should be deleted in that function
	*/
	void CFFOptionsPresetPage::AddPreset(KeyValues* kvPreset)
	{
		kvPreset->SetInt("version", FFQuantityHelper::PRESET_DATA_VERSION);

		//add the new item to the presets dropdown
		m_pPresets->AddItem(kvPreset->GetName(), kvPreset);
		m_pPresets->ActivateItemByRow(m_pPresets->GetItemCount() - 1);

		// Make new and copied presets selectable before Apply is pressed.
		SendNewPresetToPresetAssignment(kvPreset->GetName(), kvPreset->MakeCopy());
	}

	void CFFOptionsPresetPage::CreatePresetFromPanelDefault(KeyValues* kvPreset, void* pHudPanel)
	{
		SetVisible(true);
		ActivatePresetPage();

		m_kvPanelDefaultCopy = RemoveNonEssentialValues(kvPreset);
		m_pCopyDefaultPanel = pHudPanel;
		m_bAssignCopiedPreset = true;
		m_bCopy = false;
		m_bRename = false;

		ShowNameInput();
	}

	void CFFOptionsPresetPage::OnNameInputCanceled()
	{
		if (m_kvPanelDefaultCopy)
		{
			m_kvPanelDefaultCopy->deleteThis();
			m_kvPanelDefaultCopy = NULL;
		}

		m_pCopyDefaultPanel = NULL;
		m_bCopy = false;
		m_bRename = false;
	}

	void CFFOptionsPresetPage::ShowNameInput(
		const char* szDefaultName)
	{
		InputDialog* nameInput
			= new CFFHudPresetNameInputDialog(
				this,
				szDefaultName,
				m_pCopyDefaultPanel != NULL,
				m_bAssignCopiedPreset);

		KeyValues* context
			= szDefaultName
			? new KeyValues(
				"PresetNameContext",
				"name",
				szDefaultName)
			: NULL;

		if (!context)
			context = new KeyValues("PresetNameContext");

		nameInput->DoModal(context);
	}

	void CFFOptionsPresetPage::ShowNameInputError(
		const char* szReason,
		const char* szDefaultName)
	{
		MessageBox* errorDialog
			= new MessageBox(
				"#GameUI_ErrorPresetNameTitle",
				szReason,
				this);

		KeyValues* command
			= new KeyValues(
				"ShowNameInput",
				"defaultName",
				szDefaultName);

		errorDialog->AddActionSignalTarget(this);
		errorDialog->SetCommand(command);
		errorDialog->DoModal();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the buttons. New Copy Delete Rename
	//-----------------------------------------------------------------------------
	void CFFOptionsPresetPage::OnButtonCommand(KeyValues* data)
	{
		const char* pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "NewPreset") == 0 || Q_strcmp(pszCommand, "CopyPreset") == 0 || Q_strcmp(pszCommand, "RenamePreset") == 0)
		{
			if (Q_strcmp(pszCommand, "CopyPreset") == 0)
			{
				m_bCopy = true;
				ShowNameInput();
			}
			else if (Q_strcmp(pszCommand, "RenamePreset") == 0)
			{
				m_bRename = true;
				ShowNameInput(m_pPresets->GetActiveItemUserData()->GetName());
			}
			else
			{
				ShowNameInput();
			}
		}
		else if (Q_strcmp(pszCommand, "DeletePreset") == 0)
		{
			int index = m_pPresets->GetActiveItem();

			int iCount = m_kvChanges->GetInt("Count", 0);
			const char* szCount = m_kvChanges->GetString("Count", "0");
			//the idea is that the current count key doesn't exist and gets created
			KeyValues* kvCommand = new KeyValues(szCount);
			m_kvChanges->SetInt("Count", iCount + 1);
			kvCommand->SetString("Delete", m_pPresets->GetActiveItemUserData()->GetName());

			// Remove the assignment choice now; commit the deletion on Apply.
			SendDeletedPresetToPresetAssignment(kvCommand->GetString("Delete"), true);

			//deleting never shifts indexes so lets do it ourselves!!
			for (int tempIndex = index; tempIndex < m_pPresets->GetItemCount() - 1; ++tempIndex)
			{
				m_pPresets->UpdateItem(tempIndex, m_pPresets->GetItemUserData(tempIndex + 1)->GetName(), m_pPresets->GetItemUserData(tempIndex + 1));
			}
			//now delete the end one
			m_pPresets->DeleteItem(m_pPresets->GetItemCount() - 1);

			//if we just deleted the last preset
			if (m_pPresets->GetItemCount() == 0)
				//disable all the controls
			{
				//remove the text remaining from the last removed item
				m_pPresets->SetText("");
				SetPresetControlsEnabled(false);
			}
			else
			{
				if (index < m_pPresets->GetItemCount())
					m_pPresets->ActivateItemByRow(index);
				else
					m_pPresets->ActivateItemByRow(m_pPresets->GetItemCount() - 1);
			}

			m_kvChanges->AddSubKey(kvCommand);
		}
	}
};