//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include "ammodef.h"
#include "ff_utils.h"
#include "ff_shareddefs.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include "c_ff_player.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

static ConVar hud_speedometer("hud_speedometer", "1", FCVAR_ARCHIVE, "Toggle speedometer. Disclaimer: We are not responsible if you get a ticket.");
static ConVar hud_speedometer_avg("hud_speedometer_avg", "0", FCVAR_ARCHIVE, "Toggle average speedometer.");
static ConVar hud_speedometer_color("hud_speedometer_color", "2", FCVAR_ARCHIVE, "0=No color, 1=Stepped Color, 2=Fading Color (RED > hardcap :: ORANGE > softcap :: GREEN > run speed :: WHITE < run speed)", true, 0.0f, true, 2.0f);
static ConVar hud_speedometer_avg_color("hud_speedometer_avg_color", "0", FCVAR_ARCHIVE, "0=No color, 1=Stepped Color, 2=Fading Color (RED > hardcap :: ORANGE > softcap :: GREEN > run speed :: WHITE < run speed)", true, 0.0f, true, 2.0f);

Color ColorFade(int currentVal, int minVal, int maxVal, Color minColor, Color maxColor);

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudSpeedometer : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE(CHudSpeedometer, vgui::FFPanel);

	CHudSpeedometer(const char* pElementName) : vgui::FFPanel(NULL, "HudSpeedometer"), CHudElement(pElementName)
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_UNASSIGNED);
	}

	virtual ~CHudSpeedometer(void)
	{

	}

	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();
	virtual void Paint();

private:
	float m_flAvgSpeed;
	float m_flSpeed;
	float m_flNextUpdate;

	Color GetSpeedometerColor(int speed, int maxSpeed);
	float AlphaForTarget(float p, float T, float dt);

private:
	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hSpeedFont, "SpeedFont", "HudNumbers");
	CPanelAnimationVar(vgui::HFont, m_hAvgSpeedFont, "AvgSpeedFont", "Default");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "HUD_Tone_Default");

	CPanelAnimationVarAliasType(float, SpeedFont_xpos, "SpeedFont_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, SpeedFont_ypos, "SpeedFont_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, AvgSpeedFont_xpos, "AvgSpeedFont_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, AvgSpeedFont_ypos, "AvgSpeedFont_ypos", "0", "proportional_float");
};

DECLARE_HUDELEMENT(CHudSpeedometer);

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudSpeedometer::Init(void)
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudSpeedometer::VidInit(void)
{
	Reset();
	m_flAvgSpeed = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudSpeedometer::Reset()
{
	m_flNextUpdate = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
void CHudSpeedometer::OnThink()
{
	if (!hud_speedometer.GetBool() && !hud_speedometer_avg.GetBool())
		return;

	C_FFPlayer* pPlayer
		= C_FFPlayer::GetLocalFFPlayerOrAnyObserverTarget();

	if (!pPlayer)
		return;

	if (gpGlobals->curtime < m_flNextUpdate) {
		return;
	}

	Vector vecVelocity
		= pPlayer->GetAbsVelocity();

	m_flSpeed
		= FastSqrt(
			vecVelocity.x * vecVelocity.x
			+ vecVelocity.y * vecVelocity.y);

	const float dt = 0.1f;     // your m_flNextUpdate step
	const float p = 0.8f;      // 80% responsiveness
	const float T = 1.0f;      // within 1 second
	const float alpha          // 0.14866
		= AlphaForTarget(p, T, dt);

	m_flAvgSpeed
		= alpha * m_flSpeed
		+ (1.0f - alpha) * m_flAvgSpeed;

	m_flNextUpdate = gpGlobals->curtime + dt;
}

// Pick alpha to hit fraction p (e.g. 0.8) by time T, given update step dt.
float CHudSpeedometer::AlphaForTarget(float p, float T, float dt) {
	// Clamp to safe ranges
	if (p <= 0.0f) return 0.0f;
	if (p >= 1.0f) return 1.0f;
	if (dt <= 0.0f) return 1.0f;
	return 1.0f - powf(1.0f - p, dt / T);
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudSpeedometer::Paint()
{
	if (!hud_speedometer.GetBool() && !hud_speedometer_avg.GetBool())
		return;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayerOrAnyObserverTarget();

	if (!pPlayer)
		return;

	float maxSpeed = pPlayer->MaxSpeed();

	// regular speedometer
	if (hud_speedometer.GetBool())
	{
		Color speedColor
			= GetSpeedometerColor(
				static_cast<int>(m_flSpeed),
				static_cast<int>(maxSpeed));

		surface()->DrawSetTextFont(m_hSpeedFont);
		surface()->DrawSetTextColor(speedColor);
		surface()->DrawSetTextPos(SpeedFont_xpos, SpeedFont_ypos);

		wchar_t unicode[6];
		V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", (int)m_flSpeed);

		for (wchar_t* wch = unicode; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);
	}

	// average speedometer
	if (hud_speedometer_avg.GetBool())
	{
		Color avgSpeedColor
			= GetSpeedometerColor(
				static_cast<int>(m_flAvgSpeed),
				static_cast<int>(maxSpeed));

		surface()->DrawSetTextFont(m_hAvgSpeedFont);
		if (hud_speedometer.GetBool())
			surface()->DrawSetTextPos(AvgSpeedFont_xpos, AvgSpeedFont_ypos);
		else
			surface()->DrawSetTextPos(AvgSpeedFont_xpos, AvgSpeedFont_ypos + SpeedFont_ypos);

		surface()->DrawSetTextColor(m_TextColor);

		wchar_t textunicode[] = L"Average: ";
		for (wchar_t* wch = textunicode; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);

		surface()->DrawSetTextColor(avgSpeedColor);

		wchar_t unicode[6];
		V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", (int)m_flAvgSpeed);

		for (wchar_t* wch = unicode; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);
	}
}

Color CHudSpeedometer::GetSpeedometerColor(
	int speed,
	int maxSpeed)
{
	int colorMode = hud_speedometer_color.GetInt();

	return GetIntensityColor(
		speed,
		colorMode,
		255,
		BHOP_CAP_HARD * maxSpeed,
		BHOP_CAP_MID * maxSpeed,
		BHOP_CAP_SOFT * maxSpeed,
		maxSpeed,
		true);
}
