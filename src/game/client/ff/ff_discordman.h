// ff_discordman.h

#ifndef FF_DISCORDMAN_H
#define FF_DISCORDMAN_H

#include "discord_rpc.h"

class CFFDiscordManager
{
public:
	CFFDiscordManager();
	~CFFDiscordManager();

	void Init();
	void Shutdown();
	void Update(DiscordRichPresence& discordPresence);
	void Reset();

	void UpdateGameData();

	// Game events
	void LevelPreInit(const char* mapname);
	void LevelInit(const char* mapname);
	void LevelShutdown();

	// Discord Events
	static void OnReady(const DiscordUser* connectedUser);
	static void OnDisconnect(int errcode, const char* message);
	static void OnError(int errorCode, const char* szMessage);
	static void OnDiscordJoin(const char* secret);
	static void OnDiscordSpectate(const char* secret);
	static void OnDiscordJoinRequest(const DiscordUser* request);

	char m_szCurrentMap[MAX_MAP_NAME];
};

extern CFFDiscordManager _discord;

#endif // FF_DISCORDMAN_H
