#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"
#include "c_ff_player.h"

#include "ff_hud_buildtimer.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

#include "c_playerresource.h"
extern C_PlayerResource *g_PR;

extern ConVar cl_teamcolourhud;

ConVar hud_buildtimers("hud_buildtimers", "1", FCVAR_ARCHIVE, "Turns visual build timers on or off");

using namespace vgui;

DECLARE_HUDELEMENT(CHudBuildTimer);
DECLARE_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);

CHudBuildTimer::CHudBuildTimer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildTimer") 
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
}

CHudBuildTimer::~CHudBuildTimer() 
{
}

void CHudBuildTimer::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);
	
	ivgui()->AddTickSignal( GetVPanel(), 100 );

	Reset();
}

void CHudBuildTimer::VidInit()
{
	m_pDispenserIconTexture = gHUD.GetIcon("build_dispenser");
	m_pSentrygunIconTexture = gHUD.GetIcon("build_sentrygun");
	m_pDetpackIconTexture = gHUD.GetIcon("build_detpack");
	m_pMancannonIconTexture = gHUD.GetIcon("build_jumppad");
	
	Reset();
}

void CHudBuildTimer::Reset()
{
	m_fVisible = false;
	m_iBuildType = FF_BUILD_NONE;	
	m_iPlayerTeam = -1;
	m_flStartTime = 0.0f;
	m_flDuration = 0.0f;
	
	SetAlpha( 0 );
	SetPaintEnabled(false);
	SetPaintBackgroundEnabled(false);
}

void CHudBuildTimer::SetBuildTimer(int iBuildType, float flDuration) 
{
	if(m_iBuildType != iBuildType)
	{
		m_iBuildType = iBuildType;
		switch(iBuildType)
		{
		case FF_BUILD_NONE: //cancel build timer
			Reset();
			return;
		case FF_BUILD_DISPENSER:
			m_pIconTexture = m_pDispenserIconTexture;
			break;
		case FF_BUILD_SENTRYGUN:
			m_pIconTexture = m_pSentrygunIconTexture;
			break;
		case FF_BUILD_DETPACK:
			m_pIconTexture = m_pDetpackIconTexture;
			break;
		case FF_BUILD_MANCANNON:
			m_pIconTexture = m_pMancannonIconTexture;
			break;
		}
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = flDuration;

	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInBuildTimer");
	}
}

void CHudBuildTimer::MsgFunc_FF_BuildTimer(bf_read &msg) 
{
	int type = msg.ReadShort();
	float duration = msg.ReadFloat();

	SetBuildTimer(type, duration);
}

void CHudBuildTimer::OnTick()
{
	BaseClass::OnTick();

	if (!hud_buildtimers.GetBool())
	{
		Reset();
		return;
	}

	if ( gpGlobals->curtime > m_flStartTime + m_flDuration ) 
	{
		float iFadeLength = g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("FadeOutBuildTimer");
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutBuildTimer");
		}
		// Fading time is over
		else if ( gpGlobals->curtime > m_flStartTime + m_flDuration + iFadeLength) 
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
		}
	}
	else
	{
		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);
	}
}

void CHudBuildTimer::PaintBackground() 
{
	int wide = GetWide() - bar_xpos;
	int tall = GetTall();

	if (m_pHudBackground)
	{
		if (cl_teamcolourhud.GetBool())
			m_pHudBackground->DrawSelf(bar_xpos, bar_ypos, wide, tall, m_TeamColorHudBackgroundColour);
		else
			m_pHudBackground->DrawSelf(bar_xpos, bar_ypos, wide, tall, m_HudBackgroundColour);
	}
	if (m_pHudForeground)
		m_pHudForeground->DrawSelf(bar_xpos, bar_ypos, wide, tall, m_HudForegroundColour);
}

void CHudBuildTimer::Paint() 
{
	if(m_pIconTexture)
	{		
		m_pIconTexture->DrawSelf( icon_xpos, icon_ypos, icon_width, icon_height, m_HudForegroundColour );
	}
	
	float amount = clamp((gpGlobals->curtime - m_flStartTime) / m_flDuration, 0, 1.0f);
	surface()->DrawSetColor(m_HudForegroundColour);
	surface()->DrawFilledRect(bar_xpos, bar_ypos, ( bar_xpos + ( ( GetWide() - bar_xpos ) * amount ) ), GetTall());
}