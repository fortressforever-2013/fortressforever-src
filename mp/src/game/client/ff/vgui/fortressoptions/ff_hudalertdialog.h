#ifndef FF_HUDALERTDIALOG_H
#define FF_HUDALERTDIALOG_H

#include <vgui_controls/Frame.h>

namespace vgui
{
	class ComboBox;
	class FFColorPicker;
}

class CFFHudAlertDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CFFHudAlertDialog, vgui::Frame);

public:
	CFFHudAlertDialog(vgui::Panel* parent, KeyValues* kvValues, void* pHudPanel);

	void OnCommand(const char* command) override;

private:
	void AddChoice(vgui::ComboBox* combo, const char* caption, int value);
	void SelectValue(vgui::ComboBox* combo, int value);

	vgui::ComboBox* m_pTarget;
	vgui::ComboBox* m_pDuration;
	vgui::FFColorPicker* m_pColor;
	void* m_pHudPanel;
};

#endif
