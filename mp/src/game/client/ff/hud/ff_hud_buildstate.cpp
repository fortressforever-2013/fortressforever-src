/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   18:31
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_buildstate.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_buildstate
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill

	purpose:	Show information for buildables
*********************************************************************/

#include "cbase.h"
#include "ff_hud_buildstate.h"

CHudBuildState::CHudBuildState(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildState")
{
	SetParent(g_pClientMode->GetViewport());
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_UNASSIGNED );
}

CHudBuildState::~CHudBuildState()
{
}

void CHudBuildState::VidInit()
{
	SetPaintBackgroundEnabled(false);
}

void CHudBuildState::Init()
{
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);

}

void CHudBuildState::OnTick()
{
	if (!engine->IsInGame())
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return;

	m_bDrawCloak = false;


	if (pPlayer && pPlayer->GetClassSlot() == CLASS_SPY && !pPlayer->IsCloaked())
	{
		float flTime = pPlayer->GetNextCloak() - gpGlobals->curtime;
		if( flTime > 0.0f )
		{
			m_bDrawCloak = true;
			m_flCloakTimeLeft = flTime;
		}
		else
		{
			m_bDrawCloak = false;
			m_flCloakTimeLeft = 0.0f;
		}
	}
	else
	{
		m_bDrawCloak = false;
	}
}

void CHudBuildState::Paint()
{
	if (!m_bDrawCloak )
		return;

	// Draw icons
	surface()->DrawSetColor(255, 255, 255, 255);


	// Draw text
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());


	if (m_bDrawCloak)
	{
		surface()->DrawSetTextPos(text1_xpos, text1_ypos);

		V_snwprintf(m_wszCloak, 127, L"Time Left: %.2f seconds", m_flCloakTimeLeft);

		for (wchar_t* wch = m_wszCloak; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);
	}
}
