#ifndef FF_HUDPRESETNAMEINPUTDIALOG_H
#define FF_HUDPRESETNAMEINPUTDIALOG_H

#include <vgui_controls/InputDialog.h>
#include <vgui_controls/CheckButton.h>

class CFFHudPresetNameInputDialog : public vgui::InputDialog
{
	DECLARE_CLASS_SIMPLE(CFFHudPresetNameInputDialog, vgui::InputDialog);

public:
	CFFHudPresetNameInputDialog(
		vgui::Panel* parent,
		const char* defaultName,
		bool bShowAssign,
		bool bAssign);

protected:
	void PerformLayout(
		int x,
		int y,
		int wide,
		int tall) override;
	void OnCommand(const char* command) override;
	void OnClose() override;
	void OnKeyCodeTyped(vgui::KeyCode code) override;

private:
	vgui::CheckButton* m_pAssign;
	bool m_bFinished = false;
};

#endif
