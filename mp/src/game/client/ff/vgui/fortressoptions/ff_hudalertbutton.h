#ifndef FF_HUDALERTBUTTON_H
#define FF_HUDALERTBUTTON_H

#include <vgui_controls/Button.h>

class CFFHudAlertButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE(CFFHudAlertButton, vgui::Button);

public:
	CFFHudAlertButton(vgui::Panel* parent, const char* name, vgui::Panel* target);

protected:
	void ApplySchemeSettings(vgui::IScheme* pScheme) override;

private:
	vgui::HFont m_hIconFont = 0;
};

#endif
