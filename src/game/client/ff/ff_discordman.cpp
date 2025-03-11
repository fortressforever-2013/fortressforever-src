// ff_discordman.cpp
// rewrote entirely in SDK2013
#include "cbase.h"
#include "ff_discordman.h"

#include "igameresources.h"
#include "c_ff_team.h"
#include "ff_gamerules.h"

#include "valve_minmax_off.h"
#include <time.h>
#include <sstream>
#include <string>
#include "valve_minmax_on.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//#define DISCORD_LIBRARY_DLL "discord-rpc.dll"
//#define DISCORD_APP_ID "404135974801637378"
#define DISCORD_APP_ID "1203330706017619988"
//#define STEAM_ID "243750" // 2013-CHANGELATER

// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
//#define DISCORD_UPDATE_RATE 10.0f

ConVar use_discord("cl_discord", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_USERINFO,
	"Enable discord rich presence integration (current, server, map etc)");
static int64_t startTimestamp = time(0);

CFFDiscordManager _discord;

CFFDiscordManager::CFFDiscordManager()
{
}

CFFDiscordManager::~CFFDiscordManager()
{
}

void CFFDiscordManager::Init()
{
	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = OnReady;
	handlers.disconnected = OnDisconnect;
	handlers.errored = OnError;
	handlers.joinGame = OnDiscordJoin;
	handlers.spectateGame = OnDiscordSpectate;
	handlers.joinRequest = OnDiscordJoinRequest;

	char appid[255];
	V_sprintf_safe(appid, "%d", engine->GetAppID());
	Discord_Initialize(DISCORD_APP_ID, &handlers, 1, appid);

	Reset();
}

void CFFDiscordManager::OnReady(const DiscordUser* connectedUser)
{
	DevMsg("[DISCORD] Connected to user %s#%s - %s\n",
		connectedUser->username,
		connectedUser->discriminator,
		connectedUser->userId);
}

void CFFDiscordManager::OnDisconnect(int errcode, const char* message)
{
	DevMsg("[DISCORD] Disconnected (%d: %s)\n", errcode, message);
}

void CFFDiscordManager::OnError(int errcode, const char* message)
{
	DevMsg("[DISCORD] Error (%d: %s)\n", errcode, message);
}

void CFFDiscordManager::OnDiscordJoin(const char* secret)
{

}

void CFFDiscordManager::OnDiscordSpectate(const char* secret)
{

}

void CFFDiscordManager::OnDiscordJoinRequest(const DiscordUser* request)
{

}

void CFFDiscordManager::Shutdown()
{
	Discord_Shutdown();
}

void CFFDiscordManager::Update(DiscordRichPresence& discordPresence)
{
	if (!use_discord.GetInt())
		return;

	Discord_UpdatePresence(&discordPresence);
}

void CFFDiscordManager::Reset()
{
	memset(&m_szCurrentMap, 0, MAX_MAP_NAME);
	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.state = "Idle";
	discordPresence.details = "Main Menu";
	discordPresence.startTimestamp = startTimestamp;
	discordPresence.largeImageKey = "logo-big";

	Update(discordPresence);
}

void CFFDiscordManager::LevelPreInit(const char* mapname)
{
	Q_strncpy(m_szCurrentMap, mapname, MAX_MAP_NAME);

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	discordPresence.state = "Joining a server...";
	discordPresence.largeImageKey = "logo-big";
	discordPresence.startTimestamp = startTimestamp;

	Update(discordPresence);
}

void CFFDiscordManager::LevelInit(const char* mapname)
{
	Q_strncpy(m_szCurrentMap, mapname, MAX_MAP_NAME);

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	char buffer[256];
	discordPresence.state = "In-Game";
	V_sprintf_safe(buffer, "Map: %s", mapname);
	discordPresence.details = buffer;
	discordPresence.largeImageKey = "logo-big";
	discordPresence.startTimestamp = startTimestamp;

	Update(discordPresence);

	UpdateGameData();
}

void CFFDiscordManager::UpdateGameData()
{
	IGameResources* pGR = GameResources();

	if (!pGR)
		return;

	DiscordRichPresence discordPresence;
	memset(&discordPresence, 0, sizeof(discordPresence));

	int curPlayers = 0;
	int maxPlayers = gpGlobals->maxClients;

	char detailsStr[128];
	char stateStr[128];

	std::stringstream scoresStream;
	scoresStream << FFGameRules()->GetGameDescription() << " (";

	for (int i = TEAM_BLUE; i <= TEAM_GREEN; i++)
	{
		C_FFTeam* team = GetGlobalFFTeam(i);

		if (!team)
			continue;

		// If team is disabled, we probably don't want their score.
		if (team->Get_Teams() == -1)
			continue;

		scoresStream << team->Get_Score();

		// if the next team isn't disabled, add a separator
		if (GetGlobalFFTeam(i + 1) && GetGlobalFFTeam(i + 1)->Get_Teams() != -1)
			scoresStream << " | ";
	}
	scoresStream << ")";

	// we could also grab the player count from the above for loop
	// by summing all teams' player counts
	// but that wouldn't count spectators or unassigned so..
	for (int i = 1; i < maxPlayers; i++)
		if (pGR->IsConnected(i))
			curPlayers++;

	// step by step conversion to avoid a dangling pointer
	std::string scoresString = scoresStream.str();
	const char* actualTeamScores = scoresString.c_str();

	V_sprintf_safe(stateStr, "Map: %s (%i/%i)", m_szCurrentMap, curPlayers, maxPlayers);
	Q_strncpy(detailsStr, actualTeamScores, 128);
	detailsStr[127] = '\0';

	discordPresence.state = stateStr;
	discordPresence.details = detailsStr;
	discordPresence.largeImageKey = "logo-big";
	discordPresence.startTimestamp = startTimestamp;

	Update(discordPresence);
}

void CFFDiscordManager::LevelShutdown()
{
	Reset();
}
