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

DECLARE_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudBuildTimer::CHudBuildTimer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildTimer") 
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudBuildTimer::~CHudBuildTimer() 
{
	if (m_pBGTexture)
	{
		delete m_pBGTexture;
		m_pBGTexture = NULL;
	}

	if (m_pFGTexture)
	{
		delete m_pFGTexture;
		m_pFGTexture = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudBuildTimer::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);
	Reset();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudBuildTimer::VidInit()
{
	m_pDispenserIconTexture = gHUD.GetIcon("build_dispenser");
	m_pSentrygunIconTexture = gHUD.GetIcon("build_sentrygun");
	m_pDetpackIconTexture = gHUD.GetIcon("build_detpack");
	m_pMancannonIconTexture = gHUD.GetIcon("build_jumppad");
	
	Reset();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudBuildTimer::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(BUILD_TIMER_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(BUILD_TIMER_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudBuildTimer::Reset()
{
	m_fVisible = false;
	m_iBuildType = FF_BUILD_NONE;	
	m_iPlayerTeam = -1;
	m_flStartTime = 0.0f;
	m_flDuration = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Set timer
//-----------------------------------------------------------------------------
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

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = flDuration;

	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInBuildTimer");
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudBuildTimer::MsgFunc_FF_BuildTimer(bf_read &msg) 
{
	int type = msg.ReadShort();
	float duration = msg.ReadFloat();

	SetBuildTimer(type, duration);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHudBuildTimer::ShouldDraw()
{
	if( !CHudElement::ShouldDraw() )
		return false;

	if (!hud_buildtimers.GetBool())
	{
		Reset();
		return false;
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
			return false;
		}
	}

	return true;
}

void CHudBuildTimer::Paint() 
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	ConVarRef cl_teamcolourhud("cl_teamcolourhud");
	Color cColor = Color(255, 255, 255, 255);
	if( cl_teamcolourhud.GetBool() )
	{
		if ( g_PR )
			cColor = g_PR->GetTeamColor( pPlayer->GetTeamNumber() );
	}
	else
	{
		cColor = NON_TEAMCOLORED_HUD_COLOR;
	}
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, BUILD_TIMER_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( bar_xpos, bar_ypos, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, BUILD_TIMER_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( bar_xpos, bar_ypos, GetWide(), GetTall() );

	if(m_pIconTexture)
	{		
		m_pIconTexture->DrawSelf( icon_xpos, icon_ypos, icon_width, icon_height, Color(255, 255, 255, 255) );
	}
	
	float amount = clamp((gpGlobals->curtime - m_flStartTime) / m_flDuration, 0, 1.0f);
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawFilledRect(bar_xpos, bar_ypos, ( bar_xpos + ( ( GetWide() - bar_xpos ) * amount ) ), bar_ypos + GetTall());
}

DECLARE_HUDELEMENT(CHudBuildTimer);