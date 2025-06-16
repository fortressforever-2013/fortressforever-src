//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot.cpp
// Team Fortress NextBot
// Michael Booth, February 2009

#include "cbase.h"
#include "ff_player.h"
#include "ff_gamerules.h"
#include "ff_obj_sentrygun.h"
#include "team_control_point_master.h"
#include "ff_weapon_pipebomblauncher.h"
#include "ff_bot.h"
#include "ff_bot_manager.h"
#include "ff_bot_vision.h"
#include "ff_team.h"
#include "bot/map_entities/ff_bot_generator.h"
#include "trigger_area_capture.h"
#include "GameEventListener.h"
#include "NextBotUtil.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"
#include "bot/behavior/ff_bot_use_item.h"
#include "func_respawnroom.h"
#include "soundenvelope.h"

#include "player_vs_environment/tf_population_manager.h"

#include "bot/behavior/ff_bot_behavior.h"
#include "bot/map_entities/ff_bot_generator.h"
#include "bot/map_entities/ff_bot_hint_entity.h"

ConVar ff_bot_force_class( "ff_bot_force_class", "", FCVAR_GAMEDLL, "If set to a class name, all TFBots will respawn as that class" );

ConVar ff_bot_notice_gunfire_range( "ff_bot_notice_gunfire_range", "3000", FCVAR_GAMEDLL );
ConVar ff_bot_notice_quiet_gunfire_range( "ff_bot_notice_quiet_gunfire_range", "500", FCVAR_GAMEDLL );
ConVar ff_bot_sniper_personal_space_range( "ff_bot_sniper_personal_space_range", "1000", FCVAR_CHEAT, "Enemies beyond this range don't worry the Sniper" );
ConVar ff_bot_pyro_deflect_tolerance( "ff_bot_pyro_deflect_tolerance", "0.5", FCVAR_CHEAT );
ConVar ff_bot_keep_class_after_death( "ff_bot_keep_class_after_death", "0", FCVAR_GAMEDLL );
ConVar ff_bot_prefix_name_with_difficulty( "ff_bot_prefix_name_with_difficulty", "0", FCVAR_GAMEDLL, "Append the skill level of the bot to the bot's name" );
ConVar ff_bot_near_point_travel_distance( "ff_bot_near_point_travel_distance", "750", FCVAR_CHEAT, "If within this travel distance to the current point, bot is 'near' it" );
ConVar ff_bot_pyro_shove_away_range( "ff_bot_pyro_shove_away_range", "250", FCVAR_CHEAT, "If a Pyro bot's target is closer than this, compression blast them away" );
ConVar ff_bot_pyro_always_reflect( "ff_bot_pyro_always_reflect", "0", FCVAR_CHEAT, "Pyro bots will always reflect projectiles fired at them. For tesing/debugging purposes." );

ConVar ff_bot_sniper_spot_min_range( "ff_bot_sniper_spot_min_range", "1000", FCVAR_CHEAT );
ConVar ff_bot_sniper_spot_max_count( "ff_bot_sniper_spot_max_count", "10", FCVAR_CHEAT, "Stop searching for sniper spots when each side has found this many" );
ConVar ff_bot_sniper_spot_search_count( "ff_bot_sniper_spot_search_count", "10", FCVAR_CHEAT, "Search this many times per behavior update frame" );
ConVar ff_bot_sniper_spot_point_tolerance( "ff_bot_sniper_spot_point_tolerance", "750", FCVAR_CHEAT );
ConVar ff_bot_sniper_spot_epsilon( "ff_bot_sniper_spot_epsilon", "100", FCVAR_CHEAT );

ConVar ff_bot_sniper_goal_entity_move_tolerance( "ff_bot_sniper_goal_entity_move_tolerance", "500", FCVAR_CHEAT );

ConVar ff_bot_suspect_spy_touch_interval( "ff_bot_suspect_spy_touch_interval", "5", FCVAR_CHEAT, "How many seconds back to look for touches against suspicious spies" );
ConVar ff_bot_suspect_spy_forget_cooldown( "ff_bot_suspect_spy_forget_cooldown", "5", FCVAR_CHEAT, "How long to consider a suspicious spy as suspicious" );

ConVar ff_bot_debug_tags( "ff_bot_debug_tags", "0", FCVAR_CHEAT, "ent_text will only show tags on bots" );

extern ConVar ff_bot_sniper_spot_max_count;
extern ConVar ff_bot_fire_weapon_min_time;
extern ConVar ff_bot_sniper_misfire_chance;
extern ConVar ff_bot_difficulty;
extern ConVar ff_bot_farthest_visible_theater_sample_count;
extern ConVar ff_bot_sniper_spot_min_range;
extern ConVar ff_bot_sniper_spot_epsilon;
extern ConVar ff_bot_path_lookahead_range;



//-----------------------------------------------------------------------------------------------------
bool IsPlayerClassname( const char *string )
{
	for ( int i = CLASS_SCOUT; i < CLASS_COUNT_ALL; ++i )
	{
		if ( !stricmp( string, GetPlayerClassData( i )->m_szClassName ) )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
bool IsTeamName( const char *string )
{
	if ( !stricmp( string, "red" ) )
		return true;

	if ( !stricmp( string, "blue" ) )
		return true;

	return false;
}


//-----------------------------------------------------------------------------------------------------
CFFBot::DifficultyType StringToDifficultyLevel( const char *string )
{
	if ( !stricmp( string, "easy" ) )
		return CFFBot::EASY;

	if ( !stricmp( string, "normal" ) )
		return CFFBot::NORMAL;

	if ( !stricmp( string, "hard" ) )
		return CFFBot::HARD;

	if ( !stricmp( string, "expert" ) )
		return CFFBot::EXPERT;

	return CFFBot::UNDEFINED;
}


//-----------------------------------------------------------------------------------------------------
const char *DifficultyLevelToString( CFFBot::DifficultyType skill )
{
	switch( skill )
	{
	case CFFBot::EASY:		return "Easy ";
	case CFFBot::NORMAL:	return "Normal ";
	case CFFBot::HARD:		return "Hard ";
	case CFFBot::EXPERT:	return "Expert ";
	}

	return "Undefined ";
}


//-----------------------------------------------------------------------------------------------------
const char *GetRandomBotName( void )
{
	static const char *nameList[] =
	{
		"Chucklenuts",
		"CryBaby",
		"WITCH",
		"ThatGuy",
		"Still Alive",
		"Hat-Wearing MAN",
		"Me",
		"Numnutz",
		"H@XX0RZ",
		"The G-Man",
		"Chell",
		"The Combine",
		"Totally Not A Bot",
		"Pow!",
		"Zepheniah Mann",
		"THEM",
		"LOS LOS LOS",
		"10001011101",
		"DeadHead",
		"ZAWMBEEZ",
		"MindlessElectrons",
		"TAAAAANK!",
		"The Freeman",
		"Black Mesa",
		"Soulless",
		"CEDA",
		"BeepBeepBoop",
		"NotMe",
		"CreditToTeam",
		"BoomerBile",
		"Someone Else",
		"Mann Co.",
		"Dog",
		"Kaboom!",
		"AmNot",
		"0xDEADBEEF",
		"HI THERE",
		"SomeDude",
		"GLaDOS",
		"Hostage",
		"Headful of Eyeballs",
		"CrySomeMore",
		"Aperture Science Prototype XR7",
		"Humans Are Weak",
		"AimBot",
		"C++",
		"GutsAndGlory!",
		"Nobody",
		"Saxton Hale",
		"RageQuit",
		"Screamin' Eagles",

		"Ze Ubermensch",
		"Maggot",
		"CRITRAWKETS",
		"Herr Doktor",
		"Gentlemanne of Leisure",
		"Companion Cube",
		"Target Practice",
		"One-Man Cheeseburger Apocalypse",
		"Crowbar",
		"Delicious Cake",
		"IvanTheSpaceBiker",
		"I LIVE!",
		"Cannon Fodder",

		"trigger_hurt",
		"Nom Nom Nom",
		"Divide by Zero",
		"GENTLE MANNE of LEISURE",
		"MoreGun",
		"Tiny Baby Man",
		"Big Mean Muther Hubbard",
		"Force of Nature",

		"Crazed Gunman",
		"Grim Bloody Fable",
		"Poopy Joe",
		"A Professional With Standards",
		"Freakin' Unbelievable",
		"SMELLY UNFORTUNATE",
		"The Administrator",
		"Mentlegen",

		"Archimedes!",
		"Ribs Grow Back",
		"It's Filthy in There!",
		"Mega Baboon",
		"Kill Me",
		"Glorified Toaster with Legs",

#ifdef STAGING_ONLY
		"John Spartan",
		"Leeloo Dallas Multipass",
		"Sho'nuff",
		"Bruce Leroy",
		"CAN YOUUUUUUUUU DIG IT?!?!?!?!",
		"Big Gulp, Huh?",
		"Stupid Hot Dog",
		"I'm your huckleberry",
		"The Crocketeer",
#endif
		NULL
	};
	static int nameCount = 0;
	static int nameIndex = 0;

	if ( nameCount == 0 )
	{
		for( ; nameList[ nameCount ]; ++nameCount );

		// randomize the initial index
		nameIndex = RandomInt( 0, nameCount-1 );
	}

	const char *name = nameList[ nameIndex++ ];

	if ( nameIndex >= nameCount )
		nameIndex = 0;

	return name;
}


//-----------------------------------------------------------------------------------------------------
void CreateBotName( int iTeam, int iClassIndex, CFFBot::DifficultyType skill, char* pBuffer, int iBufferSize )
{
	char szBotNameBuffer[256];
	char szEnemyOrFriendlyString[256];

	const char *pBotName = "";
	const char *pFriendlyOrEnemyTitle = "";

	// @note (Tom Bui): it is okay to get localized name in training, since we should be on a listen server
	if ( FFGameRules()->IsInTraining() )
	{
		// get the friendly/enemy title
		const char *pBotTitle = NULL;
		if ( iTeam != TEAM_UNASSIGNED )
		{
			int iHumanTeam = FFGameRules()->GetAssignedHumanTeam();
			if ( iHumanTeam != TEAM_ANY )
			{
				if ( iHumanTeam == iTeam )
				{
					pBotTitle = "#TF_Bot_Title_Friendly";
				}
				else
				{
					pBotTitle = "#TF_Bot_Title_Enemy";
				}
			}
		}
		wchar_t *pLocalizedTitle = pBotTitle ? g_pVGuiLocalize->Find( pBotTitle ) : NULL;
		if ( pLocalizedTitle )
		{
			g_pVGuiLocalize->ConvertUnicodeToANSI( pLocalizedTitle, szEnemyOrFriendlyString, sizeof( szEnemyOrFriendlyString ) );
			pFriendlyOrEnemyTitle = szEnemyOrFriendlyString;
		}

		// get the class name
		wchar_t *pLocalizedName = NULL;
		if ( iClassIndex >= CLASS_SCOUT && iClassIndex < CLASS_CIVILIAN )
		{
			pLocalizedName = g_pVGuiLocalize->Find( g_aPlayerClassNames[ iClassIndex ] );
		}
		else
		{
			pLocalizedName = g_pVGuiLocalize->Find( "#TF_Bot_Generic_ClassName" );
		}
		g_pVGuiLocalize->ConvertUnicodeToANSI( pLocalizedName, szBotNameBuffer, sizeof( szBotNameBuffer ) );
		pBotName = szBotNameBuffer;
	}
	else
	{
		pBotName = GetRandomBotName();
	}
	
	const char *pDifficultyString = ff_bot_prefix_name_with_difficulty.GetBool() ? DifficultyLevelToString( skill ) : "";

	// we use this as our formatting, because we don't know the language of the downstream clients
	CFmtStr name( "%s%s%s", 
				  pDifficultyString, pFriendlyOrEnemyTitle, pBotName );
	Q_strncpy( pBuffer, name.Access(), iBufferSize );
}


//-----------------------------------------------------------------------------------------------------
CON_COMMAND_F( ff_bot_add, "Add a bot.", FCVAR_GAMEDLL )
{
	// Listenserver host or rcon access only!
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	bool bQuotaManaged = true;
	int botCount = 1;
	const char *classname = NULL;
	const char *teamname = "auto";
	const char *pszBotNameViaArg = NULL;
	CFFBot::DifficultyType skill = clamp( (CFFBot::DifficultyType)ff_bot_difficulty.GetInt(), CFFBot::EASY, CFFBot::EXPERT );

	int i;
	for( i=1; i<args.ArgC(); ++i )
	{
		CFFBot::DifficultyType trySkill = StringToDifficultyLevel( args.Arg(i) );
		int nArgAsInteger = atoi( args.Arg(i) );

		// each argument could be a classname, a team, a difficulty level, a count, or a name
		if ( IsPlayerClassname( args.Arg(i) ) )
		{
			classname = args.Arg(i);
		}
		else if ( IsTeamName( args.Arg(i) ) )
		{
			teamname = args.Arg(i);
		}
		else if ( !stricmp( args.Arg( i ), "noquota" ) )
		{
			bQuotaManaged = false;
		}
		else if ( trySkill != CFFBot::UNDEFINED )
		{
			skill = trySkill;
		}
		else if ( nArgAsInteger > 0 )
		{
			botCount = nArgAsInteger;
			pszBotNameViaArg = NULL; // can't have a custom name if spawning multiple bots
		}
		else if ( botCount == 1 )
		{
			pszBotNameViaArg = args.Arg( i );
		}
		else
		{
			Warning( "Invalid argument '%s'\n", args.Arg(i) );
		}
	}

	// cvar can override classname
	classname = FStrEq( ff_bot_force_class.GetString(), "" ) ? classname : ff_bot_force_class.GetString();
	int iClassIndex = classname ? GetClassIndexFromString( classname ) : CLASS_UNDEFINED;

	int iTeam = TEAM_UNASSIGNED;
	if ( FStrEq( teamname, "red" ) )
	{
		iTeam = FF_TEAM_RED;
	}
	else if ( FStrEq( teamname, "blue" ) )
	{
		iTeam = FF_TEAM_BLUE;
	}

	if ( FFGameRules()->IsInTraining() )
	{
		skill = CFFBot::EASY;
	}
	
	char name[256];
	int iNumAdded = 0;
	for( i=0; i<botCount; ++i )
	{
		CFFBot *pBot = NULL;
		const char *pszBotName = NULL;

		if ( !pszBotNameViaArg )
		{
			CreateBotName( iTeam, iClassIndex, skill, name, sizeof(name) );
			pszBotName = name;
		}
		else
		{
			pszBotName = pszBotNameViaArg;
		}

		pBot = NextBotCreatePlayerBot< CFFBot >( pszBotName );

		if ( pBot ) 
		{
			if ( bQuotaManaged )
			{
				pBot->SetAttribute( CFFBot::QUOTA_MANANGED );
			}

			pBot->HandleCommand_JoinTeam( teamname );

			pBot->SetDifficulty( skill );

			// if no class is set, auto-select one
			const char *thisClassname = classname ? classname : pBot->GetNextSpawnClassname();
			pBot->HandleCommand_JoinClass( thisClassname );

			// set up a proper name now that we are in training
			if ( FFGameRules()->IsInTraining() )
			{
				CreateBotName( pBot->GetTeamNumber(), pBot->GetPlayerClass()->GetClassIndex(), skill, name, sizeof(name) );
				engine->SetFakeClientConVarValue( pBot->edict(), "name", name );
			}

			++iNumAdded;
		}
	}

	if ( bQuotaManaged )
	{
		TheTFBots().OnForceAddedBots( iNumAdded );
	}
}


//-----------------------------------------------------------------------------------------------------
CON_COMMAND_F( ff_bot_kick, "Remove a TFBot by name, or all bots (\"all\").", FCVAR_GAMEDLL )
{
	// Listenserver host or rcon access only!
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if ( args.ArgC() < 2 )
	{
		DevMsg( "%s <bot name>, \"red\", \"blue\", or \"all\"> <optional: \"moveToSpectatorTeam\"> \n", args.Arg(0) );
		return;
	}

	bool bMoveToSpectatorTeam = false;
	int iTeam = TEAM_UNASSIGNED;
	int i;
	const char *pPlayerName = "";
	for( i=1; i<args.ArgC(); ++i )
	{
		// each argument could be a classname, a team, or a count
		if ( FStrEq( args.Arg(i), "red" ) )
		{
			iTeam = FF_TEAM_RED;
		}
		else if ( FStrEq( args.Arg(i), "blue" ) )
		{
			iTeam = FF_TEAM_BLUE;
		}
		else if ( FStrEq( args.Arg(i), "all" ) )
		{
			iTeam = TEAM_ANY;
		}
		else if ( FStrEq( args.Arg(i), "moveToSpectatorTeam" ) )
		{
			bMoveToSpectatorTeam = true;
		}
		else 
		{
			pPlayerName = args.Arg(i);
		}
	}

	int iNumKicked = 0;
	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if ( !player )
			continue;

		if ( FNullEnt( player->edict() ) )
			continue;

		if ( player->MyNextBotPointer() )
		{
			if ( iTeam == TEAM_ANY ||
				 FStrEq( pPlayerName, player->GetPlayerName() ) ||
				 ( player->GetTeamNumber() == iTeam ) ||
				 ( player->GetTeamNumber() == iTeam ) )
			{
				if ( bMoveToSpectatorTeam )
				{
					player->ChangeTeam( TEAM_SPECTATOR, false, true );
				}
				else
				{
					engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", player->GetUserID() ) );
				}
				CFFBot* pBot = dynamic_cast< CFFBot* >( player );
				if ( pBot && pBot->HasAttribute( CFFBot::QUOTA_MANANGED ) )
				{
					++iNumKicked;
				}				
			}
		}
	}
	TheTFBots().OnForceKickedBots( iNumKicked );
}


//-----------------------------------------------------------------------------------------------------
CON_COMMAND_F( ff_bot_kill, "Kill a TFBot by name, or all bots (\"all\").", FCVAR_GAMEDLL )
{
	// Listenserver host or rcon access only!
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if ( args.ArgC() < 2 )
	{
		DevMsg( "%s <bot name>, \"red\", \"blue\", or \"all\"> <optional: \"moveToSpectatorTeam\"> \n", args.Arg(0) );
		return;
	}

	int iTeam = TEAM_UNASSIGNED;
	int i;
	const char *pPlayerName = "";
	for( i=1; i<args.ArgC(); ++i )
	{
		// each argument could be a classname, a team, or a count
		if ( FStrEq( args.Arg(i), "red" ) )
		{
			iTeam = FF_TEAM_RED;
		}
		else if ( FStrEq( args.Arg(i), "blue" ) )
		{
			iTeam = FF_TEAM_BLUE;
		}
		else if ( FStrEq( args.Arg(i), "all" ) )
		{
			iTeam = TEAM_ANY;
		}
		else if ( FStrEq( args.Arg(i), "moveToSpectatorTeam" ) )
		{
			// bMoveToSpectatorTeam = true;
		}
		else 
		{
			pPlayerName = args.Arg(i);
		}
	}

	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if ( !player )
			continue;

		if ( FNullEnt( player->edict() ) )
			continue;

		if ( player->MyNextBotPointer() )
		{
			if ( iTeam == TEAM_ANY ||
				FStrEq( pPlayerName, player->GetPlayerName() ) ||
				( player->GetTeamNumber() == iTeam ) ||
				( player->GetTeamNumber() == iTeam ) )
			{
				CTakeDamageInfo info( player, player, 9999999.9f, DMG_ENERGYBEAM, TF_DMG_CUSTOM_NONE );
				player->TakeDamage( info );
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------
void CMD_BotWarpTeamToMe( void )
{
	CBasePlayer *player = UTIL_GetListenServerHost();
	if ( !player )
		return;

	CTeam *myTeam = player->GetTeam();
	for( int i=0; i<myTeam->GetNumPlayers(); ++i )
	{
		if ( !myTeam->GetPlayer(i)->IsAlive() )
			continue;

		myTeam->GetPlayer(i)->SetAbsOrigin( player->GetAbsOrigin() );
	}
}
static ConCommand ff_bot_warp_team_to_me( "ff_bot_warp_team_to_me", CMD_BotWarpTeamToMe, "", FCVAR_GAMEDLL | FCVAR_CHEAT );


//-----------------------------------------------------------------------------------------------------
IMPLEMENT_INTENTION_INTERFACE( CFFBot, CFFBotMainAction );


//-----------------------------------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( ff_bot, CFFBot );


//-----------------------------------------------------------------------------------------------------
/**
 * Allocate a bot and bind it to the edict
 */
CBasePlayer *CFFBot::AllocatePlayerEntity( edict_t *edict, const char *playerName )
{
	CBasePlayer::s_PlayerEdict = edict;
	return static_cast< CBasePlayer * >( CreateEntityByName( "ff_bot" ) );
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::PressFireButton( float duration )
{
	// can't fire if stunned
	// @todo Tom Bui: Eventually, we'll probably want to check the actual weapon for supress fire
	if ( m_Shared.IsControlStunned() || m_Shared.IsLoserStateStunned() || HasAttribute( CFFBot::SUPPRESS_FIRE ) )
	{
		ReleaseFireButton();
		return;
	}

	BaseClass::PressFireButton( duration );
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::PressAltFireButton( float duration )
{
	// can't fire if stunned
	// @todo Tom Bui: Eventually, we'll probably want to check the actual weapon for supress fire
	if ( m_Shared.IsControlStunned() || m_Shared.IsLoserStateStunned() || HasAttribute( CFFBot::SUPPRESS_FIRE ) )
	{
		ReleaseAltFireButton();
		return;
	}

	BaseClass::PressAltFireButton( duration );
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::PressSpecialFireButton( float duration )
{
	// can't fire if stunned
	// @todo Tom Bui: Eventually, we'll probably want to check the actual weapon for supress fire
	if ( m_Shared.IsControlStunned() || m_Shared.IsLoserStateStunned() || HasAttribute( CFFBot::SUPPRESS_FIRE ) )
	{
		ReleaseAltFireButton();
		return;
	}

	BaseClass::PressSpecialFireButton( duration );
}


//-----------------------------------------------------------------------------------------------------
class CCountClassMembers
{
public:
	CCountClassMembers( const CFFBot *me, int teamID )
	{
		m_me = me;
		m_myTeam = teamID;
		m_teamSize = 0;
		
		for( int i=0; i<CLASS_CIVILIAN; ++i )
			m_count[i] = 0;
	}

	bool operator() ( CBasePlayer *basePlayer )
	{
		CFFPlayer *player = (CFFPlayer *)basePlayer;

		if ( player->GetTeamNumber() != m_myTeam )
			return true;

		++m_teamSize;

		if ( m_me->IsSelf( player ) )
			return true;

		++m_count[ player->GetDesiredPlayerClassIndex() ];

		return true;
	}

	const CFFBot *m_me;
	int m_myTeam;
	int m_count[ CLASS_CIVILIAN+1 ];
	int m_teamSize;
};


//-----------------------------------------------------------------------------------------------------
/**
 * NOTE: Assumes bot's difficulty has been set, and the bot is on a team.
 */
const char *CFFBot::GetNextSpawnClassname( void ) const
{
	struct ClassSelectionInfo
	{
		int m_class;
		int m_minTeamSizeToSelect;					// team must have this many members to choose this class
		int m_countPerTeamSize;						// must have 1 Medic for each 4 team members, for example
		int m_minLimit;								// minimum that must be present (once other constraints are met)
		int m_maxLimit[ NUM_DIFFICULTY_LEVELS ];	// maximum that can be present (-1 for infinite)
	};

	const int NoLimit = -1;

	static ClassSelectionInfo defenseRoster[] = 
	{
		{ CLASS_ENGINEER,		0, 4, 1, { 1, 2, 3, 3 } },
		{ CLASS_SOLDIER,			0, 0, 0, { NoLimit, NoLimit, NoLimit, NoLimit } },
		{ CLASS_DEMOMAN,			0, 0, 0, { 2, 3, 3, 3 } },
		{ CLASS_PYRO,			3, 0, 0, { NoLimit, NoLimit, NoLimit, NoLimit } },
		{ CLASS_HEAVYWEAPONS,	3, 0, 0, { 1, 1, 2, 2 } },
		{ CLASS_MEDIC,			4, 4, 1, { 1, 1, 2, 2 } },
		{ CLASS_SNIPER,			5, 0, 0, { 0, 1, 1, 1 } },
		{ CLASS_SPY,				5, 0, 0, { 0, 1, 2, 2 } },

		{ CLASS_UNDEFINED,		0, -1 },
	};

	static ClassSelectionInfo offenseRoster[] = 
	{
		{ CLASS_SCOUT,			0, 0, 1, { 3, 3, 3, 3 } },
		{ CLASS_SOLDIER,			0, 0, 0, { NoLimit, NoLimit, NoLimit, NoLimit } },
		{ CLASS_DEMOMAN,			0, 0, 0, { 2, 3, 3, 3 } },							// must limit demomen, or the whole team will go demo to take out tough sentryguns
		{ CLASS_PYRO,			3, 0, 0, { NoLimit, NoLimit, NoLimit, NoLimit } },
		{ CLASS_HEAVYWEAPONS,	3, 0, 0, { 1, 1, 2, 2 } },
		{ CLASS_MEDIC,			4, 4, 1, { 1, 1, 2, 2 } },
		{ CLASS_SNIPER,			5, 0, 0, { 0, 1, 1, 1 } },
		{ CLASS_SPY,				5, 0, 0, { 0, 1, 2, 2 } },
		{ CLASS_ENGINEER,		5, 0, 0, { 1, 1, 1, 1 } },

		{ CLASS_UNDEFINED,		0, -1 },
	};

	static ClassSelectionInfo compRoster[] =
	{
		{ CLASS_SCOUT,			0, 0, 0, { 0, 0, 2, 2 } },
		{ CLASS_SOLDIER,			0, 0, 0, { 0, 0, NoLimit, NoLimit } },
		{ CLASS_DEMOMAN,			0, 0, 0, { 0, 0, 2, 2 } },							// must limit demomen, or the whole team will go demo to take out tough sentryguns
		{ CLASS_PYRO,			0, -1 },
		{ CLASS_HEAVYWEAPONS,	3, 0, 0, { 0, 0, 2, 2 } },
		{ CLASS_MEDIC,			1, 0, 1, { 0, 0, 1, 1 } },
		{ CLASS_SNIPER,			0, -1 },
		{ CLASS_SPY,				0, -1 },
		{ CLASS_ENGINEER,		0, -1 },

		{ CLASS_UNDEFINED,		0, -1 },
	};

	// if we are an engineer with an active sentry or teleporters, don't switch
	if ( IsPlayerClass( CLASS_ENGINEER ) )
	{
		if ( const_cast< CFFBot * >( this )->GetObjectOfType( OBJ_SENTRYGUN ) ||
			 const_cast< CFFBot * >( this )->GetObjectOfType( OBJ_TELEPORTER, MODE_TELEPORTER_EXIT ) )
		{
			return "engineer";
		}
	}

	// count classes in use by my team, not including me
	CCountClassMembers currentRoster( this, GetTeamNumber() );
	ForEachPlayer( currentRoster );

	// assume offense
	ClassSelectionInfo *desiredRoster = offenseRoster;
	
	if ( FFGameRules()->IsMatchTypeCompetitive() )
	{
		desiredRoster = compRoster;
	}
	else if ( FFGameRules()->IsInKothMode() )
	{
		CTeamControlPoint *point = GetMyControlPoint();
		if ( point )
		{
			if ( GetTeamNumber() == ObjectiveResource()->GetOwningTeam( point->GetPointIndex() ) )
			{
				// defend our point
				desiredRoster = defenseRoster;
			}
		}
	}
	else if ( FFGameRules()->GetGameType() == TF_GAMETYPE_CP )
	{
		CUtlVector< CTeamControlPoint * > captureVector;
		FFGameRules()->CollectCapturePoints( const_cast< CFFBot * >( this ), &captureVector );

		CUtlVector< CTeamControlPoint * > defendVector;
		FFGameRules()->CollectDefendPoints( const_cast< CFFBot * >( this ), &defendVector );

		// if we have any points we can capture, try to do so
		if ( captureVector.Count() > 0 || defendVector.Count() == 0 )
		{
			desiredRoster = offenseRoster;
		}
		else
		{
			desiredRoster = defenseRoster;
		}
	}
	else if ( FFGameRules()->GetGameType() == TF_GAMETYPE_ESCORT )
	{
		if ( GetTeamNumber() == FF_TEAM_RED )
		{
			desiredRoster = defenseRoster;
		}
	}

	// build vector of classes we can pick from
	CUtlVector< int > desiredClassVector;
	CUtlVector< int > allowedClassForBotRosterVector;

	for( int i=0; desiredRoster[ i ].m_class != CLASS_UNDEFINED; ++i )
	{
		ClassSelectionInfo *desiredClassInfo = &desiredRoster[ i ];

		if ( FFGameRules()->CanBotChooseClass( const_cast< CFFBot * >( this ), desiredClassInfo->m_class ) == false )
		{
			// not allowed to use this class
			continue;
		}
		// just in case we hit the class limits, we want to make sure we select a class that is allowed
		allowedClassForBotRosterVector.AddToTail( desiredClassInfo->m_class );

		if ( currentRoster.m_teamSize < desiredClassInfo->m_minTeamSizeToSelect )
		{
			// team is too small to choose this class
			continue;
		}

		// check limits
		if ( currentRoster.m_count[ desiredClassInfo->m_class ] < desiredClassInfo->m_minLimit )
		{
			// below required limit - choose only this class
			desiredClassVector.RemoveAll();
			desiredClassVector.AddToTail( desiredClassInfo->m_class );
			break;
		}

		int maxLimit = desiredClassInfo->m_maxLimit[ (int)clamp( GetDifficulty(), CFFBot::EASY, CFFBot::EXPERT ) ];

		if ( maxLimit > NoLimit && currentRoster.m_count[ desiredClassInfo->m_class ] >= maxLimit )
		{
			// at or above limit for this class
			continue;
		}

		if ( desiredClassInfo->m_countPerTeamSize > 0 )
		{
			// how many of this class should there be at the given "per" count
			int maxCountPer = currentRoster.m_teamSize / desiredClassInfo->m_countPerTeamSize;
			if ( currentRoster.m_count[ desiredClassInfo->m_class ] - desiredClassInfo->m_minTeamSizeToSelect < maxCountPer )
			{
				// below required limit - choose only this class
				desiredClassVector.RemoveAll();
				desiredClassVector.AddToTail( desiredClassInfo->m_class );
				break;
			}
		}

		// valid class to choose
		desiredClassVector.AddToTail( desiredClassInfo->m_class );
	}

	if ( desiredClassVector.Count() == 0 )
	{
		if ( allowedClassForBotRosterVector.Count() == 0 )
		{
			// nothing available
			Warning( "TFBot unable to choose a class, defaulting to 'auto'\n" );
			return "auto";
		}
		else
		{
			desiredClassVector = allowedClassForBotRosterVector;
		}
	}

	int which = RandomInt( 0, desiredClassVector.Count()-1 );

	// if we need to destroy a sentry, pick a class that can do so
	if ( GetEnemySentry() ) 
	{
		// best sentry demolitions
		int demoman = desiredClassVector.Find( CLASS_DEMOMAN );
		if ( demoman >= 0 )
		{
			which = demoman;
		}
		else
		{
			// next best sentry demolitions
			int spy = desiredClassVector.Find( CLASS_SPY );
			if ( spy >= 0 )
			{
				which = spy;
			}
			else
			{
				// good sentry demolitions
				int soldier = desiredClassVector.Find( CLASS_SOLDIER );
				if ( soldier >= 0 )
				{
					which = soldier;
				}
			}
		}
	}

	TFPlayerClassData_t *classData = GetPlayerClassData( desiredClassVector[ which ] );
	if ( classData )
	{
		return classData->m_szClassName;
	}

	Warning( "TFBot unable to get data for desired class, defaulting to 'auto'\n" );
	return "auto";
}


//-----------------------------------------------------------------------------------------------------
CFFBot::CFFBot()
{
	m_body = new CFFBotBody( this );
	m_locomotor = new CFFBotLocomotion( this );
	m_vision = new CFFBotVision( this );
	ALLOCATE_INTENTION_INTERFACE( CFFBot );

	m_spawnArea = NULL;
	m_weaponRestrictionFlags = 0;
	m_attributeFlags = 0;
	m_homeArea = NULL;
	m_squad = NULL;
	m_didReselectClass = false;
	m_enemySentry = NULL;
	m_spotWhereEnemySentryLastInjuredMe = vec3_origin;
	m_isLookingAroundForEnemies = true;
	m_behaviorFlags = 0;
	m_attentionFocusEntity = NULL;
	m_noisyTimer.Invalidate();

	if ( FFGameRules()->IsInTraining() )
	{
		m_difficulty = CFFBot::EASY;
	}
	else
	{
		m_difficulty = clamp( (CFFBot::DifficultyType)ff_bot_difficulty.GetInt(), CFFBot::EASY, CFFBot::EXPERT );
	}

	m_actionPoint = NULL;
	m_proxy = NULL;
	m_spawner = NULL;

	m_myControlPoint = NULL;

	SetMission( NO_MISSION, MISSION_DOESNT_RESET_BEHAVIOR_SYSTEM );
	SetMissionTarget( NULL );
	m_missionString.Clear();

	m_fModelScaleOverride = -1.0f;
	m_maxVisionRangeOverride = -1.0f;
	m_squadFormationError = 0.0f;

	m_hFollowingFlagTarget = NULL;

	SetShouldQuickBuild( false );
	SetAutoJump( 0.f, 0.f );

	ClearSniperSpots();

	ListenForGameEvent( "teamplay_point_startcapture" );
	ListenForGameEvent( "teamplay_point_captured" );
	ListenForGameEvent( "teamplay_round_win" );
	ListenForGameEvent( "teamplay_flag_event" );
}


//-----------------------------------------------------------------------------------------------------
CFFBot::~CFFBot()
{
	// delete Intention first, since destruction of Actions may access other components
	DEALLOCATE_INTENTION_INTERFACE;

	if ( m_body )
		delete m_body;

	if ( m_locomotor )
		delete m_locomotor;

	if ( m_vision )
		delete m_vision;

	m_suspectedSpyVector.PurgeAndDeleteElements();
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::Spawn()
{
	BaseClass::Spawn();

	m_spawnArea = NULL;
	m_justLostPointTimer.Invalidate();
	m_squad = NULL;
	m_didReselectClass = false;
	m_isLookingAroundForEnemies = true;
	m_attentionFocusEntity = NULL;

	m_suspectedSpyVector.PurgeAndDeleteElements();
	m_knownSpyVector.RemoveAll();
	m_delayedNoticeVector.RemoveAll();

	m_myControlPoint = NULL;
	ClearSniperSpots();
	ClearTags();

	m_hFollowingFlagTarget = NULL;

	m_requiredWeaponStack.Clear();
	SetShouldQuickBuild( false );

	SetSquadFormationError( 0.0f );
	SetBrokenFormation( false );

	GetVisionInterface()->ForgetAllKnownEntities();
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::SetMission( MissionType mission, bool resetBehaviorSystem )
{
	SetPrevMission( m_mission );
	m_mission = mission;

	if ( resetBehaviorSystem )
	{
		// reset the behavior system to start the given mission
		GetIntentionInterface()->Reset();
	}

	// Temp hack - some missions play an idle loop
	if ( m_mission > NO_MISSION )
	{
		StartIdleSound();
	}
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::PhysicsSimulate( void )
{
	BaseClass::PhysicsSimulate();

	if ( m_spawnArea == NULL )
	{
		m_spawnArea = GetLastKnownArea();
	}

	if ( HasAttribute( CFFBot::ALWAYS_CRIT ) && !m_Shared.InCond( TF_COND_CRITBOOSTED_USER_BUFF ) )
	{
		m_Shared.AddCond( TF_COND_CRITBOOSTED_USER_BUFF );
	}

       // force my speed to be recalculated to keep squad together and restore speed afterwards
       RecalculateSpeed();

	if ( IsInASquad() )
	{
		if ( GetSquad()->GetMemberCount() <= 1 || GetSquad()->GetLeader() == NULL )
		{
			// squad has collapsed - disband it
			LeaveSquad();
		}
	}


	// If we're dead, choose a new class.
	// We need to do this outside of the behavior system, since changing class can
	// sometimes force an immediate respawn, which will destroy the bot's existing actions out from under it.
	if ( !IsAlive() && !m_didReselectClass && ff_bot_keep_class_after_death.GetBool() == false && FFGameRules()->CanBotChangeClass( this ) )
	{
	}

	return BaseClass::ShouldGib( info );
}


//-----------------------------------------------------------------------------------------------------
bool CFFBot::IsAllowedToPickUpFlag( void ) const
{
	if ( !BaseClass::IsAllowedToPickUpFlag() )
	{
		return false;
	}

	// only the leader of a squad can pick up the flag
	if ( IsInASquad() && !GetSquad()->IsLeader( const_cast< CFFBot * >( this ) ) )
		return false;

	// mission bots can't pick up the flag
	return !IsOnAnyMission();
}


//-----------------------------------------------------------------------------------------------------
void CFFBot::InitClass( void )
{
	BaseClass::InitClass();
}

void CFFBot::ModifyMaxHealth( int nNewMaxHealth, bool bSetCurrentHealth /*= true*/, bool bAllowModelScaling /*= true*/ )
{
       // simplify max health modification for Fortress Forever

	if ( bSetCurrentHealth )
	{
		SetHealth( nNewMaxHealth );
	}

if ( bAllowModelScaling )
{
SetModelScale( m_fModelScaleOverride > 0.0f ? m_fModelScaleOverride : 1.0f );
}
}

//-----------------------------------------------------------------------------------------------------
/**
 * Invoked when a game event occurs
 */
void CFFBot::FireGameEvent( IGameEvent *event )
{
	const char *eventName = event->GetName();

	if ( FStrEq( eventName, "teamplay_point_captured" ) )
	{
		ClearMyControlPoint();

		int whoCapped = event->GetInt( "team" );
		int pointID = event->GetInt( "cp" );

		if ( whoCapped == GetTeamNumber() )
		{
			OnTerritoryCaptured( pointID );
		}
		else
		{
			OnTerritoryLost( pointID );

			m_justLostPointTimer.Start( RandomFloat( 10.0f, 20.0f ) );
		}
	}
	else if ( FStrEq( eventName, "teamplay_point_startcapture" ) )
	{
		int pointID = event->GetInt( "cp" );

		OnTerritoryContested( pointID );
	}
	else if ( FStrEq( eventName, "teamplay_flag_event" ) )
	{
		if ( event->GetInt( "eventtype" ) == TF_FLAGEVENT_PICKUP )
		{
			int iPlayer = event->GetInt( "player" );
			if ( iPlayer == entindex() )
			{
				// I just picked up the flag
				OnPickUp( NULL, NULL );
			}
		}
	}
}

	
//-----------------------------------------------------------------------------------------------------
void CFFBot::Event_Killed( const CTakeDamageInfo &info )
{
	BaseClass::Event_Killed( info );

	if ( HasProxy() )
	{
		GetProxy()->OnKilled();
	}

	// announce Spies
}


//-----------------------------------------------------------------------------------------------------
// Return true if the given point is being captured
bool CFFBot::IsPointBeingCaptured( CTeamControlPoint *point ) const
{
	if ( point == NULL )
		return false;

	if ( point->LastContestedAt() > 0.0f && ( gpGlobals->curtime - point->LastContestedAt() ) < 5.0f )
	{
		// the point is, or was very recently, contested
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// Return true if any point is being captured
bool CFFBot::IsAnyPointBeingCaptured( void ) const
{
	CTeamControlPointMaster *master = g_hControlPointMasters.Count() ? g_hControlPointMasters[0] : NULL;
	if ( master )
	{
		for( int i=0; i<master->GetNumPoints(); ++i )
		{
			CTeamControlPoint *point = master->GetControlPoint( i );

			if ( IsPointBeingCaptured( point ) )
				return true;
		}
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// Return true if we are within a short travel distance of the current point
bool CFFBot::IsNearPoint( CTeamControlPoint *point ) const
{
	CTFNavArea *myArea = GetLastKnownArea();

	if ( !myArea || !point )
	{
		return false;
	}

	CTFNavArea *pointArea = TheTFNavMesh()->GetControlPointCenterArea( point->GetPointIndex() );

	if ( !pointArea )
	{
		return false;
	}

	float travelToPoint = fabs( myArea->GetIncursionDistance( GetTeamNumber() ) - pointArea->GetIncursionDistance( GetTeamNumber() ) );

	return travelToPoint < ff_bot_near_point_travel_distance.GetFloat();
}


//---------------------------------------------------------------------------------------------
// Return time left to capture the point before we lose the game
float CFFBot::GetTimeLeftToCapture( void ) const
{
	if ( FFGameRules()->IsInKothMode() )
	{
		if ( FFGameRules()->GetKothTeamTimer( GetEnemyTeam( GetTeamNumber() ) ) )
		{
			return FFGameRules()->GetKothTeamTimer( GetEnemyTeam( GetTeamNumber() ) )->GetTimeRemaining();
		}
	}
	else if ( FFGameRules()->GetActiveRoundTimer() )
	{
		return FFGameRules()->GetActiveRoundTimer()->GetTimeRemaining();
	}

	return 0.0f;
}


//-----------------------------------------------------------------------------------------------------
// Do internal setup when control point changes
void CFFBot::SetupSniperSpotAccumulation( void )
{
	VPROF_BUDGET( "CFFBot::SetupSniperSpotAccumulation", "NextBot" );

	CBaseEntity *goalEntity = NULL;

       if ( FFGameRules()->GetGameType() == TF_GAMETYPE_CP )
       {
               goalEntity = GetMyControlPoint();
       }

	if ( !goalEntity )
	{
		ClearSniperSpots();
		return;
	}

	if ( goalEntity == m_snipingGoalEntity )
	{
		
		Vector toGoal = m_snipingGoalEntity->WorldSpaceCenter() - m_lastSnipingGoalEntityPosition;

		if ( toGoal.IsLengthLessThan( ff_bot_sniper_goal_entity_move_tolerance.GetFloat() ) )
		{
			// already set up
			return;
		}
	}

	ClearSniperSpots();

	int myTeam = GetTeamNumber();
	int enemyTeam = ( myTeam == FF_TEAM_BLUE ) ? FF_TEAM_RED : FF_TEAM_BLUE;

	bool isDefendingPoint = false;
	CTFNavArea *goalEntityArea = NULL;

	if ( FFGameRules()->GetGameType() == TF_GAMETYPE_ESCORT )
	{
		// the cart is owned by the invaders
		isDefendingPoint = ( goalEntity->GetTeamNumber() != myTeam );
		goalEntityArea = (CTFNavArea *)TheTFNavMesh()->GetNearestNavArea( goalEntity->WorldSpaceCenter(), GETNAVAREA_CHECK_GROUND, 500.0f );
	}
	else
	{
		isDefendingPoint = ( GetMyControlPoint()->GetOwner() == myTeam );
		goalEntityArea = TheTFNavMesh()->GetControlPointCenterArea( GetMyControlPoint()->GetPointIndex() );
	}

	// we are sniping a different control point - setup for new point accumulation
	m_sniperVantageAreaVector.RemoveAll();
	m_sniperTheaterAreaVector.RemoveAll();

	if ( !goalEntityArea )
	{
		return;
	}

	for( int i=0; i<TheNavAreas.Count(); ++i )
	{
		CTFNavArea *area = (CTFNavArea *)TheNavAreas[i];

		if ( !area->IsReachableByTeam( myTeam ) || !area->IsReachableByTeam( enemyTeam ) )
		{
			continue;
		}

		if ( area->GetIncursionDistance( enemyTeam ) <= goalEntityArea->GetIncursionDistance( enemyTeam ) )
		{
			m_sniperTheaterAreaVector.AddToTail( area );
		}

		// if this is my point, I can stand on it, or go a bit beyond it
		float myIncursionTolerance = ff_bot_sniper_spot_point_tolerance.GetFloat();

		if ( !isDefendingPoint )
		{
			// not my point, keep back from it a bit
			myIncursionTolerance *= -1.0f;
		}
		
		if ( area->GetIncursionDistance( myTeam ) <= goalEntityArea->GetIncursionDistance( myTeam ) + myIncursionTolerance )
		{
			m_sniperVantageAreaVector.AddToTail( area );
		}
	}

	m_snipingGoalEntity = goalEntity;
	m_lastSnipingGoalEntityPosition = goalEntity->WorldSpaceCenter();
}


//-----------------------------------------------------------------------------------------------------
// Randomly sample points within candidate areas to find good sniping positions
void CFFBot::AccumulateSniperSpots( void )
{
	VPROF_BUDGET( "CFFBot::AccumulateSniperSpots", "NextBot" );

	SetupSniperSpotAccumulation();

	if ( m_sniperVantageAreaVector.Count() == 0 || m_sniperTheaterAreaVector.Count() == 0 )
	{
		// retry every so often to catch cases where the incursion data is invalid during setup time
		// due to blocked/closed off areas, etc.
		if ( m_retrySniperSpotSetupTimer.IsElapsed() )
		{
			// retry
			ClearSniperSpots();
		}

		return;
	}

	SniperSpotInfo info;

	for( int count=0; count<ff_bot_sniper_spot_search_count.GetInt(); ++count )
	{
		// pick a random vantage area to sample
		int which = RandomInt( 0, m_sniperVantageAreaVector.Count()-1 );
		info.m_vantageArea = m_sniperVantageAreaVector[ which ];
		info.m_vantageSpot = info.m_vantageArea->GetRandomPoint();

		// pick a random theater area to sample
		which = RandomInt( 0, m_sniperTheaterAreaVector.Count()-1 );
		info.m_theaterArea = m_sniperTheaterAreaVector[ which ];
		info.m_theaterSpot = info.m_theaterArea->GetRandomPoint();

		info.m_range = ( info.m_vantageSpot - info.m_theaterSpot ).Length();
		if ( info.m_range < ff_bot_sniper_spot_min_range.GetFloat() )
		{
			// not long enough sightline
			continue;
		}

		for( int i=0; i<m_sniperSpotVector.Count(); ++i )
		{
			if ( ( info.m_vantageSpot - m_sniperSpotVector[i].m_vantageSpot ).IsLengthLessThan( ff_bot_sniper_spot_epsilon.GetFloat() ) )
			{
				// too close to existing spot
				continue;
			}
		}

		Vector eyeOffset( 0, 0, 60.0f );
		if ( IsLineOfFireClear( info.m_vantageSpot + eyeOffset, info.m_theaterSpot + eyeOffset ) )
		{
			// valid spot

			// maximize the time it takes the enemy to get to us
			info.m_advantage = info.m_vantageArea->GetIncursionDistance( GetEnemyTeam( GetTeamNumber() ) ) - info.m_theaterArea->GetIncursionDistance( GetEnemyTeam( GetTeamNumber() ) );

			// if we have already maxxed out our sniper spots, replace the worst one if this is better
			if ( m_sniperSpotVector.Count() >= ff_bot_sniper_spot_max_count.GetInt() )
			{
				int worst = -1;

				for( int i=0; i<m_sniperSpotVector.Count(); ++i )
				{
					if ( worst < 0 || m_sniperSpotVector[i].m_advantage < m_sniperSpotVector[ worst ].m_advantage )
					{
						worst = i;
					}
				}

				// if our new spot is better, replace it
				if ( info.m_advantage > m_sniperSpotVector[ worst ].m_advantage )
				{
					m_sniperSpotVector[ worst ] = info;
				}
			}
			else
			{
				m_sniperSpotVector.AddToTail( info );
			}
		}
	}

	if ( IsDebugging( NEXTBOT_BEHAVIOR ) )
	{
		for( int i=0; i<m_sniperSpotVector.Count(); ++i )
		{
			NDebugOverlay::Cross3D( m_sniperSpotVector[i].m_vantageSpot, 5.0f, 255, 0, 255, true, 0.1f );
			NDebugOverlay::Line( m_sniperSpotVector[i].m_vantageSpot, m_sniperSpotVector[i].m_theaterSpot, 0, 200, 0, true, 0.1f );
		}
	}
}



//-----------------------------------------------------------------------------------------------------
void CFFBot::ClearSniperSpots( void )
{
	m_sniperSpotVector.RemoveAll();
	m_sniperVantageAreaVector.RemoveAll();
	m_sniperTheaterAreaVector.RemoveAll();
	m_snipingGoalEntity = NULL;
	m_retrySniperSpotSetupTimer.Start( RandomFloat( 5.0f, 10.0f ) );
}



//---------------------------------------------------------------------------------------------
class CCollectReachableObjects : public ISearchSurroundingAreasFunctor
{
public:
	CCollectReachableObjects( const CFFBot *me, float maxRange, const CUtlVector< CHandle< CBaseEntity > > &potentialVector, CUtlVector< CHandle< CBaseEntity > > *collectionVector ) : m_potentialVector( potentialVector )
	{
		m_me = me;
		m_maxRange = maxRange;
		m_collectionVector = collectionVector;
	}

	virtual bool operator() ( CNavArea *area, CNavArea *priorArea, float travelDistanceSoFar )
	{
		// do any of the potential objects overlap this area?
		FOR_EACH_VEC( m_potentialVector, it )
		{
			CBaseEntity *obj = m_potentialVector[ it ];

			if ( obj && area->Contains( obj->WorldSpaceCenter() ) )
			{
				// reachable - keep it
				if ( !m_collectionVector->HasElement( obj ) )
				{
					m_collectionVector->AddToTail( obj );
				}
			}
		}
		return true;
	}

	virtual bool ShouldSearch( CNavArea *adjArea, CNavArea *currentArea, float travelDistanceSoFar )
	{
		if ( adjArea->IsBlocked( m_me->GetTeamNumber() ) )
		{
			return false;
		}

		if ( travelDistanceSoFar > m_maxRange )
		{
			// too far away
			return false;
		}

		return currentArea->IsContiguous( adjArea );
	}

	const CFFBot *m_me;
	float m_maxRange;
	const CUtlVector< CHandle< CBaseEntity > > &m_potentialVector;
	CUtlVector< CHandle< CBaseEntity > > *m_collectionVector;
};


//
// Search outwards from startSearchArea and collect all reachable objects from the given list that pass the given filter
// Items in selectedObjectVector will be approximately sorted in nearest-to-farthest order (because of SearchSurroundingAreas)
//
void CFFBot::SelectReachableObjects( const CUtlVector< CHandle< CBaseEntity > > &candidateObjectVector, 
									 CUtlVector< CHandle< CBaseEntity > > *selectedObjectVector, 
									 const INextBotFilter &filter, 
									 CNavArea *startSearchArea, 
									 float maxRange ) const
{
	if ( startSearchArea == NULL || selectedObjectVector == NULL )
		return;

	selectedObjectVector->RemoveAll();

	// filter candidate objects
	CUtlVector< CHandle< CBaseEntity > > filteredObjectVector;
	for( int i=0; i<candidateObjectVector.Count(); ++i )
	{
		if ( filter.IsSelected( candidateObjectVector[i] ) )
		{
			filteredObjectVector.AddToTail( candidateObjectVector[i] );
		}
	}

	// only keep those that are reachable by us
	CCollectReachableObjects collector( this, maxRange, filteredObjectVector, selectedObjectVector );
	SearchSurroundingAreas( startSearchArea, collector );
}


//---------------------------------------------------------------------------------------------
bool CFFBot::IsAmmoLow( void ) const
{
	CFFWeaponBase *myWeapon = GetActiveFFWeapon();
	if ( myWeapon )
	{
		if ( myWeapon->GetWeaponID() == FF_WEAPON_WRENCH )
		{
			// wrench is special. it's a melee weapon that wants ammo - metal
			return ( GetAmmoCount( TF_AMMO_METAL ) <= 0 );
		}

		if ( myWeapon->IsMeleeWeapon() )
		{
			// we never run out of ammo with a melee weapon
			return false;
		}

		// no projectile, no ammo needed
		const char *weaponAlias = WeaponIdToAlias( myWeapon->GetWeaponID() );
		if ( weaponAlias )
		{
			WEAPON_FILE_INFO_HANDLE	weaponInfoHandle = LookupWeaponInfoSlot( weaponAlias );
			if ( weaponInfoHandle != GetInvalidWeaponInfoHandle() )
			{
				CFFWeaponInfo *weaponInfo = static_cast< CFFWeaponInfo * >( GetFileWeaponInfoFromHandle( weaponInfoHandle ) );
				if ( weaponInfo && weaponInfo->GetWeaponData( FF_WEAPON_PRIMARY_MODE ).m_iProjectile == TF_PROJECTILE_NONE )
				{
					// we don't shoot anything, so we don't need ammo
					return false;
				}
			}
		}

		float ratio = (float)GetAmmoCount( TF_AMMO_PRIMARY ) / (float)( const_cast< CFFBot * >( this )->GetMaxAmmo( TF_AMMO_PRIMARY ) );

		if ( ratio < 0.2f )
		{
			return true;
		}
		//if ( !myWeapon->HasPrimaryAmmo() && myWeapon->GetWeaponID() != FF_WEAPON_BUILDER && myWeapon->GetWeaponID() != FF_WEAPON_MEDIGUN )
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
bool CFFBot::IsAmmoFull( void ) const
{
	bool isPrimaryFull = GetAmmoCount( TF_AMMO_PRIMARY ) >= const_cast< CFFBot * >( this )->GetMaxAmmo( TF_AMMO_PRIMARY );
	bool isSecondaryFull = GetAmmoCount( TF_AMMO_SECONDARY ) >= const_cast< CFFBot * >( this )->GetMaxAmmo( TF_AMMO_SECONDARY );

	if ( IsPlayerClass( CLASS_ENGINEER ) )
	{
		// wrench is special. it's a melee weapon that wants ammo - metal
		return ( GetAmmoCount( TF_AMMO_METAL ) >= 200 ) && isPrimaryFull && isSecondaryFull;
	}

	return isPrimaryFull && isSecondaryFull;

/*
	CFFWeaponBase *myWeapon = GetActiveFFWeapon();
	if ( myWeapon )
	{
		if ( IsPlayerClass( CLASS_ENGINEER ) )
		{
			// wrench is special. it's a melee weapon that wants ammo - metal
			return ( GetAmmoCount( TF_AMMO_METAL ) >= 200 );
		}

		if ( myWeapon->IsMeleeWeapon() )
		{
			// we never run out of ammo with a melee weapon
			return true;
		}

		// no projectile, no ammo needed
		const char *weaponAlias = WeaponIdToAlias( myWeapon->GetWeaponID() );
		if ( weaponAlias )
		{
			WEAPON_FILE_INFO_HANDLE	weaponInfoHandle = LookupWeaponInfoSlot( weaponAlias );
			if ( weaponInfoHandle != GetInvalidWeaponInfoHandle() )
			{
				CFFWeaponInfo *weaponInfo = static_cast< CFFWeaponInfo * >( GetFileWeaponInfoFromHandle( weaponInfoHandle ) );
				if ( weaponInfo && weaponInfo->GetWeaponData( FF_WEAPON_PRIMARY_MODE ).m_iProjectile == TF_PROJECTILE_NONE )
				{
					// we don't shoot anything, so we don't need ammo
					return true;
				}
			}
		}

		bool isPrimaryFull = GetAmmoCount( TF_AMMO_PRIMARY ) >= const_cast< CFFBot * >( this )->GetMaxAmmo( TF_AMMO_PRIMARY );
		bool isSecondaryFull = GetAmmoCount( TF_AMMO_SECONDARY ) >= const_cast< CFFBot * >( this )->GetMaxAmmo( TF_AMMO_SECONDARY );

		return isPrimaryFull && isSecondaryFull;
	}

	return false;
*/
}


bool CFFBot::IsDormantWhenDead( void ) const
{
	return false;
}


//-----------------------------------------------------------------------------------------------------
/**
 * When someone fires their weapon
 */
void CFFBot::OnWeaponFired( CBaseCombatCharacter *whoFired, CBaseCombatWeapon *weapon )
{
	VPROF_BUDGET( "CFFBot::OnWeaponFired", "NextBot" );

	BaseClass::OnWeaponFired( whoFired, weapon );

	if ( !whoFired || !whoFired->IsAlive() )
		return;

	if ( IsRangeGreaterThan( whoFired, ff_bot_notice_gunfire_range.GetFloat() ) )
		return;

	int noticeChance = 100;

	if ( IsQuietWeapon( (CFFWeaponBase *)weapon ) )
	{
		if ( IsRangeGreaterThan( whoFired, ff_bot_notice_quiet_gunfire_range.GetFloat() ) )
		{
			// too far away to hear in any event
			return;
		}

		switch( GetDifficulty() )
		{
		case EASY:
			noticeChance = 10;
			break;

		case NORMAL:
			noticeChance = 30;
			break;

		case HARD:
			noticeChance = 60;
			break;

		default:
		case EXPERT:
			noticeChance = 90;
			break;
		}

		if ( IsEnvironmentNoisy() )
		{
			// less likely to notice with all the noise
			noticeChance /= 2;
		}
	}
	else if ( IsRangeLessThan( whoFired, 1000.0f ) )
	{
		// loud gunfire in our area - it's now "noisy" for a bit
		m_noisyTimer.Start( 3.0f );
	}

	if ( RandomInt( 1, 100 ) > noticeChance )
	{
		return;
	}

	// notice the gunfire
	GetVisionInterface()->AddKnownEntity( whoFired );
}


//-----------------------------------------------------------------------------------------------------
// Return true if we match the given debug symbol
bool CFFBot::IsDebugFilterMatch( const char *name ) const
{
	// player classname
	if ( !Q_strnicmp( name, const_cast< CFFBot * >( this )->GetPlayerClass()->GetName(), Q_strlen( name ) ) )
	{
		return true;
	}

	return BaseClass::IsDebugFilterMatch( name );
}


//-----------------------------------------------------------------------------------------------------
class CFindClosestPotentiallyVisibleAreaToPos
{
public:
	CFindClosestPotentiallyVisibleAreaToPos( const Vector &pos )
	{
		m_pos = pos;
		m_closeArea = NULL;
		m_closeRangeSq = FLT_MAX;
	}

	bool operator() ( CNavArea *baseArea )
	{
		CTFNavArea *area = (CTFNavArea *)baseArea;

		Vector close;
		area->GetClosestPointOnArea( m_pos, &close );

		float rangeSq = ( close - m_pos ).LengthSqr();
		if ( rangeSq < m_closeRangeSq )
		{
			m_closeArea = area;
			m_closeRangeSq = rangeSq;
		}

		return true;
	}

	Vector m_pos;
	CTFNavArea *m_closeArea;
	float m_closeRangeSq;
};


//-----------------------------------------------------------------------------------------------------
// Update our view to watch where members of the given team will be coming from
void CFFBot::UpdateLookingAroundForIncomingPlayers( bool lookForEnemies )
{
	if ( !m_lookAtEnemyInvasionAreasTimer.IsElapsed() )
		return;

	const float maxLookInterval = 1.0f;
	m_lookAtEnemyInvasionAreasTimer.Start( RandomFloat( 0.333f, maxLookInterval ) );

	float minGazeRange = m_Shared.InCond( TF_COND_ZOOMED ) ? 750.0f : 150.0f;

	CTFNavArea *myArea = GetLastKnownArea();
	if ( myArea )
	{
		int team = GetTeamNumber();

		// if we want to look where teammates come from, we need to pass in
		// the *enemy* team, since the method collects *enemy* invasion areas
		if ( !lookForEnemies )
		{
			team = GetEnemyTeam( team );
		}

		const CUtlVector< CTFNavArea * > &invasionAreaVector = myArea->GetEnemyInvasionAreaVector( team );

		if ( invasionAreaVector.Count() > 0 )
		{
			// try to not look directly at walls
			const int retryCount = 20.0f;
			for( int r=0; r<retryCount; ++r )
			{
				int which = RandomInt( 0, invasionAreaVector.Count()-1 );
				Vector gazeSpot = invasionAreaVector[ which ]->GetRandomPoint() + Vector( 0, 0, 0.75f * HumanHeight );

				if ( IsRangeGreaterThan( gazeSpot, minGazeRange ) && GetVisionInterface()->IsLineOfSightClear( gazeSpot ) )
				{
					// use maxLookInterval so these looks override body aiming from path following
					GetBodyInterface()->AimHeadTowards( gazeSpot, IBody::INTERESTING, maxLookInterval, NULL, "Looking toward enemy invasion areas" );
					break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------------
/**
 * Update our view to keep an eye on areas where the enemy will be coming from
 */
void CFFBot::UpdateLookingAroundForEnemies( void )
{
	if ( !m_isLookingAroundForEnemies )
		return;

	if ( HasAttribute( CFFBot::IGNORE_ENEMIES ) )
		return;

	if ( m_Shared.IsControlStunned() )
		return;

	const float maxLookInterval = 1.0f;

	const CKnownEntity *known = GetVisionInterface()->GetPrimaryKnownThreat();

	if ( known )
	{
		if ( known->IsVisibleInFOVNow() )
		{
			if ( IsPlayerClass( CLASS_SPY ) && 
				 GetDifficulty() >= CFFBot::HARD &&
				 m_Shared.InCond( TF_COND_DISGUISED ) &&
				 !m_Shared.IsStealthed() )
			{
				// smart Spies don't look at their victims until it's too late...
				// look around at where *teammates* will be coming from to fool the enemy
				UpdateLookingAroundForIncomingPlayers( LOOK_FOR_FRIENDS );
				return;
			}

			// I see you!
			GetBodyInterface()->AimHeadTowards( known->GetEntity(), IBody::CRITICAL, 1.0f, NULL, "Aiming at a visible threat" );
			return;
		}

/* apparently sounds update last known position...
		if ( known->WasEverVisible() && known->GetTimeSinceLastSeen() < 3.0f )
		{
			// I saw you just a moment ago...
			GetBodyInterface()->AimHeadTowards( known->GetLastKnownPosition() + GetClassEyeHeight(), IBody::IMPORTANT, 1.0f, NULL, "Aiming at a last known threat position" );
			return;
		}
*/

		// known but not currently visible (I know you're around here somewhere)

		// if there is unobstructed space between us, turn around
		if ( IsLineOfSightClear( known->GetEntity(), IGNORE_ACTORS ) )
		{
			Vector toThreat = known->GetEntity()->GetAbsOrigin() - GetAbsOrigin();
			float threatRange = toThreat.NormalizeInPlace();

			float aimError = M_PI/6.0f;

			float s, c;
			FastSinCos( aimError, &s, &c );

			float error = threatRange * s;
			Vector imperfectAimSpot = known->GetEntity()->WorldSpaceCenter();
			imperfectAimSpot.x += RandomFloat( -error, error );
			imperfectAimSpot.y += RandomFloat( -error, error );

			GetBodyInterface()->AimHeadTowards( imperfectAimSpot, IBody::IMPORTANT, 1.0f, NULL, "Turning around to find threat out of our FOV" );
			return;
		}
			
		if ( !IsPlayerClass( CLASS_SNIPER ) )
		{
			// look toward potentially visible area nearest the last known position
			CTFNavArea *myArea = GetLastKnownArea();
			if ( myArea )
			{
				const CTFNavArea *closeArea = NULL;
				CFindClosestPotentiallyVisibleAreaToPos find( known->GetLastKnownPosition() );
				myArea->ForAllPotentiallyVisibleAreas( find );

				closeArea = find.m_closeArea;

				if ( closeArea )
				{
					// try to not look directly at walls
					const int retryCount = 10.0f;
					for( int r=0; r<retryCount; ++r )
					{
						Vector gazeSpot = closeArea->GetRandomPoint() + Vector( 0, 0, 0.75f * HumanHeight );

						if ( GetVisionInterface()->IsLineOfSightClear( gazeSpot ) )
						{
							// use maxLookInterval so these looks override body aiming from path following
							GetBodyInterface()->AimHeadTowards( gazeSpot, IBody::IMPORTANT, maxLookInterval, NULL, "Looking toward potentially visible area near known but hidden threat" );
							return;
						}
					}					

					// can't find a clear line to look along
					if ( IsDebugging( NEXTBOT_VISION | NEXTBOT_ERRORS ) )
					{
						ConColorMsg( Color( 255, 255, 0, 255 ), "%3.2f: %s can't find clear line to look at potentially visible near known but hidden entity %s(#%d)\n", 
										gpGlobals->curtime,
										GetDebugIdentifier(),
										known->GetEntity()->GetClassname(),
										known->GetEntity()->entindex() );
					}
				}
				else if ( IsDebugging( NEXTBOT_VISION | NEXTBOT_ERRORS ) )
				{
					ConColorMsg( Color( 255, 255, 0, 255 ), "%3.2f: %s no potentially visible area to look toward known but hidden entity %s(#%d)\n", 
									gpGlobals->curtime,
									GetDebugIdentifier(),
									known->GetEntity()->GetClassname(),
									known->GetEntity()->entindex() );
				}
			}

			return;
		}
	}

	// no known threat - look toward where enemies will come from
	UpdateLookingAroundForIncomingPlayers( LOOK_FOR_ENEMIES );
}


//---------------------------------------------------------------------------------------------
class CFindVantagePoint : public ISearchSurroundingAreasFunctor
{
public:
	CFindVantagePoint( int enemyTeamIndex )
	{
		m_enemyTeamIndex = enemyTeamIndex;
		m_vantageArea = NULL;
	}

	virtual bool operator() ( CNavArea *baseArea, CNavArea *priorArea, float travelDistanceSoFar )
	{
		CTFNavArea *area = (CTFNavArea *)baseArea;

		CTeam *enemyTeam = GetGlobalTeam( m_enemyTeamIndex );
		for( int i=0; i<enemyTeam->GetNumPlayers(); ++i )
		{
			CFFPlayer *enemy = (CFFPlayer *)enemyTeam->GetPlayer(i);

			if ( !enemy->IsAlive() || !enemy->GetLastKnownArea() )
				continue;

			CTFNavArea *enemyArea = (CTFNavArea *)enemy->GetLastKnownArea();
			if ( enemyArea->IsCompletelyVisible( area ) )
			{
				// nearby area from which we can see the enemy team
				m_vantageArea = area;
				return false;
			}
		}

		return true;
	}

	int m_enemyTeamIndex;
	CTFNavArea *m_vantageArea;
};


//-----------------------------------------------------------------------------------------------------
// Return a nearby area where we can see a member of the enemy team
CTFNavArea *CFFBot::FindVantagePoint( float maxTravelDistance ) const
{
	CFindVantagePoint find( GetTeamNumber() == FF_TEAM_BLUE ? FF_TEAM_RED : FF_TEAM_BLUE );
	SearchSurroundingAreas( GetLastKnownArea(), find, maxTravelDistance );
	return find.m_vantageArea;
}


//-----------------------------------------------------------------------------------------------------
/**
 * Return perceived danger of threat (0=none, 1=immediate deadly danger)
 * @todo: Move this to contextual query
 * @todo: Differentiate between potential threats (that sentry up ahead along our route) and immediate threats (the sentry I'm in range of)
 */
float CFFBot::GetThreatDanger( CBaseCombatCharacter *who ) const
{
	if ( who == NULL )
		return 0.0f;

	if ( IsPlayerClass( CLASS_SNIPER ) )
	{
		if ( IsRangeGreaterThan( who, ff_bot_sniper_personal_space_range.GetFloat() ) )
		{
			// far away enemies are no threat to a Sniper
			return 0.0f;
		}
	}

	if ( who->IsPlayer() )
	{
		CFFPlayer *player = ToFFPlayer( who );

		// ubers are scary
		if ( player->m_Shared.IsInvulnerable() )
			return 1.0f;

		switch( player->GetPlayerClass()->GetClassIndex() )
		{
		case CLASS_MEDIC:
			return 0.2f;		// 1/5

		case CLASS_ENGINEER:
		case CLASS_SNIPER:
			return 0.4f;		// 2/5

		case CLASS_SCOUT:
		case CLASS_SPY:
		case CLASS_DEMOMAN:
			return 0.6f;		// 3/5

		case CLASS_SOLDIER:
		case CLASS_HEAVYWEAPONS:
			return 0.8f;		// 4/5

		case CLASS_PYRO:
			return 1.0f;		// 5/5
		}

	}
	else
	{
		// sentry gun
		CObjectSentrygun *sentry = dynamic_cast< CObjectSentrygun * >( who );
		if ( sentry )
		{
			if ( !sentry->IsAlive() || sentry->IsPlacing() || sentry->HasSapper() || sentry->IsPlasmaDisabled() || sentry->IsUpgrading() || sentry->IsBuilding() )
				return 0.0f;

			switch( sentry->GetUpgradeLevel() )
			{
			case 3:		return 1.0f;
			case 2:		return 0.8f;
			default:	return 0.6f;
			}
		}
	}

	return 0.0f;
}


//-----------------------------------------------------------------------------------------------------
/**
 * Return the max range at which we can effectively attack
 */
float CFFBot::GetMaxAttackRange( void ) const
{
	CFFWeaponBase *myWeapon = GetActiveFFWeapon();
	if ( !myWeapon )
		return 0.0f;

	if ( myWeapon->IsMeleeWeapon() )
	{
		return 100.0f;
	}
	
	if ( myWeapon->IsWeapon( FF_WEAPON_FLAMETHROWER ) )
	{
	}

	if ( HasWeaponRestriction( PRIMARY_ONLY ) )
	{
		Weapon_Switch( Weapon_GetSlot( TF_WPN_TYPE_PRIMARY ) );
		return true;
	}

	if ( HasWeaponRestriction( SECONDARY_ONLY ) )
	{
		Weapon_Switch( Weapon_GetSlot( TF_WPN_TYPE_SECONDARY ) );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// Equip the best weapon we have to attack the given threat
void CFFBot::EquipBestWeaponForThreat( const CKnownEntity *threat )
{
	if ( EquipRequiredWeapon() )
		return;


	 
	CFFWeaponBase *primary = dynamic_cast< CFFWeaponBase *>( Weapon_GetSlot( TF_WPN_TYPE_PRIMARY ) );
	if ( !IsCombatWeapon( primary ) )
	{
		primary = NULL;
	}

	CFFWeaponBase *secondary = dynamic_cast< CFFWeaponBase *>( Weapon_GetSlot( TF_WPN_TYPE_SECONDARY ) );
	if ( !IsCombatWeapon( secondary ) )
	{
		secondary = NULL;
	}

	
			}
		}
	}

	// fall back to our secondary (or go right to it if its the only thing we have that has reach)
	CBaseCombatWeapon *secondary = Weapon_GetSlot( TF_WPN_TYPE_SECONDARY );
	if ( secondary )
	{
		if ( GetAmmoCount( TF_AMMO_SECONDARY ) > 0 )
		{
			Weapon_Switch( secondary );
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// Force us to equip and use this weapon until popped off the required stack
void CFFBot::PushRequiredWeapon( CFFWeaponBase *weapon )
{
	m_requiredWeaponStack.Push( weapon );
}


//-----------------------------------------------------------------------------------------------------
// Pop top required weapon off of stack and discard
void CFFBot::PopRequiredWeapon( void )
{
	m_requiredWeaponStack.Pop();
}


//-----------------------------------------------------------------------------------------------------
// return true if given weapon can be used to attack
bool CFFBot::IsCombatWeapon( CFFWeaponBase *weapon ) const
{
	if ( weapon == MY_CURRENT_GUN )		// MY_CURRENT_GUN == NULL
	{
		weapon = GetActiveFFWeapon();
	}

	if ( weapon )
	{
		switch ( weapon->GetWeaponID() )
		{
		case FF_WEAPON_MEDIGUN:
		case FF_WEAPON_PDA:
		case FF_WEAPON_PDA_ENGINEER_BUILD:
		case FF_WEAPON_PDA_ENGINEER_DESTROY:
		case FF_WEAPON_PDA_SPY:
		case FF_WEAPON_BUILDER:
		case FF_WEAPON_DISPENSER:
		case FF_WEAPON_INVIS:
		case FF_WEAPON_LUNCHBOX:
		case FF_WEAPON_BUFF_ITEM:
		case FF_WEAPON_PUMPKIN_BOMB:
			return false;
		};
	}

	return true;
}


//-----------------------------------------------------------------------------------------------------
// return true if given weapon is a "hitscan" weapon
bool CFFBot::IsHitScanWeapon( CFFWeaponBase *weapon ) const
{
	if ( weapon == MY_CURRENT_GUN )		// MY_CURRENT_GUN == NULL
	{
		weapon = GetActiveFFWeapon();
	}

	if ( weapon )
	{
		switch ( weapon->GetWeaponID() )
		{
		case FF_WEAPON_SHOTGUN_PRIMARY:
		case FF_WEAPON_SHOTGUN_SOLDIER:
		case FF_WEAPON_SHOTGUN_HWG:
		case FF_WEAPON_SHOTGUN_PYRO:
		case FF_WEAPON_SCATTERGUN:
		case FF_WEAPON_SNIPERRIFLE:
		case FF_WEAPON_MINIGUN:
		case FF_WEAPON_SMG:
		case FF_WEAPON_CHARGED_SMG:
		case FF_WEAPON_PISTOL:
		case FF_WEAPON_PISTOL_SCOUT:
		case FF_WEAPON_REVOLVER:
		case FF_WEAPON_SENTRY_BULLET:
		case FF_WEAPON_SENTRY_ROCKET:
		case FF_WEAPON_SENTRY_REVENGE:
		case FF_WEAPON_HANDGUN_SCOUT_PRIMARY:
		case FF_WEAPON_HANDGUN_SCOUT_SECONDARY:
		case FF_WEAPON_SODA_POPPER:
		case FF_WEAPON_SNIPERRIFLE_DECAP:
		case FF_WEAPON_PEP_BRAWLER_BLASTER:
		case FF_WEAPON_SNIPERRIFLE_CLASSIC:
			return true;
		};
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// return true if given weapon "sprays" bullets/fire/etc continuously (ie: not individual rockets/etc)
bool CFFBot::IsContinuousFireWeapon( CFFWeaponBase *weapon ) const
{
	if ( weapon == MY_CURRENT_GUN )
	{
		weapon = GetActiveFFWeapon();
	}

	if ( !IsCombatWeapon( weapon ) )
		return false;

	if ( weapon )
	{
		switch ( weapon->GetWeaponID() )
		{
		case FF_WEAPON_ROCKETLAUNCHER:
		case FF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case FF_WEAPON_GRENADELAUNCHER:
		case FF_WEAPON_PIPEBOMBLAUNCHER:
		case FF_WEAPON_PISTOL:
		case FF_WEAPON_PISTOL_SCOUT:
		case FF_WEAPON_FLAREGUN:
		case FF_WEAPON_JAR:
		case FF_WEAPON_COMPOUND_BOW:
			return false;
		};
	}

	return true;

}


//-----------------------------------------------------------------------------------------------------
// return true if given weapon launches explosive projectiles with splash damage
bool CFFBot::IsExplosiveProjectileWeapon( CFFWeaponBase *weapon ) const
{
	if ( weapon == MY_CURRENT_GUN )
	{
		weapon = GetActiveFFWeapon();
	}

	if ( weapon )
	{
		switch ( weapon->GetWeaponID() )
		{
		case FF_WEAPON_ROCKETLAUNCHER:
		case FF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case FF_WEAPON_GRENADELAUNCHER:
		case FF_WEAPON_PIPEBOMBLAUNCHER:
		case FF_WEAPON_JAR:
			return true;
		};
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// return true if given weapon has small clip and long reload cost (ie: rocket launcher, etc)
bool CFFBot::IsBarrageAndReloadWeapon( CFFWeaponBase *weapon ) const
{
	if ( weapon == MY_CURRENT_GUN )
	{
		weapon = GetActiveFFWeapon();
	}

	if ( weapon ) 
	{
		switch ( weapon->GetWeaponID() )
		{
		case FF_WEAPON_ROCKETLAUNCHER:
		case FF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case FF_WEAPON_GRENADELAUNCHER:
		case FF_WEAPON_PIPEBOMBLAUNCHER:
		case FF_WEAPON_SCATTERGUN:
			return true;
		};
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// Return true if given weapon doesn't make much sound when used (ie: spy knife, etc)
bool CFFBot::IsQuietWeapon( CFFWeaponBase *weapon ) const
{
	if ( weapon == MY_CURRENT_GUN )
	{
		weapon = GetActiveFFWeapon();
	}

	if ( weapon ) 
	{
		switch ( weapon->GetWeaponID() )
		{
		case FF_WEAPON_KNIFE:
		case FF_WEAPON_FISTS:
		case FF_WEAPON_PDA:
		case FF_WEAPON_PDA_ENGINEER_BUILD:
		case FF_WEAPON_PDA_ENGINEER_DESTROY:
		case FF_WEAPON_PDA_SPY:
		case FF_WEAPON_BUILDER:
		case FF_WEAPON_MEDIGUN:
		case FF_WEAPON_DISPENSER:
		case FF_WEAPON_INVIS:
		case FF_WEAPON_FLAREGUN:
		case FF_WEAPON_LUNCHBOX:
		case FF_WEAPON_JAR:
		case FF_WEAPON_COMPOUND_BOW:
		case FF_WEAPON_SWORD:
		case FF_WEAPON_CROSSBOW:
			return true;
		};
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// Return true if a weapon has no obstructions along the line between the given points
bool CFFBot::IsLineOfFireClear( const Vector &from, const Vector &to ) const
{
	trace_t trace;
	NextBotTraceFilterIgnoreActors botFilter( NULL, COLLISION_GROUP_NONE );
	CTraceFilterIgnoreFriendlyCombatItems ignoreFriendlyCombatFilter( this, COLLISION_GROUP_NONE, GetTeamNumber() );
	CTraceFilterChain filter( &botFilter, &ignoreFriendlyCombatFilter );

	UTIL_TraceLine( from, to, MASK_SOLID_BRUSHONLY, &filter, &trace );

	return !trace.DidHit();
}


//-----------------------------------------------------------------------------------------------------
// Return true if a weapon has no obstructions along the line from our eye to the given position
bool CFFBot::IsLineOfFireClear( const Vector &where ) const
{
	return IsLineOfFireClear( const_cast< CFFBot * >( this )->EyePosition(), where );
}


//-----------------------------------------------------------------------------------------------------
// Return true if a weapon has no obstructions along the line between the given point and entity
bool CFFBot::IsLineOfFireClear( const Vector &from, CBaseEntity *who ) const
{
	trace_t trace;
	NextBotTraceFilterIgnoreActors botFilter( NULL, COLLISION_GROUP_NONE );
	CTraceFilterIgnoreFriendlyCombatItems ignoreFriendlyCombatFilter( this, COLLISION_GROUP_NONE, GetTeamNumber() );
	CTraceFilterChain filter( &botFilter, &ignoreFriendlyCombatFilter );

	UTIL_TraceLine( from, who->WorldSpaceCenter(), MASK_SOLID_BRUSHONLY, &filter, &trace );

	return !trace.DidHit() || trace.m_pEnt == who;
}


//-----------------------------------------------------------------------------------------------------
// Return true if a weapon has no obstructions along the line from our eye to the given entity
bool CFFBot::IsLineOfFireClear( CBaseEntity *who ) const
{
	return IsLineOfFireClear( const_cast< CFFBot * >( this )->EyePosition(), who );
}


//-----------------------------------------------------------------------------------------------------
bool CFFBot::IsEntityBetweenTargetAndSelf( CBaseEntity *other, CBaseEntity *target )
{
	Vector toTarget = target->GetAbsOrigin() - GetAbsOrigin();
	float rangeToTarget = toTarget.NormalizeInPlace();

	Vector toOther = other->GetAbsOrigin() - GetAbsOrigin();
	float rangeToOther = toOther.NormalizeInPlace();

	return rangeToOther < rangeToTarget && DotProduct( toTarget, toOther ) > 0.7071f;
}


//-----------------------------------------------------------------------------------------------------
// Return true if we are sure this player actually is an enemy spy
bool CFFBot::IsKnownSpy( CFFPlayer *player ) const
{
	for( int i=0; i<m_knownSpyVector.Count(); ++i )
	{
		CFFPlayer *spy = m_knownSpyVector[i];
		if ( spy && player->entindex() == spy->entindex() )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------------
// Return true if we suspect this player might be an enemy spy
CFFBot::SuspectedSpyInfo_t* CFFBot::IsSuspectedSpy( CFFPlayer *pPlayer )
{
	for( int i=0; i<m_suspectedSpyVector.Count(); ++i )
	{
		SuspectedSpyInfo_t* pSpyInfo = m_suspectedSpyVector[i];
		CFFPlayer* pSpy = pSpyInfo->m_suspectedSpy;
		if ( pSpy && pPlayer->entindex() == pSpy->entindex() )
		{
			return pSpyInfo;
		}
	}

	return NULL;
}


//-----------------------------------------------------------------------------------------------------
// Note that this player might be a spy
void CFFBot::SuspectSpy( CFFPlayer *pPlayer )
{
	SuspectedSpyInfo_t* pSpyInfo = IsSuspectedSpy( pPlayer );

	// Start suspecting this spy if we're not aware of them until now
	if( pSpyInfo == NULL )
	{
		// add to head for LRU effect
		pSpyInfo = new SuspectedSpyInfo_t;
		pSpyInfo->m_suspectedSpy = pPlayer;
		m_suspectedSpyVector.AddToHead( pSpyInfo );
	}
	
	// Suspicious!
	pSpyInfo->Suspect();

	// Too suspicious?
	if( pSpyInfo->TestForRealizing() )
	{
		RealizeSpy( pPlayer );
	}
}

void CFFBot::SuspectedSpyInfo_t::Suspect()
{
	int nCurTime = floor(gpGlobals->curtime);

	// Add our new entry
	m_touchTimes.AddToHead( nCurTime );
}

bool CFFBot::SuspectedSpyInfo_t::TestForRealizing()
{
	// Remove any old entries
	int nCurTime = floor(gpGlobals->curtime);
	int nCutoffTime = nCurTime - ff_bot_suspect_spy_touch_interval.GetInt();

	FOR_EACH_VEC_BACK( m_touchTimes, i )
	{
		if( m_touchTimes[i] <= nCutoffTime )
			m_touchTimes.Remove( i );
	}

	// Add our new entry
	m_touchTimes.AddToHead( nCurTime );
	
	// Setup an array of bools representing the past few seconds that we want
	// to look for suspicious activity
	CUtlVector<bool> vecSeconds;
	vecSeconds.SetSize( ff_bot_suspect_spy_touch_interval.GetInt() );
	FOR_EACH_VEC( vecSeconds, i )
	{
		vecSeconds[i] = false;
	}

	// Go through each time chunk and mark if there was suspicious activity
	FOR_EACH_VEC( m_touchTimes, i )
	{
		int nTouchTime = m_touchTimes[i];
		int nTimeSlot = nCurTime - nTouchTime;

		if( nTimeSlot >= 0 && nTimeSlot < vecSeconds.Count() )
		{
			vecSeconds[nTimeSlot] = true;
		}
	}

	// If all are true, then the spy has been suspicious enough to warrant being realized
	FOR_EACH_VEC( vecSeconds, i )
	{
		if( vecSeconds[i] == false )
		{
			return false;
		}
	}

	return true;
}


bool CFFBot::SuspectedSpyInfo_t::IsCurrentlySuspected()
{
	float flCutoffTime = gpGlobals->curtime - ff_bot_suspect_spy_forget_cooldown.GetFloat();
	if( m_touchTimes.Count() && m_touchTimes.Head() > flCutoffTime )
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------------------------------
// Note that this player *IS* a spy
void CFFBot::RealizeSpy( CFFPlayer *pPlayer )
{
	// We already know about this spy
	if ( IsKnownSpy( pPlayer ) )
		return;

	// add to head for LRU effect
	m_knownSpyVector.AddToHead( pPlayer );

	// inform my teammates
	SpeakConceptIfAllowed( MP_CONCEPT_PLAYER_CLOAKEDSPY );

	// If I am suspicious of this spy, make everyone around me know that
	// they should be suspicious too
	SuspectedSpyInfo_t* pSuspectInfo = IsSuspectedSpy( pPlayer );
	if( pSuspectInfo && pSuspectInfo->IsCurrentlySuspected() )
	{
		// Tell others around us we've realized there's a spy
		CUtlVector< CFFPlayer * > playerVector;
		CollectPlayers( &playerVector, GetTeamNumber(), COLLECT_ONLY_LIVING_PLAYERS );
		FOR_EACH_VEC( playerVector, i )
		{
			CFFPlayer* pOther = playerVector[i];

			if( !pOther->IsBot() )
				continue;

			//Make sure they're close by
			Vector vecBetween = EyePosition() - pOther->EyePosition();
			if( vecBetween.IsLengthLessThan( 512.f ) )
			{
				// If they dont know about this spy
				CFFBot* pOtherBot = static_cast<CFFBot*>( pOther );
				if( !pOtherBot->IsKnownSpy( pPlayer ) )
				{
					// I was suspicious that they were a spy, make my friend suspicious as well.
					
					pOtherBot->SuspectSpy( pPlayer );

					// Tell them about it
					pOtherBot->RealizeSpy( pPlayer );
				}
			}
		}
	}
	
}


//-----------------------------------------------------------------------------------------------------
// Remove player from spy suspect system
void CFFBot::ForgetSpy( CFFPlayer *pPlayer )
{
	StopSuspectingSpy( pPlayer );
	m_knownSpyVector.FindAndFastRemove( pPlayer );
}

void CFFBot::StopSuspectingSpy( CFFPlayer *pPlayer )
{
	// Find the entry matching this spy
	for( int i=0; i<m_suspectedSpyVector.Count(); ++i )
	{
		SuspectedSpyInfo_t* pSpyInfo = m_suspectedSpyVector[i];
		CFFPlayer* pSpy = pSpyInfo->m_suspectedSpy;
		if ( pSpy && pPlayer->entindex() == pSpy->entindex() )
		{
			delete pSpyInfo;
			m_suspectedSpyVector.Remove(i);
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------------
// Return the nearest human player on the given team who is looking directly at me
CFFPlayer *CFFBot::GetClosestHumanLookingAtMe( int team ) const
{
	CUtlVector< CFFPlayer * > otherVector;
	CollectPlayers( &otherVector, team, COLLECT_ONLY_LIVING_PLAYERS );

	float closeRange = FLT_MAX;
	CFFPlayer *close = NULL;

	for( int i=0; i<otherVector.Count(); ++i )
	{
		CFFPlayer *other = otherVector[i];

		if ( other->IsBot() )
			continue;

		Vector otherEye, otherForward;
		other->EyePositionAndVectors( &otherEye, &otherForward, NULL, NULL );

		Vector toMe = const_cast< CFFBot * >( this )->EyePosition() - otherEye;
		float range = toMe.NormalizeInPlace();

		if ( range < closeRange )
		{
			const float cosTolerance = 0.98f;
			if ( DotProduct( toMe, otherForward ) > cosTolerance )
			{
				// a human is looking toward me - check LOS
				if ( IsLineOfSightClear( otherEye, IGNORE_NOTHING, other ) )
				{
					close = other;
					closeRange = range;
				}
			}
		}
	}

	return close;
}


//-----------------------------------------------------------------------------------------------------
// become a member of the given squad
void CFFBot::JoinSquad( CFFBotSquad *squad )
{
	if ( squad )
	{
		squad->Join( this );
		m_squad = squad;
	}
}


//-----------------------------------------------------------------------------------------------------
// leave our current squad
void CFFBot::LeaveSquad( void )
{
	if ( m_squad )
	{
		m_squad->Leave( this );
		m_squad = NULL;
	}
}

//-----------------------------------------------------------------------------------------------------
// leave our current squad
void CFFBot::DeleteSquad( void )
{
	if ( m_squad )
	{
		m_squad = NULL;
	}
}

//---------------------------------------------------------------------------------------------
bool CFFBot::IsWeaponRestricted( CFFWeaponBase *weapon ) const
{
       if ( !weapon )
       {
               return false;
       }

       if ( HasWeaponRestriction( MELEE_ONLY ) )
       {
               return !weapon->IsMeleeWeapon();
       }

       // Fortress Forever does not maintain TF2-style loadout slots
       // Ignore PRIMARY_ONLY and SECONDARY_ONLY restrictions

       return false;
}


//---------------------------------------------------------------------------------------------
//
// Return true if there is something we want to reflect directly ahead of us
//
bool CFFBot::ShouldFireCompressionBlast( void )
{
	if ( FFGameRules()->IsInTraining() )
	{
		// no reflection in training mode
		return false;
	}

	if ( !ff_bot_pyro_always_reflect.GetBool() )
	{
		if ( IsDifficulty( CFFBot::EASY ) )
		{
			// easy bots can't reflect at all
			return false;
		}

		if ( IsDifficulty( CFFBot::NORMAL ) )
		{
			// normal bots reflect some of the time
			if ( TransientlyConsistentRandomValue( 1.0f ) < 0.5f )
			{
				return false;
			}
		}

		if ( IsDifficulty( CFFBot::HARD ) )
		{
			// hard bots reflect most of the time
			if ( TransientlyConsistentRandomValue( 1.0f ) < 0.1f )
			{
				return false;
			}
		}
	}

				}

				if ( pushVictim->GetGroundEntity() == NULL )
				{
					// they are in the air - juggle them some of the time
					return ( TransientlyConsistentRandomValue( 0.5f ) < 0.5f );
				}

				if ( pushVictim->IsCapturingPoint() )
				{
					// push them off the point!
					return true;
				}

				// be pushy sometimes
				if ( TransientlyConsistentRandomValue( 3.0f ) < 0.5f )
				{
					return true;
				}
			}
		}
	}


	Vector vecEye = EyePosition();
	Vector vecForward, vecRight, vecUp;

	AngleVectors( EyeAngles(), &vecForward, &vecRight, &vecUp );

	Vector vecCenter = vecEye + vecForward * 128;
	Vector vecSize = Vector( 128, 128, 64 );

	const int maxCollectedEntities = 128;
	CBaseEntity	*pObjects[ maxCollectedEntities ];
	int count = UTIL_EntitiesInBox( pObjects, maxCollectedEntities, vecCenter - vecSize, vecCenter + vecSize, FL_CLIENT | FL_GRENADE );

	for ( int i = 0; i < count; i++ )
	{
		CBaseEntity *pObject = pObjects[i];
		if ( pObject == this )
			continue;

		if ( pObject->GetTeamNumber() == GetTeamNumber() )
			continue;

		// should air blast player logic is already done before this loop
		if ( pObject->IsPlayer() )
			continue;

		// is this something I want to deflect?
		if ( !pObject->IsDeflectable() )
			continue;

		if ( FClassnameIs( pObject, "ff_projectile_rocket" ) || FClassnameIs( pObject, "ff_projectile_energy_ball" ) )
		{
			// is it headed right for me?
			Vector vecThemUnitVel = pObject->GetAbsVelocity();
			vecThemUnitVel.z = 0.0f;
			vecThemUnitVel.NormalizeInPlace();

			Vector horzForward( vecForward.x, vecForward.y, 0.0f );
			horzForward.NormalizeInPlace();

			if ( DotProduct( horzForward, vecThemUnitVel ) > -ff_bot_pyro_deflect_tolerance.GetFloat() )
				continue;
		}

		// can I see it?
		if ( !GetVisionInterface()->IsLineOfSightClear( pObject->WorldSpaceCenter() ) )
			continue;

		// bounce it!
		return true;
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// Compute a pseudo random value (0-1) that stays consistent for the 
// given period of time, but changes unpredictably each period.
float CFFBot::TransientlyConsistentRandomValue( float period, int seedValue ) const
{
	CNavArea *area = GetLastKnownArea();
	if ( !area )
	{
		return 0.0f;
	}

	// this term stays stable for 'period' seconds, then changes in an unpredictable way
	int timeMod = (int)( gpGlobals->curtime / period ) + 1;
	return fabs( FastCos( (float)( seedValue + ( entindex() * area->GetID() * timeMod ) ) ) );
}


//---------------------------------------------------------------------------------------------
// Given a target entity, find a target within 'maxSplashRadius' that has clear line of fire
// to both the target entity and to me.
bool CFFBot::FindSplashTarget( CBaseEntity *target, float maxSplashRadius, Vector *splashTarget ) const
{
	if ( !target || !splashTarget )
		return false;

	*splashTarget = target->WorldSpaceCenter();

	const int retryCount = 50;
	for( int i=0; i<retryCount; ++i )
	{
		Vector probe = target->WorldSpaceCenter() + RandomVector( -maxSplashRadius, maxSplashRadius );

		trace_t trace;
		NextBotTraceFilterIgnoreActors filter( NULL, COLLISION_GROUP_NONE );

		UTIL_TraceLine( target->WorldSpaceCenter(), probe, MASK_SOLID_BRUSHONLY, &filter, &trace );
		if ( trace.DidHitWorld() )
		{
			// can we shoot this spot?
			if ( IsLineOfFireClear( trace.endpos ) )
			{
				// yes, found a corner-sticky target
				*splashTarget = trace.endpos;

				NDebugOverlay::Line( target->WorldSpaceCenter(), trace.endpos, 255, 0, 0, true, 60.0f );
				NDebugOverlay::Cross3D( trace.endpos, 5.0f, 255, 255, 0, true, 60.0f );

				return true;
			}
		}
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// Restrict bot's attention to only this entity (or radius around this entity) to the exclusion of everything else
void CFFBot::SetAttentionFocus( CBaseEntity *focusOn )
{
	m_attentionFocusEntity = focusOn;
}


//---------------------------------------------------------------------------------------------
// Remove attention focus restrictions
void CFFBot::ClearAttentionFocus( void )
{
	m_attentionFocusEntity = NULL;
}


//---------------------------------------------------------------------------------------------
bool CFFBot::IsAttentionFocused( void ) const
{
	return m_attentionFocusEntity != NULL;
}


//---------------------------------------------------------------------------------------------
bool CFFBot::IsAttentionFocusedOn( CBaseEntity *who ) const
{
	if ( m_attentionFocusEntity == NULL || who == NULL )
	{
		return false;
	}

	if ( m_attentionFocusEntity->entindex() == who->entindex() )
	{
		// specifically focused on this entity
		return true;
	}

	CFFBotActionPoint *actionPoint = dynamic_cast< CFFBotActionPoint * >( m_attentionFocusEntity.Get() );
	if ( actionPoint )
	{
		// we attend to everything within the action point's radius
		return actionPoint->IsWithinRange( who );
	}

	return false;
}


//---------------------------------------------------------------------------------------------
// Notice the given threat after the given number of seconds have elapsed
void CFFBot::DelayedThreatNotice( CHandle< CBaseEntity > who, float noticeDelay )
{
	float when = gpGlobals->curtime + noticeDelay;

	// if we already have a delayed notice for this threat, ignore the new one unless the delay is less
	for( int i=0; i<m_delayedNoticeVector.Count(); ++i )
	{
		if ( m_delayedNoticeVector[i].m_who == who )
		{
			if ( m_delayedNoticeVector[i].m_when > when )
			{
				// update delay to shorter time
				m_delayedNoticeVector[i].m_when = when;
			}
			return;
		}
	}

	// new notice
	DelayedNoticeInfo delay;
	delay.m_who = who;
	delay.m_when = when;
	m_delayedNoticeVector.AddToTail( delay );
}


//---------------------------------------------------------------------------------------------
void CFFBot::UpdateDelayedThreatNotices( void )
{
	for( int i=0; i<m_delayedNoticeVector.Count(); ++i )
	{
		if ( m_delayedNoticeVector[i].m_when <= gpGlobals->curtime )
		{
			// delay is up - notice this threat
			CBaseEntity *who = m_delayedNoticeVector[i].m_who;

			if ( who )
			{
				if ( who->IsPlayer() )
				{
					CFFPlayer *player = ToFFPlayer( who );
					if ( player->IsPlayerClass( CLASS_SPY ) )
					{
						RealizeSpy( player );
					}
				}

				GetVisionInterface()->AddKnownEntity( who );
			}

			m_delayedNoticeVector.Remove( i );
			--i;
		}
	}
}


//---------------------------------------------------------------------------------------------
void CFFBot::GiveRandomItem( loadout_positions_t loadoutPosition )
{
       // Fortress Forever has no item schema support
       // function intentionally left blank
}


//---------------------------------------------------------------------------------------------
bool CFFBot::IsSquadmate( CFFPlayer *who ) const
{
       if ( !m_squad || !who || !who->IsBotOfType( FF_BOT_TYPE ) )
		return false;

	return GetSquad() == ToTFBot( who )->GetSquad();
}


//---------------------------------------------------------------------------------------------
// Set Spy disguise to be a class that someone on the enemy team is actually using
void CFFBot::DisguiseAsMemberOfEnemyTeam( void )
{
	CUtlVector< CFFPlayer * > enemyVector;
	CollectPlayers( &enemyVector, GetEnemyTeam( GetTeamNumber() ) );

	int disguise = RandomInt( CLASS_SCOUT, CLASS_CIVILIAN-1 );

	if ( enemyVector.Count() > 0 )
	{
		disguise = enemyVector[ RandomInt( 0, enemyVector.Count()-1 ) ]->GetPlayerClass()->GetClassIndex();
	}

	m_Shared.Disguise( GetEnemyTeam( GetTeamNumber() ), disguise );
}


//---------------------------------------------------------------------------------------------
void CFFBot::ClearTags( void )
{
	m_tags.RemoveAll();
}


//---------------------------------------------------------------------------------------------
void CFFBot::AddTag( const char *tag )
{
	if ( !HasTag( tag ) )
	{
		m_tags.AddToTail( CFmtStr( "%s", tag ) );
	}
}


//---------------------------------------------------------------------------------------------
void CFFBot::RemoveTag( const char *tag )
{
	for ( int i=0; i<m_tags.Count(); ++i )
	{
		if ( FStrEq( tag, m_tags[i] ) )
		{
			m_tags.Remove(i);
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------
// TODO: Make this an efficient lookup/match
bool CFFBot::HasTag( const char *tag )
{
	for( int i=0; i<m_tags.Count(); ++i )
	{
		if ( FStrEq( tag, m_tags[i] ) )
		{
			return true;
		}
	}

	return false;
}


//---------------------------------------------------------------------------------------------
CBaseObject *CFFBot::GetNearestKnownSappableTarget( void )
{
	CUtlVector< CKnownEntity > knownVector;
	GetVisionInterface()->CollectKnownEntities( &knownVector );

	CBaseObject *closeObject = NULL;
	float closeObjectRangeSq = 500.0f * 500.0f;

	for( int i=0; i<knownVector.Count(); ++i )
	{
		CBaseObject *enemyObject = dynamic_cast< CBaseObject * >( knownVector[i].GetEntity() );
		if ( enemyObject && !enemyObject->HasSapper() && IsEnemy( enemyObject ) )
		{
			float rangeSq = GetRangeSquaredTo( enemyObject );
			if ( rangeSq < closeObjectRangeSq )
			{
				closeObjectRangeSq = rangeSq;
				closeObject = enemyObject;
			}		
		}
	}

	return closeObject;
}


//-----------------------------------------------------------------------------------------
Action< CFFBot > *CFFBot::OpportunisticallyUseWeaponAbilities( void )
{
	if ( !m_opportunisticTimer.IsElapsed() )
	{
		return NULL;
	}

	m_opportunisticTimer.Start( RandomFloat( 0.1f, 0.2f ) );


	// if I'm wearing a charge shield, use it!
	if ( IsPlayerClass( CLASS_DEMOMAN ) && m_Shared.IsShieldEquipped() )
	{
		Vector forward;
		EyeVectors( &forward );
		bool bShouldCharge = GetLocomotionInterface()->IsPotentiallyTraversable( GetAbsOrigin(), GetAbsOrigin() + 100.0f * forward, ILocomotion::IMMEDIATELY );
		if ( HasAttribute( CFFBot::AIR_CHARGE_ONLY ) && ( GetGroundEntity() || GetAbsVelocity().z > 0 ) )
		{
			bShouldCharge = false;
		}

		if ( bShouldCharge )
		{
			PressAltFireButton();
		}
	}
	// if I'm wearing parachute, check if I should activate my parachute
	else if ( m_Shared.IsParachuteEquipped() )
	{
		bool bIsBurning = m_Shared.InCond( TF_COND_BURNING );
		float flHealthPercent = (float)GetHealth() / GetMaxHealth();
		const float flHealthThreshold = 0.5f;
		// should I activate parachute?
		if ( !m_Shared.InCond( TF_COND_PARACHUTE_DEPLOYED ) )
		{
			float flMinParachuteGroundDistance = 300.f;
			// check if I'm falling, high enough off the ground to deploy parachute, and not burning
			if ( flHealthPercent >= flHealthThreshold && !bIsBurning && GetAbsVelocity().z < 0 && GetLocomotionInterface()->IsPotentiallyTraversable( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -flMinParachuteGroundDistance ), ILocomotion::IMMEDIATELY ) )
			{
				PressJumpButton();
			}
		}
		// should I deactivate parachute?
		else
		{
			float flCancelParachuteDistance = 150.f;
			// if I'm burning or close enough to landing, deactivate the parachute or health less than some threshold
			if ( flHealthPercent < flHealthThreshold || bIsBurning || !GetLocomotionInterface()->IsPotentiallyTraversable( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -flCancelParachuteDistance ), ILocomotion::IMMEDIATELY ) )
			{
				PressJumpButton();
			}
		}
	}

	
	}
	else if ( m_autoJumpTimer.IsElapsed() )
	{
		m_autoJumpTimer.Start( RandomFloat( m_flAutoJumpMin, m_flAutoJumpMax ) );
		return true;
	}
	
	return false;
}


void CFFBot::SetFlagTarget( CCaptureFlag* pFlag )
{
	if ( m_hFollowingFlagTarget != pFlag )
	{
		if ( m_hFollowingFlagTarget )
		{
			m_hFollowingFlagTarget->RemoveFollower( this );
		}

		m_hFollowingFlagTarget = pFlag;
		if ( m_hFollowingFlagTarget )
		{
			m_hFollowingFlagTarget->AddFollower( this );
		}
	}
}


int CFFBot::DrawDebugTextOverlays(void)
{
	int offset = ff_bot_debug_tags.GetBool() ? 1 : BaseClass::DrawDebugTextOverlays();

	CUtlString strTags = "Tags : ";
	for( int i=0; i<m_tags.Count(); ++i )
	{
		strTags.Append( m_tags[i] );
		strTags.Append( " " );
	}

	EntityText( offset, strTags.Get(), 0 );
	offset++;

	return offset;
}


void CFFBot::AddEventChangeAttributes( const CFFBot::EventChangeAttributes_t* newEvent )
{
	m_eventChangeAttributes.AddToTail( newEvent );
}


const CFFBot::EventChangeAttributes_t* CFFBot::GetEventChangeAttributes( const char* pszEventName ) const
{
	for ( int i=0; i<m_eventChangeAttributes.Count(); ++i )
	{
		if ( FStrEq( m_eventChangeAttributes[i]->m_eventName, pszEventName ) )
		{
			return m_eventChangeAttributes[i];
		}
	}
	return NULL;
}


void CFFBot::OnEventChangeAttributes( const CFFBot::EventChangeAttributes_t* pEvent )
{
	if ( pEvent )
	{
		SetDifficulty( pEvent->m_skill );

		ClearWeaponRestrictions();
		SetWeaponRestriction( pEvent->m_weaponRestriction );

		SetMission( pEvent->m_mission );

		ClearAllAttributes();
		SetAttribute( pEvent->m_attributeFlags );

		SetMaxVisionRangeOverride( pEvent->m_maxVisionRange );

