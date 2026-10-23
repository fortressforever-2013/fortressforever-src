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

#define DAMAGENUMBER_COLOR_ARMOR	Color( 184, 207, 239, 255 )
#define DAMAGENUMBER_COLOR_NOARMOR	Color( 239, 184, 184, 255 )
#define DAMAGENUMBER_COLOR_BUILDABLE	Color( 255, 255, 255, 255 )
#define DAMAGENUMBER_COLOR_FRIENDLYFIRE	Color( 255, 0, 0, 255 )
#define DAMAGENUMBER_COLOR_HEALING	Color( 15, 210, 15, 255 )
#define DAMAGENUMBER_COLOR_REPAIRING	Color( 255, 130, 50, 255 )
#define DAMAGENUMBER_TARGET_NOARMOR	0
#define DAMAGENUMBER_TARGET_ARMOR	1
#define DAMAGENUMBER_TARGET_BUILDABLE	2
#define DAMAGENUMBER_TARGET_FRIENDLYFIRE	3
#define DAMAGENUMBER_TARGET_HEALING	4
#define DAMAGENUMBER_TARGET_REPAIRING	5

#define SPECIALTEXT_HEADSHOT	1
#define SPECIALTEXT_LEGSHOT	2
#define SPECIALTEXT_CONCUSSED	3
#define SPECIALTEXT_BACKSTAB	4
#define SPECIALTEXT_TRANQED	5
#define SPECIALTEXT_INFECTED	6
#define SPECIALTEXT_OFFSET	24.0f
#define SPECIALTEXT_FLOAT_TIME	1.0f
#define SPECIALTEXT_FLOAT_SPEED	12.0f

ConVar hud_damagenumbers ("hud_damagenumbers", "1", FCVAR_ARCHIVE, "Show floating damage numbers and special text as you hit someone");
// Super shotgun has a 0.7s cycle time (time between shots).
ConVar hud_damagenumbers_accumulate_time("hud_damagenumbers_accumulate_time", "0.75", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Seconds after the last hit to combine damage on the same target before rising and fading.", true, 0.0f, false, 0.0f);
ConVar hud_damagenumbers_float_speed("hud_damagenumbers_float_speed", "12.0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Damage number rise speed in world units per second (0 disables movement).", true, 0.0f, false, 0.0f);
ConVar hud_damagenumbers_float_time("hud_damagenumbers_float_time", "1.0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Seconds damage numbers rise and fade after combining ends.", true, 0.01f, false, 0.0f);
ConVar hud_damagenumbers_height_offset("hud_damagenumbers_height_offset", "44.0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Vertical offset in world units for damage numbers and special hit text, applied on the next hit.");
ConVar hud_damagenumbers_replacement_jump("hud_damagenumbers_replacement_jump", "16", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Screen pixels to move an older damage total upward when a new total appears on the same target (0 disables the jump).", true, 0.0f, false, 0.0f);

struct SpecialTextEntry_t
{
	int		iTargetEntIndex;
	int		iTextType;
	int		iTargetType;
	Vector	vecPosition;
	float	flSpawnTime;
};

struct DamageNumberEntry_t
{
	int	iTargetEntIndex;
	int	iAccumulatedDamage;
	int	iTargetType;
	float	flLastHitTime;
	Vector	vecPosition;
	bool	bFloating;
	float	flFloatStartTime;
	bool	bReplaced;
};

struct AirshotTextEntry_t
{
	int		iTargetEntIndex;
	int		iTargetType;
	Vector	vecPosition;
	float	flSpawnTime;
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
	void MsgFunc_SpecialText(bf_read& msg);
	void MsgFunc_AirshotText(bf_read& msg);
protected:
	virtual void Paint(void);
	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);
private:
	CUtlVector< DamageNumberEntry_t > m_DamageEntries;
	CUtlVector< SpecialTextEntry_t > m_SpecialTextEntries;
	CUtlVector< AirshotTextEntry_t > m_AirshotEntries;
	vgui::HFont m_hNumberFont;
};

DECLARE_HUDELEMENT(CHudDamageNumber);
DECLARE_HUD_MESSAGE(CHudDamageNumber, DamageNumber);
DECLARE_HUD_MESSAGE(CHudDamageNumber, SpecialText);
DECLARE_HUD_MESSAGE(CHudDamageNumber, AirshotText);

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
	HOOK_HUD_MESSAGE(CHudDamageNumber, SpecialText);
	HOOK_HUD_MESSAGE(CHudDamageNumber, AirshotText);
	m_DamageEntries.Purge();
	m_SpecialTextEntries.Purge();
	m_AirshotEntries.Purge();
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
	m_SpecialTextEntries.Purge();
	m_AirshotEntries.Purge();
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
	if (!hud_damagenumbers.GetBool())
	return false;

	return m_DamageEntries.Count() > 0 || m_SpecialTextEntries.Count() > 0 || m_AirshotEntries.Count() > 0;
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

		if ((flNow - entry.flLastHitTime) > hud_damagenumbers_accumulate_time.GetFloat())
		continue;

		entry.iAccumulatedDamage += iDamage;
		entry.iTargetType = iTargetType;
		entry.flLastHitTime = flNow;
		entry.vecPosition = vecPosition + Vector(0, 0, hud_damagenumbers_height_offset.GetFloat());

		return;
	}

	// Separate older totals from a new total on the same target.
	// Keep their existing rise and fade progress; Paint adds the replacement jump.
	for (int i = 0; i < m_DamageEntries.Count(); i++)
	{
		DamageNumberEntry_t& entry = m_DamageEntries[i];
		if (entry.iTargetEntIndex == iTargetEntIndex)
			entry.bReplaced = true;
	}

	DamageNumberEntry_t newEntry;
	newEntry.iTargetEntIndex = iTargetEntIndex;
	newEntry.iAccumulatedDamage = iDamage;
	newEntry.iTargetType = iTargetType;
	newEntry.flLastHitTime = flNow;
	newEntry.vecPosition = vecPosition + Vector(0, 0, hud_damagenumbers_height_offset.GetFloat());
	newEntry.bFloating = false;
	newEntry.flFloatStartTime = 0.0f;
	newEntry.bReplaced = false;

	m_DamageEntries.AddToTail(newEntry);
}

void CHudDamageNumber::MsgFunc_SpecialText(bf_read& msg)
{
	int iTargetEntIndex = msg.ReadShort();
	int iTextType = msg.ReadByte();
	int iTargetType = msg.ReadByte();
	Vector vecPosition;
	vecPosition.x = msg.ReadFloat();
	vecPosition.y = msg.ReadFloat();
	vecPosition.z = msg.ReadFloat();
	for (int i = 0; i < m_SpecialTextEntries.Count(); i++)
	{
		if (m_SpecialTextEntries[i].iTargetEntIndex == iTargetEntIndex)
		{
			m_SpecialTextEntries[i].iTextType = iTextType;
			m_SpecialTextEntries[i].iTargetType = iTargetType;
			m_SpecialTextEntries[i].vecPosition = vecPosition + Vector(0, 0, hud_damagenumbers_height_offset.GetFloat());
			m_SpecialTextEntries[i].flSpawnTime = gpGlobals->curtime;
			return;
		}
	}
	SpecialTextEntry_t newEntry;
	newEntry.iTargetEntIndex = iTargetEntIndex;
	newEntry.iTextType = iTextType;
	newEntry.iTargetType = iTargetType;
	newEntry.vecPosition = vecPosition + Vector(0, 0, hud_damagenumbers_height_offset.GetFloat());
	newEntry.flSpawnTime = gpGlobals->curtime;
	m_SpecialTextEntries.AddToTail(newEntry);
}

void CHudDamageNumber::MsgFunc_AirshotText(bf_read& msg)
{
	int iTargetEntIndex = msg.ReadShort();
	int iTargetType = msg.ReadByte();
	Vector vecPosition;
	vecPosition.x = msg.ReadFloat();
	vecPosition.y = msg.ReadFloat();
	vecPosition.z = msg.ReadFloat();

	for (int i = 0; i < m_AirshotEntries.Count(); i++)
	{
		if (m_AirshotEntries[i].iTargetEntIndex == iTargetEntIndex)
		{
			m_AirshotEntries[i].iTargetType = iTargetType;
			m_AirshotEntries[i].vecPosition = vecPosition;
			m_AirshotEntries[i].flSpawnTime = gpGlobals->curtime;
			return;
		}
	}

	AirshotTextEntry_t newEntry;
	newEntry.iTargetEntIndex = iTargetEntIndex;
	newEntry.iTargetType = iTargetType;
	newEntry.vecPosition = vecPosition;
	newEntry.flSpawnTime = gpGlobals->curtime;
	m_AirshotEntries.AddToTail(newEntry);
}

void CHudDamageNumber::Paint(void)
{
	float flNow = gpGlobals->curtime;

	for (int i = m_DamageEntries.Count() - 1; i >= 0; i--)
	{
		DamageNumberEntry_t& entry = m_DamageEntries[i];

		if (!entry.bFloating)
		{
			if ((flNow - entry.flLastHitTime) > hud_damagenumbers_accumulate_time.GetFloat())
			{
				entry.bFloating = true;
				entry.flFloatStartTime = entry.flLastHitTime + hud_damagenumbers_accumulate_time.GetFloat();
			}
		}

		float flAlpha = 255.0f;
		Vector vecDrawPos = entry.vecPosition;

		if (entry.bFloating)
		{
			float flElapsed = flNow - entry.flFloatStartTime;

			if (flElapsed > hud_damagenumbers_float_time.GetFloat())
			{
				m_DamageEntries.Remove(i);
				continue;
			}

			vecDrawPos.z += flElapsed * hud_damagenumbers_float_speed.GetFloat();
			flAlpha = RemapValClamped(flElapsed, 0.0f, hud_damagenumbers_float_time.GetFloat(), 255.0f, 0.0f);
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
		// Separate the old total from the new one regardless of distance to the target.
		if (entry.bReplaced)
			iScreenY -= hud_damagenumbers_replacement_jump.GetInt();
		surface()->DrawSetTextPos(iScreenX - (iTextWide / 2), iScreenY - (iTextTall / 2));
		surface()->DrawUnicodeString(wszDamage);
	}

	for (int i = m_SpecialTextEntries.Count() - 1; i >= 0; i--)
	{
		SpecialTextEntry_t& label = m_SpecialTextEntries[i];
		float flElapsed = flNow - label.flSpawnTime;
		if (flElapsed > SPECIALTEXT_FLOAT_TIME)
		{
			m_SpecialTextEntries.Remove(i);
			continue;
		}

		Vector vecLabelPos = label.vecPosition;
		vecLabelPos.z += flElapsed * SPECIALTEXT_FLOAT_SPEED;
		float flAlpha = RemapValClamped(flElapsed, 0.0f, SPECIALTEXT_FLOAT_TIME, 255.0f, 0.0f);
		int iLabelScreenX, iLabelScreenY;
		if (!GetVectorInScreenSpace(vecLabelPos, iLabelScreenX, iLabelScreenY))
		continue;
		iLabelScreenY -= SPECIALTEXT_OFFSET;
		Color textColor;
		switch (label.iTargetType)
		{
		default: textColor = DAMAGENUMBER_COLOR_NOARMOR;	break;
		case DAMAGENUMBER_TARGET_ARMOR:	textColor = DAMAGENUMBER_COLOR_ARMOR;	break;
		case DAMAGENUMBER_TARGET_BUILDABLE:	textColor = DAMAGENUMBER_COLOR_BUILDABLE;	break;
		case DAMAGENUMBER_TARGET_FRIENDLYFIRE:	textColor = DAMAGENUMBER_COLOR_FRIENDLYFIRE;	break;
		case DAMAGENUMBER_TARGET_HEALING:	textColor = DAMAGENUMBER_COLOR_HEALING;	break;
		case DAMAGENUMBER_TARGET_REPAIRING:	textColor = DAMAGENUMBER_COLOR_REPAIRING;	break;
		}
		const wchar_t* pwszLabel = L"";
		switch (label.iTextType)
		{
		case SPECIALTEXT_HEADSHOT:	pwszLabel = L"HEADSHOT!";	break;
		case SPECIALTEXT_LEGSHOT:	pwszLabel = L"LEGSHOT!";	break;
		case SPECIALTEXT_CONCUSSED:	pwszLabel = L"CONCUSSED!";	break;
		case SPECIALTEXT_BACKSTAB:	pwszLabel = L"BACKSTAB!";	break;
		case SPECIALTEXT_TRANQED:	pwszLabel = L"TRANQUILIZED!";	break;
		case SPECIALTEXT_INFECTED:	pwszLabel = L"INFECTED!";	break;
		}
		surface()->DrawSetTextFont(m_hNumberFont);
		surface()->DrawSetTextColor(textColor.r(), textColor.g(), textColor.b(), (int)flAlpha);
		int iLabelWide, iLabelTall;
		surface()->GetTextSize(m_hNumberFont, pwszLabel, iLabelWide, iLabelTall);
		surface()->DrawSetTextPos(iLabelScreenX - (iLabelWide / 2), iLabelScreenY - (iLabelTall / 2));
		surface()->DrawUnicodeString(pwszLabel);
	}

	for (int i = m_AirshotEntries.Count() - 1; i >= 0; i--)
	{
		AirshotTextEntry_t& entry = m_AirshotEntries[i];
		float flElapsed = flNow - entry.flSpawnTime;
		if (flElapsed > SPECIALTEXT_FLOAT_TIME)
		{
			m_AirshotEntries.Remove(i);
			continue;
		}
		Vector vecDrawPos = entry.vecPosition;
		vecDrawPos.z += flElapsed * SPECIALTEXT_FLOAT_SPEED;
		int iScreenX, iScreenY;
		if (!GetVectorInScreenSpace(vecDrawPos, iScreenX, iScreenY))
		continue;
		float flAlpha = RemapValClamped(flElapsed, 0.0f, SPECIALTEXT_FLOAT_TIME, 255.0f, 0.0f);
		Color textColor;
		switch (entry.iTargetType)
		{
		default: textColor = DAMAGENUMBER_COLOR_NOARMOR;	break;
		case DAMAGENUMBER_TARGET_ARMOR:	textColor = DAMAGENUMBER_COLOR_ARMOR;	break;
		case DAMAGENUMBER_TARGET_FRIENDLYFIRE:	textColor = DAMAGENUMBER_COLOR_FRIENDLYFIRE;	break;
		}
		const wchar_t* pwszLabel = L"AIRSHOT!";
		surface()->DrawSetTextFont(m_hNumberFont);
		surface()->DrawSetTextColor(textColor.r(), textColor.g(), textColor.b(), (int)flAlpha);
		int iLabelWide, iLabelTall;
		surface()->GetTextSize(m_hNumberFont, pwszLabel, iLabelWide, iLabelTall);
		surface()->DrawSetTextPos(iScreenX - (iLabelWide / 2), iScreenY - (iLabelTall / 2));
		surface()->DrawUnicodeString(pwszLabel);
	}
}
