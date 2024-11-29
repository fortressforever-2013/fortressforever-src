#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"

#include "ff_hud_grenade1timer.h"
#include "ff_playerclass_parse.h"
#include "ff_grenade_parse.h"
#include "c_ff_player.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

#include "c_playerresource.h"
extern C_PlayerResource *g_PR;

extern ConVar cl_teamcolourhud;

extern ConVar hud_grenadetimers;

using namespace vgui;

CHudGrenade1Timer *g_pGrenade1Timer = NULL;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenade1Timer::CHudGrenade1Timer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudGrenade1Timer") 
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_UNASSIGNED );

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudGrenade1Timer::~CHudGrenade1Timer() 
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
void CHudGrenade1Timer::Init() 
{
	g_pGrenade1Timer = this;
	ResetTimer();
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::VidInit()
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(GREN1_TIMER_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(GREN1_TIMER_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set a timer
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::SetTimer(float duration) 
{
	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInGrenade1Timer");
	}
	m_Timers.AddToTail(timer_t(gpGlobals->curtime, duration));
	m_flLastTime = gpGlobals->curtime + duration;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all timers
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::ResetTimer( void )
{
	SetAlpha(0);
	m_Timers.RemoveAll();
	m_fVisible = false;
	m_flLastTime = -10.0f;
	m_iClass = 0;
	m_iPlayerTeam = -1;
}

//-----------------------------------------------------------------------------
// Purpose: See if any timers are active
//-----------------------------------------------------------------------------
bool CHudGrenade1Timer::ActiveTimer( void ) const
{
	return m_Timers.Count() > 0;
}

// dexter: we wanna know how many are going. we should have about that many sounds playing too
int CHudGrenade1Timer::ActiveTimerCount( void ) const
{
	return m_Timers.Count();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::MsgFunc_FF_Grenade1Timer(bf_read &msg) 
{
	float duration = msg.ReadFloat();

	SetTimer(duration);
}

bool CHudGrenade1Timer::ShouldDraw()
{
	if( !CHudElement::ShouldDraw() )
		return false;

	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer)
		return false;
	
	if (!hud_grenadetimers.GetBool())
		return false;

	int iClass = pPlayer->GetClassSlot();

	//if no class
	if(iClass == 0)
	{
		m_iClass = iClass;
		return false;
	}
	else if(m_iClass != iClass)
	{
		m_iClass = iClass;

		const char *szClassNames[] = { "scout", "sniper", "soldier", 
									 "demoman", "medic", "hwguy", 
									 "pyro", "spy", "engineer", 
									 "civilian" };

		PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
		bool bReadInfo = ReadPlayerClassDataFromFileForSlot( filesystem, szClassNames[m_iClass - 1], &hClassInfo, g_pGameRules->GetEncryptionKey());

		if (!bReadInfo)
			return false;

		const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

		if (!pClassInfo)
			return false;

		if ( strcmp( pClassInfo->m_szPrimaryClassName, "None" ) != 0 )
		{
			const char *grenade_name = pClassInfo->m_szPrimaryClassName;

			GRENADE_FILE_INFO_HANDLE hGrenInfo = LookupGrenadeInfoSlot(grenade_name);
			if (!hGrenInfo)
				return false;

			CFFGrenadeInfo *pGrenInfo = GetFileGrenadeInfoFromHandle(hGrenInfo);
			if (!pGrenInfo)
				return false;

			m_pIconTexture = pGrenInfo->iconHud;
		}
		else
		{
			return false;
		}
	}

	if ( gpGlobals->curtime > m_flLastTime ) 
	{
		float iFadeLength = g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("FadeOutGrenade1Timer");
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade1Timer");
		}
		// Fading time is over
		else if ( gpGlobals->curtime > m_flLastTime + iFadeLength) 
		{
			return false;
		}
	}

	return true;
}

void CHudGrenade1Timer::Paint() 
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = GetCustomClientColor( -1, pPlayer->GetTeamNumber() );
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, GREN1_TIMER_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( bar_xpos, bar_ypos, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, GREN1_TIMER_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( bar_xpos, bar_ypos, GetWide(), GetTall() );

	if(m_pIconTexture)
	{
		m_pIconTexture->DrawSelf( icon_xpos, icon_ypos, icon_color );
	}

	int num_timers = m_Timers.Count();
	int timer_to_remove = -1;
	float bar_newypos = bar_ypos;

	for (int i = m_Timers.Head(); i != m_Timers.InvalidIndex(); i = m_Timers.Next(i)) 
	{
		float timer_height = GetTall() / num_timers;

		bool bIsLastTimer = (m_Timers.Next(i) == m_Timers.InvalidIndex());
		timer_t *timer = &m_Timers.Element(i);

		if (gpGlobals->curtime > timer->m_flStartTime + timer->m_flDuration) 
		{
			timer_to_remove = i;
		}
		else
		{
			float amount = clamp((gpGlobals->curtime - timer->m_flStartTime) / timer->m_flDuration, 0, 1.0f);

			// Draw progress bar
			if (amount < 0.15f || ( bIsLastTimer && pPlayer && pPlayer->m_iGrenadeState == FF_GREN_PRIMEONE ))
			{
				surface()->DrawSetColor(GetFgColor());
			}
			else
			{
				Color clr = GetFgColor();
				clr.setA(clr.a() * 0.3f);

				surface()->DrawSetColor(clr);
			}

			surface()->DrawFilledRect(bar_xpos, bar_newypos, ( bar_xpos + ( ( GetWide() - bar_xpos ) * amount ) ), bar_newypos + timer_height);

			bar_newypos += timer_height;
		}
	}

	// Remove a timer this frame
	if (timer_to_remove > -1) 
		m_Timers.Remove(timer_to_remove);
}

DECLARE_HUDELEMENT(CHudGrenade1Timer);