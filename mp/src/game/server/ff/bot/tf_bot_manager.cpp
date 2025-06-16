//========= Copyright Valve Corporation, All rights reserved. ============//
//----------------------------------------------------------------------------------------------------------------
// ff_bot_manager.cpp
// Team Fortress NextBotManager
// Tom Bui, February 2010
//----------------------------------------------------------------------------------------------------------------

#include "cbase.h"
#include "ff_bot_manager.h"

#include "Player/NextBotPlayer.h"
#include "team.h"
#include "ff_bot.h"
#include "ff_gamerules.h"
#include "bot/map_entities/ff_bot_hint.h"
#include "bot/map_entities/ff_bot_hint_sentrygun.h"
#include "bot/map_entities/ff_bot_hint_teleporter_exit.h"


//----------------------------------------------------------------------------------------------------------------

// Creates and sets CFFBotManager as the NextBotManager singleton
static CFFBotManager sTFBotManager;

extern ConVar ff_bot_force_class;
ConVar ff_bot_difficulty( "ff_bot_difficulty", "1", FCVAR_NONE, "Defines the skill of bots joining the game.  Values are: 0=easy, 1=normal, 2=hard, 3=expert." );
ConVar ff_bot_quota( "ff_bot_quota", "0", FCVAR_NONE, "Determines the total number of tf bots in the game." );
ConVar ff_bot_quota_mode( "ff_bot_quota_mode", "normal", FCVAR_NONE, "Determines the type of quota.\nAllowed values: 'normal', 'fill', and 'match'.\nIf 'fill', the server will adjust bots to keep N players in the game, where N is bot_quota.\nIf 'match', the server will maintain a 1:N ratio of humans to bots, where N is bot_quota." );
ConVar ff_bot_join_after_player( "ff_bot_join_after_player", "1", FCVAR_NONE, "If nonzero, bots wait until a player joins before entering the game." );
ConVar ff_bot_auto_vacate( "ff_bot_auto_vacate", "1", FCVAR_NONE, "If nonzero, bots will automatically leave to make room for human players." );
ConVar ff_bot_offline_practice( "ff_bot_offline_practice", "0", FCVAR_NONE, "Tells the server that it is in offline practice mode." );
ConVar ff_bot_melee_only( "ff_bot_melee_only", "0", FCVAR_GAMEDLL, "If nonzero, TFBots will only use melee weapons" );

extern const char *GetRandomBotName( void );
extern void CreateBotName( int iTeam, int iClassIndex, CFFBot::DifficultyType skill, char* pBuffer, int iBufferSize );

static bool UTIL_KickBotFromTeam( int kickTeam )
{
	int i;

	// try to kick a dead bot first
	for ( i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
		CFFBot* pBot = dynamic_cast<CFFBot*>(pPlayer);

		if (pBot == NULL)
			continue;

		if ( pBot->HasAttribute( CFFBot::QUOTA_MANANGED ) == false )
			continue;

		if ( ( pPlayer->GetFlags() & FL_FAKECLIENT ) == 0 )
			continue;

		if ( !pPlayer->IsAlive() && pPlayer->GetTeamNumber() == kickTeam )
		{
			// its a bot on the right team - kick it
			engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", pPlayer->GetUserID() ) );

			return true;
		}
	}

	// no dead bots, kick any bot on the given team
	for ( i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
		CFFBot* pBot = dynamic_cast<CFFBot*>(pPlayer);

		if (pBot == NULL)
			continue;

		if ( pBot->HasAttribute( CFFBot::QUOTA_MANANGED ) == false )
			continue;

		if ( ( pPlayer->GetFlags() & FL_FAKECLIENT ) == 0 )
			continue;

		if (pPlayer->GetTeamNumber() == kickTeam)
		{
			// its a bot on the right team - kick it
			engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", pPlayer->GetUserID() ) );

			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------

CFFBotManager::CFFBotManager()
	: NextBotManager()
	, m_flNextPeriodicThink( 0 )
{
	NextBotManager::SetInstance( this );
}


//----------------------------------------------------------------------------------------------------------------
CFFBotManager::~CFFBotManager()
{
	NextBotManager::SetInstance( NULL );
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::OnMapLoaded( void )
{
	NextBotManager::OnMapLoaded();

	ClearStuckBotData();
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::OnRoundRestart( void )
{
	NextBotManager::OnRoundRestart();

	// clear all hint ownership
	CFFBotHint *hint = NULL;
	while( ( hint = (CFFBotHint *)( gEntList.FindEntityByClassname( hint, "func_tfbot_hint" ) ) ) != NULL )
	{
		hint->SetOwnerEntity( NULL );
	}

	CFFBotHintSentrygun *sentryHint = NULL;
	while( ( sentryHint = (CFFBotHintSentrygun *)( gEntList.FindEntityByClassname( sentryHint, "bot_hint_sentrygun" ) ) ) != NULL )
	{
		sentryHint->SetOwnerEntity( NULL );
	}

	CFFBotHintTeleporterExit *teleporterHint = NULL;
	while( ( teleporterHint = (CFFBotHintTeleporterExit *)( gEntList.FindEntityByClassname( teleporterHint, "bot_hint_teleporter_exit" ) ) ) != NULL )
	{
		teleporterHint->SetOwnerEntity( NULL );
	}



	m_isMedeivalBossScenarioSetup = false;
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::Update()
{
	MaintainBotQuota();

	DrawStuckBotData();


	NextBotManager::Update();
}



//----------------------------------------------------------------------------------------------------------------
bool CFFBotManager::RemoveBotFromTeamAndKick( int nTeam )
{
	CUtlVector< CFFPlayer* > vecCandidates;

	// Gather potential candidates
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer == NULL )
			continue;

		if ( FNullEnt( pPlayer->edict() ) )
			continue;

		if ( !pPlayer->IsConnected() )
			continue;

		CFFBot* pBot = dynamic_cast<CFFBot*>( pPlayer );
		if ( pBot && pBot->HasAttribute( CFFBot::QUOTA_MANANGED ) )
		{
			if ( pBot->GetTeamNumber() == nTeam )
			{
				vecCandidates.AddToTail( pPlayer );
			}
		}
	}
	
	CFFPlayer *pVictim = NULL;
	if ( vecCandidates.Count() > 0 )
	{
		// first look for bots that are currently dead
		FOR_EACH_VEC( vecCandidates, i )
		{
			CFFPlayer *pPlayer = vecCandidates[i];
			if ( pPlayer && !pPlayer->IsAlive() )
			{
				pVictim = pPlayer;
				break;
			}
		}

		// if we didn't fine one, try to kick anyone on the team
		if ( !pVictim )
		{
			FOR_EACH_VEC( vecCandidates, i )
			{
				CFFPlayer *pPlayer = vecCandidates[i];
				if ( pPlayer )
				{
					pVictim = pPlayer;
					break;
				}
			}
		}
	}

	if ( pVictim )
	{
		if ( pVictim->IsAlive() )
		{
			pVictim->CommitSuicide();
		}
		pVictim->ForceChangeTeam( TEAM_UNASSIGNED ); // skipping TEAM_SPECTATOR because some servers don't allow spectators
		UTIL_KickBotFromTeam( TEAM_UNASSIGNED );
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::MaintainBotQuota()
{
	if ( TheNavMesh->IsGenerating() )
		return;

	if ( g_fGameOver )
		return;

	// new players can't spawn immediately after the round has been going for some time
	if ( !FFGameRules() )
		return;

	// training mode controls the bots
	if ( FFGameRules()->IsInTraining() )
		return;

	// if it is not time to do anything...
	if ( gpGlobals->curtime < m_flNextPeriodicThink )
		return;

	// think every quarter second
	m_flNextPeriodicThink = gpGlobals->curtime + 0.25f;

	// don't add bots until local player has been registered, to make sure he's player ID #1
	if ( !engine->IsDedicatedServer() )
	{
		CBasePlayer *pPlayer = UTIL_GetListenServerHost();
		if ( !pPlayer )
			return;
	}

	// We want to balance based on who's playing on game teams not necessary who's on team spectator, etc.
	int nConnectedClients = 0;
	int nTFBots = 0;
	int nTFBotsOnGameTeams = 0;
	int nNonTFBotsOnGameTeams = 0;
	int nSpectators = 0;
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer == NULL )
			continue;

		if ( FNullEnt( pPlayer->edict() ) )
			continue;

		if ( !pPlayer->IsConnected() )
			continue;

		CFFBot* pBot = dynamic_cast<CFFBot*>( pPlayer );
		if ( pBot && pBot->HasAttribute( CFFBot::QUOTA_MANANGED ) )
		{
			nTFBots++;
			if ( pPlayer->GetTeamNumber() == FF_TEAM_RED || pPlayer->GetTeamNumber() == FF_TEAM_BLUE )
			{
				nTFBotsOnGameTeams++;
			}
		}
		else
		{
			if ( pPlayer->GetTeamNumber() == FF_TEAM_RED || pPlayer->GetTeamNumber() == FF_TEAM_BLUE )
			{
				nNonTFBotsOnGameTeams++;
			}
			else if ( pPlayer->GetTeamNumber() == TEAM_SPECTATOR )
			{
				nSpectators++;
			}
		}

		nConnectedClients++;
	}

	int desiredBotCount = ff_bot_quota.GetInt();
	int nTotalNonTFBots = nConnectedClients - nTFBots;

	if ( FStrEq( ff_bot_quota_mode.GetString(), "fill" ) )
	{
		desiredBotCount = MAX( 0, desiredBotCount - nNonTFBotsOnGameTeams );
	}
	else if ( FStrEq( ff_bot_quota_mode.GetString(), "match" ) )
	{
		// If bot_quota_mode is 'match', we want the number of bots to be bot_quota * total humans
		desiredBotCount = (int)MAX( 0, ff_bot_quota.GetFloat() * nNonTFBotsOnGameTeams );
	}

	// wait for a player to join, if necessary
	if ( ff_bot_join_after_player.GetBool() )
	{
		if ( ( nNonTFBotsOnGameTeams == 0 ) && ( nSpectators == 0 ) )
		{
			desiredBotCount = 0;
		}
	}

	// if bots will auto-vacate, we need to keep one slot open to allow players to join
	if ( ff_bot_auto_vacate.GetBool() )
	{
		desiredBotCount = MIN( desiredBotCount, gpGlobals->maxClients - nTotalNonTFBots - 1 );
	}
	else
	{
		desiredBotCount = MIN( desiredBotCount, gpGlobals->maxClients - nTotalNonTFBots );
	}

	// add bots if necessary
	if ( desiredBotCount > nTFBotsOnGameTeams )
	{
		// don't try to add a bot if it would unbalance
		if ( !FFGameRules()->WouldChangeUnbalanceTeams( FF_TEAM_BLUE, TEAM_UNASSIGNED ) ||
			 !FFGameRules()->WouldChangeUnbalanceTeams( FF_TEAM_RED, TEAM_UNASSIGNED ) )
		{
			CFFBot *pBot = GetAvailableBotFromPool();
			if ( pBot == NULL )
			{
				pBot = NextBotCreatePlayerBot< CFFBot >( GetRandomBotName() );
			}
			if ( pBot )
			{
				pBot->SetAttribute( CFFBot::QUOTA_MANANGED );

				// join a team before we pick our class, since we use our teammates to decide what class to be
				pBot->HandleCommand_JoinTeam( "auto" );

				const char *classname = FStrEq( ff_bot_force_class.GetString(), "" ) ? pBot->GetNextSpawnClassname() : ff_bot_force_class.GetString();
				pBot->HandleCommand_JoinClass( classname );

				// give the bot a proper name
				char name[256];
				CFFBot::DifficultyType skill = pBot->GetDifficulty();
				CreateBotName( pBot->GetTeamNumber(), pBot->GetPlayerClass()->GetClassIndex(), skill, name, sizeof( name ) );
				engine->SetFakeClientConVarValue( pBot->edict(), "name", name );
				
				// Keep track of any bots we add during a match
				CMatchInfo *pMatchInfo = GTFGCClientSystem()->GetMatch();
				if ( pMatchInfo )
				{
					pMatchInfo->m_nBotsAdded++;
				}
			}
		}
	}
	else if ( desiredBotCount < nTFBotsOnGameTeams )
	{
		// kick a bot to maintain quota
		
		// first remove any unassigned bots
		if ( UTIL_KickBotFromTeam( TEAM_UNASSIGNED ) )
			return;

		int kickTeam;

		CTeam *pRed = GetGlobalTeam( FF_TEAM_RED );
		CTeam *pBlue = GetGlobalTeam( FF_TEAM_BLUE );

		// remove from the team that has more players
		if ( pBlue->GetNumPlayers() > pRed->GetNumPlayers() )
		{
			kickTeam = FF_TEAM_BLUE;
		}
		else if ( pBlue->GetNumPlayers() < pRed->GetNumPlayers() )
		{
			kickTeam = FF_TEAM_RED;
		}
		// remove from the team that's winning
		else if ( pBlue->GetScore() > pRed->GetScore() )
		{
			kickTeam = FF_TEAM_BLUE;
		}
		else if ( pBlue->GetScore() < pRed->GetScore() )
		{
			kickTeam = FF_TEAM_RED;
		}
		else
		{
			// teams and scores are equal, pick a team at random
			kickTeam = (RandomInt( 0, 1 ) == 0) ? FF_TEAM_BLUE : FF_TEAM_RED;
		}

		// attempt to kick a bot from the given team
		if ( UTIL_KickBotFromTeam( kickTeam ) )
			return;

		// if there were no bots on the team, kick a bot from the other team
		UTIL_KickBotFromTeam( kickTeam == FF_TEAM_BLUE ? FF_TEAM_RED : FF_TEAM_BLUE );
	}
}


//----------------------------------------------------------------------------------------------------------------
bool CFFBotManager::IsAllBotTeam( int iTeam )
{
	CTeam *pTeam = GetGlobalTeam( iTeam );
	if ( pTeam == NULL )
	{
		return false;
	}

	// check to see if any players on the team are humans
	for ( int i = 0, n = pTeam->GetNumPlayers(); i < n; ++i )
	{
		CFFPlayer *pPlayer = ToFFPlayer( pTeam->GetPlayer( i ) );
		if ( pPlayer == NULL )
		{
			continue;
		}
		if ( pPlayer->IsBot() == false )
		{
			return false;
		}
	}

	// if we made it this far, then they must all be bots!
	if ( pTeam->GetNumPlayers() != 0 )
	{
		return true;
	}

	// okay, this is a bit trickier...
	// if there are no people on this team, then we need to check the "assigned" human team
	return FFGameRules()->GetAssignedHumanTeam() != iTeam;
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::SetIsInOfflinePractice(bool bIsInOfflinePractice)
{
	ff_bot_offline_practice.SetValue( bIsInOfflinePractice ? 1 : 0 );
}


//----------------------------------------------------------------------------------------------------------------
bool CFFBotManager::IsInOfflinePractice() const
{
	return ff_bot_offline_practice.GetInt() != 0;
}


//----------------------------------------------------------------------------------------------------------------
bool CFFBotManager::IsMeleeOnly() const
{
	return ff_bot_melee_only.GetBool();
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::RevertOfflinePracticeConvars()
{
	ff_bot_quota.Revert();
	ff_bot_quota_mode.Revert();
	ff_bot_auto_vacate.Revert();
	ff_bot_difficulty.Revert();
	ff_bot_offline_practice.Revert();
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::LevelShutdown()
{
	m_flNextPeriodicThink = 0.0f;
	if ( IsInOfflinePractice() )
	{
		RevertOfflinePracticeConvars();
		SetIsInOfflinePractice( false );
	}		
}


//----------------------------------------------------------------------------------------------------------------
CFFBot* CFFBotManager::GetAvailableBotFromPool()
{
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
		CFFBot* pBot = dynamic_cast<CFFBot*>(pPlayer);

		if (pBot == NULL)
			continue;

		if ( ( pBot->GetFlags() & FL_FAKECLIENT ) == 0 )
			continue;

		if ( pBot->GetTeamNumber() == TEAM_SPECTATOR || pBot->GetTeamNumber() == TEAM_UNASSIGNED )
		{
			pBot->ClearAttribute( CFFBot::QUOTA_MANANGED );
			return pBot;
		}
	}
	return NULL;
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::OnForceAddedBots( int iNumAdded )
{
	ff_bot_quota.SetValue( ff_bot_quota.GetInt() + iNumAdded );
	m_flNextPeriodicThink = gpGlobals->curtime + 1.0f;
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::OnForceKickedBots( int iNumKicked )
{
	ff_bot_quota.SetValue( MAX( ff_bot_quota.GetInt() - iNumKicked, 0 ) );
	// allow time for the bots to be kicked
	m_flNextPeriodicThink = gpGlobals->curtime + 2.0f;
}


//----------------------------------------------------------------------------------------------------------------
CFFBotManager &TheTFBots( void )
{
	return static_cast<CFFBotManager&>( TheNextBots() );
}



//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
CON_COMMAND_F( ff_bot_debug_stuck_log, "Given a server logfile, visually display bot stuck locations.", FCVAR_GAMEDLL | FCVAR_CHEAT )
{
	// Listenserver host or rcon access only!
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if ( args.ArgC() < 2 )
	{
		DevMsg( "%s <logfilename>\n", args.Arg(0) );
		return;
	}

	FileHandle_t file = filesystem->Open( args.Arg(1), "r", "GAME" );

	const int maxBufferSize = 1024;
	char buffer[ maxBufferSize ];

	char logMapName[ maxBufferSize ];
	logMapName[0] = '\000';

	TheTFBots().ClearStuckBotData();

	if ( file )
	{
		int line = 0;
		while( !filesystem->EndOfFile( file ) )
		{
			filesystem->ReadLine( buffer, maxBufferSize, file );
			++line;

			strtok( buffer, ":" );
			strtok( NULL, ":" );
			strtok( NULL, ":" );
			char *first = strtok( NULL, " " );

			if ( !first )
				continue;

			if ( !strcmp( first, "Loading" ) )
			{
				
				strtok( NULL, " " );
				char *mapname = strtok( NULL, "\"" );

				if ( mapname )
				{
					strcpy( logMapName, mapname );
					Warning( "*** Log file from map '%s'\n", mapname );
				}
			}
			else if ( first[0] == '\"' )
			{
				// might be a player ID

				char *playerClassname = &first[1];

				char *nameEnd = playerClassname;
				while( *nameEnd != '\000' && *nameEnd != '<' )
					++nameEnd;
				*nameEnd = '\000';

				char *botIDString = ++nameEnd;
				char *IDEnd = botIDString;
				while( *IDEnd != '\000' && *IDEnd != '>' )
					++IDEnd;
				*IDEnd = '\000';

				int botID = atoi( botIDString );

				char *second = strtok( NULL, " " );
				if ( second && !strcmp( second, "stuck" ) )
				{
					CStuckBot *stuckBot = TheTFBots().FindOrCreateStuckBot( botID, playerClassname );

					CStuckBotEvent *stuckEvent = new CStuckBotEvent;


					// L 08/08/2012 - 15:15:05: "Scout<53><BOT><Blue>" stuck (position "-180.61 2471.29 216.04") (duration "2.52") L 08/08/2012 - 15:15:05:    path_goal ( "-180.61 2471.29 216.04" )
					strtok( NULL, " (\"" );	// (position

					stuckEvent->m_stuckSpot.x = (float)atof( strtok( NULL, " )\"" ) );
					stuckEvent->m_stuckSpot.y = (float)atof( strtok( NULL, " )\"" ) );
					stuckEvent->m_stuckSpot.z = (float)atof( strtok( NULL, " )\"" ) );

					strtok( NULL, ") (\"" );
					stuckEvent->m_stuckDuration = (float)atof( strtok( NULL, "\"" ) );

					strtok( NULL, ") (\"-L0123456789/:" );	// path_goal

					char *goal = strtok( NULL, ") (\"" );

					if ( goal && strcmp( goal, "NULL" ) )
					{
						stuckEvent->m_isGoalValid = true;

						stuckEvent->m_goalSpot.x = (float)atof( goal );
						stuckEvent->m_goalSpot.y = (float)atof( strtok( NULL, ") (\"" ) );
						stuckEvent->m_goalSpot.z = (float)atof( strtok( NULL, ") (\"" ) );
					}
					else
					{
						stuckEvent->m_isGoalValid = false;
					}

					stuckBot->m_stuckEventVector.AddToTail( stuckEvent );
				}
			}
		}

		filesystem->Close( file );
	}
	else
	{
		Warning( "Can't open file '%s'\n", args.Arg(1) );
	}

	//TheTFBots().DrawStuckBotData();
}


//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------
CON_COMMAND_F( ff_bot_debug_stuck_log_clear, "Clear currently loaded bot stuck data", FCVAR_GAMEDLL | FCVAR_CHEAT )
{
	// Listenserver host or rcon access only!
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	TheTFBots().ClearStuckBotData();
}


//----------------------------------------------------------------------------------------------------------------
// for parsing and debugging stuck bot server logs
void CFFBotManager::ClearStuckBotData()
{
	m_stuckBotVector.PurgeAndDeleteElements();
}


//----------------------------------------------------------------------------------------------------------------
// for parsing and debugging stuck bot server logs
CStuckBot *CFFBotManager::FindOrCreateStuckBot( int id, const char *playerClass )
{
	for( int i=0; i<m_stuckBotVector.Count(); ++i )
	{
		CStuckBot *stuckBot = m_stuckBotVector[i];

		if ( stuckBot->IsMatch( id, playerClass ) )
		{
			return stuckBot;
		}
	}

	// new instance of a stuck bot
	CStuckBot *newStuckBot = new CStuckBot( id, playerClass );
	m_stuckBotVector.AddToHead( newStuckBot );

	return newStuckBot;
}


//----------------------------------------------------------------------------------------------------------------
void CFFBotManager::DrawStuckBotData( float deltaT )
{
	if ( engine->IsDedicatedServer() )
		return;

	if ( !m_stuckDisplayTimer.IsElapsed() )
		return;

	m_stuckDisplayTimer.Start( deltaT );

	CBasePlayer *player = UTIL_GetListenServerHost();
	if ( player == NULL )
		return;

// 	Vector forward;
// 	AngleVectors( player->EyeAngles(), &forward );

	for( int i=0; i<m_stuckBotVector.Count(); ++i )
	{
		for( int j=0; j<m_stuckBotVector[i]->m_stuckEventVector.Count(); ++j )
		{
			m_stuckBotVector[i]->m_stuckEventVector[j]->Draw( deltaT );
		}

		for( int j=0; j<m_stuckBotVector[i]->m_stuckEventVector.Count()-1; ++j )
		{
			NDebugOverlay::HorzArrow( m_stuckBotVector[i]->m_stuckEventVector[j]->m_stuckSpot, 
									  m_stuckBotVector[i]->m_stuckEventVector[j+1]->m_stuckSpot,
									  3, 100, 0, 255, 255, true, deltaT );
		}

		NDebugOverlay::Text( m_stuckBotVector[i]->m_stuckEventVector[0]->m_stuckSpot, CFmtStr( "%s(#%d)", m_stuckBotVector[i]->m_name, m_stuckBotVector[i]->m_id ), false, deltaT );
	}
}


