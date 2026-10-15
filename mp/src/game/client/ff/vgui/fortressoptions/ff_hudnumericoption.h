#ifndef FF_HUDNUMERICOPTION_H
#define FF_HUDNUMERICOPTION_H

#include <vgui_controls/TextEntry.h>

class CFFCustomHudAssignPresets;

class CFFHudNumericOption : public vgui::TextEntry
{
	DECLARE_CLASS_SIMPLE(CFFHudNumericOption, vgui::TextEntry);

public:
	CFFHudNumericOption(
		vgui::Panel* parent,
		const char* name,
		CFFCustomHudAssignPresets* target,
		void* pHudPanel,
		int value,
		int minimum,
		int maximum);

	void CommitValue();

protected:
	void OnKillFocus() override;
	void OnKeyCodeTyped(vgui::KeyCode code) override;

private:
	CFFCustomHudAssignPresets* m_pTarget;
	void* m_pHudPanel;
	int m_iMinimum;
	int m_iMaximum;
	int m_iValue;
};

#endif