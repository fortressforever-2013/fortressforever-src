#include "cbase.h"
#include "ff_hudnumericoption.h"

#include "ff_customhudoptions_assignpresets.h"

using namespace vgui;

CFFHudNumericOption::CFFHudNumericOption(
	Panel* parent,
	const char* name,
	CFFCustomHudAssignPresets* target,
	void* pHudPanel,
	int value,
	int minimum,
	int maximum)
	: BaseClass(parent, name), m_pHudPanel(pHudPanel), m_iMinimum(minimum), m_iMaximum(maximum)
{
	// Ensure max char count is acceptable
	Assert(maximum <= 9999);
	SetMaximumCharCount(4);

	RemoveActionSignalTarget(parent);
	SetAllowNumericInputOnly(true);
	m_iValue = clamp(value, minimum, maximum);
	SetText(VarArgs("%d", m_iValue));
	m_pTarget = target;
}

void CFFHudNumericOption::OnKillFocus()
{
	CommitValue();
	BaseClass::OnKillFocus();
}

void CFFHudNumericOption::OnKeyCodeTyped(KeyCode code)
{
	int delta = 0;
	switch (code)
	{
	case KEY_UP:
	case KEY_RIGHT:
		delta = +1;
		break;

	case KEY_DOWN:
	case KEY_LEFT:
		delta = -1;
		break;

	case KEY_ENTER:
		CommitValue();
		return;

	default:
		BaseClass::OnKeyCodeTyped(code);
		return;
	}

	char text[16];
	GetText(text, sizeof(text));
	int value = m_iValue;
	if (text[0])
		value = atoi(text);

	SetText(VarArgs("%d", value + delta));
	CommitValue();
}

void CFFHudNumericOption::CommitValue()
{
	char text[16];
	GetText(text, sizeof(text));
	int value = m_iValue;
	if (text[0])
		value = clamp(atoi(text), m_iMinimum, m_iMaximum);

	SetText(VarArgs("%d", value));
	if (value == m_iValue)
		return;

	m_iValue = value;
	m_pTarget->UpdateNumericOption(m_pHudPanel, GetName(), value);
}
