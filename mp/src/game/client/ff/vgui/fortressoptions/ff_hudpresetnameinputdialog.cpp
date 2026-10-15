#include "cbase.h"
#include "ff_hudpresetnameinputdialog.h"

using namespace vgui;

CFFHudPresetNameInputDialog::CFFHudPresetNameInputDialog(
	Panel* parent,
	const char* defaultName,
	bool bShowAssign,
	bool bAssign)
	: BaseClass(
		parent,
		"#GameUI_EnterPresetName",
		"",
		defaultName)
{
	m_pAssign = new CheckButton(this, "Assign", "#GameUI_Assign");
	m_pAssign->SetVisible(bShowAssign);
	m_pAssign->SetSelected(bAssign);

	if (bShowAssign)
		SetTall(160);
}

void CFFHudPresetNameInputDialog::PerformLayout(
	int x,
	int y,
	int wide,
	int tall)
{
	BaseClass::PerformLayout(
		x,
		y,
		wide,
		tall);

	m_pAssign->SetBounds(
		x,
		y + 60,
		wide,
		24);
}

void CFFHudPresetNameInputDialog::OnCommand(const char* command)
{
	if (Q_stricmp(command, "OK") == 0 || Q_stricmp(command, "Cancel") == 0)
	{
		if (m_bFinished)
			return;

		m_bFinished = true;

		if (m_pContextKeyValues)
			m_pContextKeyValues->SetBool("assign", m_pAssign->IsSelected());
	}

	BaseClass::OnCommand(command);
}

void CFFHudPresetNameInputDialog::OnClose()
{
	if (!m_bFinished)
	{
		m_bFinished = true;
		PostActionSignal(new KeyValues("InputCanceled"));
	}

	BaseClass::OnClose();
}

void CFFHudPresetNameInputDialog::OnKeyCodeTyped(KeyCode code)
{
	if (code == KEY_ENTER)
	{
		OnCommand("OK");
		return;
	}

	BaseClass::OnKeyCodeTyped(code);
}
