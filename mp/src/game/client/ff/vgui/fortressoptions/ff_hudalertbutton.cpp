#include "cbase.h"
#include "ff_hudalertbutton.h"

#include <vgui/IScheme.h>
#include <vgui/ISurface.h>

using namespace vgui;

CFFHudAlertButton::CFFHudAlertButton(Panel* parent, const char* name, Panel* target)
	: BaseClass(
		parent,
		name,
		"4",
		target,
		"OpenAlertSettings")
{
}

void CFFHudAlertButton::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	if (!m_hIconFont)
		m_hIconFont = surface()->CreateFont();

	surface()->SetFontGlyphSet(
		m_hIconFont,
		"FortressForever - Status Glyphs",
		28,
		0,
		0,
		0,
		ISurface::FONTFLAG_ANTIALIAS | ISurface::FONTFLAG_ADDITIVE);

	SetFont(m_hIconFont);
	SetTextInset(0, 0);
	SetContentAlignment(Label::a_center);
}
