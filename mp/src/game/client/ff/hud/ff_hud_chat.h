//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_HUD_CHAT_H
#define FF_HUD_CHAT_H
#ifdef _WIN32
#pragma once
#endif

#include <hud_basechat.h>

class CHudChatLine : public CBaseHudChatLine
{
	DECLARE_CLASS_SIMPLE(CHudChatLine, CBaseHudChatLine);

public:
	CHudChatLine(vgui::Panel* parent, const char* panelName) : CBaseHudChatLine(parent, panelName) {}

	virtual void	ApplySchemeSettings(vgui::IScheme* pScheme);

	void			MsgFunc_SayText(bf_read& msg);



private:
	CHudChatLine(const CHudChatLine&); // not defined, not accessible
};

//-----------------------------------------------------------------------------
// Purpose: The prompt and text entry area for chat messages
//-----------------------------------------------------------------------------
class CHudChatInputLine : public CBaseHudChatInputLine
{
	DECLARE_CLASS_SIMPLE(CHudChatInputLine, CBaseHudChatInputLine);

public:
	CHudChatInputLine(CBaseHudChat* parent, char const* panelName) : CBaseHudChatInputLine(parent, panelName) {}

	virtual void	ApplySchemeSettings(vgui::IScheme* pScheme);
};

class CHudChat : public CBaseHudChat
{
	DECLARE_CLASS_SIMPLE(CHudChat, CBaseHudChat);

public:
	CHudChat(const char* pElementName);
	~CHudChat(void);

	virtual void	CreateChatInputLine(void);
	virtual void	CreateChatLines(void);

	virtual void	Init(void);
	virtual void	Reset(void);
	virtual void	ApplySchemeSettings(vgui::IScheme* pScheme);

	int				GetChatInputOffset(void);

	virtual Color	GetClientColor(int clientIndex);

	void			ChatPrintf(int iPlayerIndex, const char* fmt, ...);
};

// Don't want to overwrite ClientPrint on the client but this does the same thing, basically
void ClientPrintMsg(C_BasePlayer* player, int msg_dest, const char* msg_name, const char* param1 = NULL, const char* param2 = NULL, const char* param3 = NULL, const char* param4 = NULL);

// customizable team colors
Color GetCustomClientColor(int iPlayerIndex, int iTeamIndex = -1);

#endif	//FF_HUD_CHAT_H