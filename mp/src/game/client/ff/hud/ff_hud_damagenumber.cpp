// Fortress Forever
// Damage Numbers Logic
// ff_hud_hitindicator.cpp and ff_hud_radiotag.cpp were used as the basement 

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "cdll_util.h"

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

using namespace vgui;

#define DAMAGENUMBER_ACCUMULATE_TIME	0.6f
#define DAMAGENUMBER_FLOAT_TIME	1.0f
#define DAMAGENUMBER_FLOAT_SPEED	12.0f
#define DAMAGENUMBER_HEIGHT_OFFSET	64.0f
#define DAMAGENUMBER_COLOR_ARMOR	Color( 184, 207, 239, 255 )
#define DAMAGENUMBER_COLOR_NOARMOR	Color( 239, 184, 184, 255 )
#define DAMAGENUMBER_COLOR_BUILDABLE	Color( 255, 255, 255, 255 )
#define DAMAGENUMBER_COLOR_FRIENDLYFIRE	Color( 255, 0, 0, 255 )
#define DAMAGENUMBER_COLOR_HEALING	Color( 0, 255, 0, 255 )
#define DAMAGENUMBER_COLOR_REPAIRING	Color( 255, 127, 0, 255 )
#define DAMAGENUMBER_TARGET_NOARMOR	0
#define DAMAGENUMBER_TARGET_ARMOR	1
#define DAMAGENUMBER_TARGET_BUILDABLE	2
#define DAMAGENUMBER_TARGET_FRIENDLYFIRE	3
#define DAMAGENUMBER_TARGET_HEALING	4
#define DAMAGENUMBER_TARGET_REPAIRING	5

struct DamageNumberEntry_t
{
	int	iTargetEntIndex;
	int	iAccumulatedDamage;
	int	iTargetType;
	float	flLastHitTime;
	Vector	vecPosition;
	bool	bFloating;
	float	flFloatStartTime;
};

class CHudDamageNumber : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudDamageNumber, vgui::Panel);

public:
	CHudDamageNumber(const char* pElementName);
	void Init(void);
	void VidInit(void);
	void Reset(void);
	virtual bool ShouldDraw(void);
	void MsgFunc_DamageNumber(bf_read& msg);
protected:
	virtual void Paint(void);
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);
private:
	CUtlVector< DamageNumberEntry_t > m_DamageEntries;
	vgui::HFont m_hNumberFont;
};

DECLARE_HUDELEMENT(CHudDamageNumber);
DECLARE_HUD_MESSAGE(CHudDamageNumber, DamageNumber);

CHudDamageNumber::CHudDamageNumber(const char* pElementName)
	: CHudElement(pElementName), vgui::Panel(NULL, "HudDamageNumber")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_UNASSIGNED);
}

void CHudDamageNumber::Init(void)
{
	HOOK_HUD_MESSAGE(CHudDamageNumber, DamageNumber);
	m_DamageEntries.Purge();
}

void CHudDamageNumber::VidInit(void)
{
	SetPaintBackgroundEnabled(false);
	int iWide, iTall;
	GetHudSize(iWide, iTall);
	SetPos(0, 0);
	SetWide(iWide);
	SetTall(iTall);
}

void CHudDamageNumber::Reset(void)
{
	m_DamageEntries.Purge();
}

void CHudDamageNumber::ApplySchemeSettings(vgui::IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
	m_hNumberFont = pScheme->GetFont("HudNumbersSmall", true); // normal font is abit unplayable as it covers the whole body at long distances
}

bool CHudDamageNumber::ShouldDraw(void)
{
	if (!CHudElement::ShouldDraw())
		return false;
	return m_DamageEntries.Count() > 0;
}

void CHudDamageNumber::MsgFunc_DamageNumber(bf_read& msg)
{
	int iTargetEntIndex = msg.ReadShort();
	int iDamage = msg.ReadShort();
	int iTargetType = msg.ReadByte();

	Vector vecPosition;
	vecPosition.x = msg.ReadFloat();
	vecPosition.y = msg.ReadFloat();
	vecPosition.z = msg.ReadFloat();

	if (iDamage <= 0)

		return;

	float flNow = gpGlobals->curtime;
	for (int i = 0; i < m_DamageEntries.Count(); i++)
	{
		DamageNumberEntry_t& entry = m_DamageEntries[i];

		if (entry.iTargetEntIndex != iTargetEntIndex)
			continue;

		if (entry.bFloating)
			continue;

		if ((flNow - entry.flLastHitTime) > DAMAGENUMBER_ACCUMULATE_TIME)
			continue;

		entry.iAccumulatedDamage += iDamage;
		entry.iTargetType = iTargetType;
		entry.flLastHitTime = flNow;
		entry.vecPosition = vecPosition + Vector(0, 0, DAMAGENUMBER_HEIGHT_OFFSET);

		return;
	}

	DamageNumberEntry_t newEntry;
	newEntry.iTargetEntIndex = iTargetEntIndex;
	newEntry.iAccumulatedDamage = iDamage;
	newEntry.iTargetType = iTargetType;
	newEntry.flLastHitTime = flNow;
	newEntry.vecPosition = vecPosition + Vector(0, 0, DAMAGENUMBER_HEIGHT_OFFSET);
	newEntry.bFloating = false;
	newEntry.flFloatStartTime = 0.0f;

	m_DamageEntries.AddToTail(newEntry);
}

void CHudDamageNumber::Paint(void)
{
	float flNow = gpGlobals->curtime;

	for (int i = m_DamageEntries.Count() - 1; i >= 0; i--)
	{
		DamageNumberEntry_t& entry = m_DamageEntries[i];

		if (!entry.bFloating)
		{
			if ((flNow - entry.flLastHitTime) > DAMAGENUMBER_ACCUMULATE_TIME)
			{
				entry.bFloating = true;
				entry.flFloatStartTime = flNow;
			}
		}

		float flAlpha = 255.0f;
		Vector vecDrawPos = entry.vecPosition;

		if (entry.bFloating)
		{
			float flElapsed = flNow - entry.flFloatStartTime;

			if (flElapsed > DAMAGENUMBER_FLOAT_TIME)
			{
				m_DamageEntries.Remove(i);
				continue;
			}

			vecDrawPos.z += flElapsed * DAMAGENUMBER_FLOAT_SPEED;
			flAlpha = RemapValClamped(flElapsed, 0.0f, DAMAGENUMBER_FLOAT_TIME, 255.0f, 0.0f);
		}

		int iScreenX, iScreenY;

		if (!GetVectorInScreenSpace(vecDrawPos, iScreenX, iScreenY))
			continue;

		Color textColor;
		switch (entry.iTargetType)
		{
		default: textColor = DAMAGENUMBER_COLOR_NOARMOR;	break;
		case DAMAGENUMBER_TARGET_ARMOR:	textColor = DAMAGENUMBER_COLOR_ARMOR;	break;
		case DAMAGENUMBER_TARGET_BUILDABLE:	textColor = DAMAGENUMBER_COLOR_BUILDABLE;	break;
		case DAMAGENUMBER_TARGET_FRIENDLYFIRE:	textColor = DAMAGENUMBER_COLOR_FRIENDLYFIRE;	break;
		case DAMAGENUMBER_TARGET_HEALING:	textColor = DAMAGENUMBER_COLOR_HEALING;	break;
		case DAMAGENUMBER_TARGET_REPAIRING:	textColor = DAMAGENUMBER_COLOR_REPAIRING;	break;
		}

		wchar_t wszDamage[16];
		if (entry.iTargetType != DAMAGENUMBER_TARGET_HEALING && entry.iTargetType != DAMAGENUMBER_TARGET_REPAIRING)
			V_snwprintf(wszDamage, ARRAYSIZE(wszDamage), L"%d", entry.iAccumulatedDamage); // with the minus it doesnt look clean enough
		else
			V_snwprintf(wszDamage, ARRAYSIZE(wszDamage), L"+%d", entry.iAccumulatedDamage);

		surface()->DrawSetTextFont(m_hNumberFont);
		surface()->DrawSetTextColor(textColor.r(), textColor.g(), textColor.b(), (int)flAlpha);

		int iTextWide, iTextTall;

		surface()->GetTextSize(m_hNumberFont, wszDamage, iTextWide, iTextTall);
		surface()->DrawSetTextPos(iScreenX - (iTextWide / 2), iScreenY - (iTextTall / 2));
		surface()->DrawUnicodeString(wszDamage);
	}
}
