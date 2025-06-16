//========= Copyright Valve Corporation, All rights reserved. ============//
// ff_bot_generator.cpp
// Entity to spawn a collection of TFBots
// Michael Booth, September 2009

#include "cbase.h"

#include "ff_bot_generator.h"

#include "bot/ff_bot.h"
#include "bot/ff_bot_manager.h"
#include "ff_gamerules.h"
#include "tier3/tier3.h"
#include "vgui/ILocalize.h"

extern ConVar ff_bot_prefix_name_with_difficulty;
extern ConVar ff_bot_difficulty;

extern void CreateBotName( int iTeam, int iClassIndex, CFFBot::DifficultyType skill, char* pBuffer, int iBufferSize );

//------------------------------------------------------------------------------

BEGIN_DATADESC( CFFBotGenerator )
	DEFINE_KEYFIELD( m_spawnCount,		FIELD_INTEGER,	"count" ),
	DEFINE_KEYFIELD( m_maxActiveCount,	FIELD_INTEGER,	"maxActive" ),
	DEFINE_KEYFIELD( m_spawnInterval,	FIELD_FLOAT,	"interval" ),
	DEFINE_KEYFIELD( m_className,		FIELD_STRING,	"class" ),
	DEFINE_KEYFIELD( m_teamName,		FIELD_STRING,	"team" ),
	DEFINE_KEYFIELD( m_actionPointName,	FIELD_STRING,	"action_point" ),
	DEFINE_KEYFIELD( m_initialCommand,	FIELD_STRING,	"initial_command" ),
	DEFINE_KEYFIELD( m_bSuppressFire,	FIELD_BOOLEAN,	"suppressFire" ),
	DEFINE_KEYFIELD( m_bDisableDodge,	FIELD_BOOLEAN,	"disableDodge" ),
	DEFINE_KEYFIELD( m_iOnDeathAction,	FIELD_INTEGER,	"actionOnDeath" ),
	DEFINE_KEYFIELD( m_bUseTeamSpawnpoint,	FIELD_BOOLEAN,	"useTeamSpawnPoint" ),
	DEFINE_KEYFIELD( m_difficulty,		FIELD_INTEGER,	"difficulty" ),
	DEFINE_KEYFIELD( m_bRetainBuildings,	FIELD_BOOLEAN,	"retainBuildings" ),
	DEFINE_KEYFIELD( m_bSpawnOnlyWhenTriggered,	FIELD_BOOLEAN, "spawnOnlyWhenTriggered" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetSuppressFire", InputSetSuppressFire ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetDisableDodge", InputSetDisableDodge ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetDifficulty", InputSetDifficulty ),
	DEFINE_INPUTFUNC( FIELD_STRING, "CommandGotoActionPoint", InputCommandGotoActionPoint ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetAttentionFocus", InputSetAttentionFocus ),
	DEFINE_INPUTFUNC( FIELD_STRING, "ClearAttentionFocus", InputClearAttentionFocus ),

	DEFINE_INPUTFUNC( FIELD_VOID, "SpawnBot", InputSpawnBot ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RemoveBots", InputRemoveBots ),

	DEFINE_OUTPUT( m_onSpawned, "OnSpawned" ),
	DEFINE_OUTPUT( m_onExpended, "OnExpended" ),
	DEFINE_OUTPUT( m_onBotKilled, "OnBotKilled" ),

	DEFINE_THINKFUNC( GeneratorThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( bot_generator, CFFBotGenerator );

enum
{
	kOnDeath_Respawn,
	kOnDeath_RemoveSelf,
	kOnDeath_MoveToSpectatorTeam,
};

//------------------------------------------------------------------------------
CFFBotGenerator::CFFBotGenerator( void ) 
	: m_bBotChoosesClass(false)
	, m_bSuppressFire(false)
	, m_bDisableDodge(false)
	, m_bUseTeamSpawnpoint(false)
	, m_bRetainBuildings(false)
	, m_bExpended(false)
	, m_iOnDeathAction(kOnDeath_RemoveSelf)
	, m_difficulty(CFFBot::UNDEFINED)
	, m_spawnCountRemaining(0)
	, m_bSpawnOnlyWhenTriggered(false)
	, m_bEnabled(true)
{
	SetThink( NULL );
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;

	if ( m_bExpended )
	{
		return;
	}

	SetThink( &CFFBotGenerator::GeneratorThink );

	if ( m_spawnCountRemaining )
	{
		// already generating - don't restart count
		return;
	}
	SetNextThink( gpGlobals->curtime );
	m_spawnCountRemaining = m_spawnCount;
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;

	// just stop thinking
	SetThink( NULL );
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputSetSuppressFire( inputdata_t &inputdata )
{
	m_bSuppressFire = inputdata.value.Bool();
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputSetDisableDodge( inputdata_t &inputdata )
{
	m_bDisableDodge = inputdata.value.Bool();
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputSetDifficulty( inputdata_t &inputdata )
{
	m_difficulty = clamp( inputdata.value.Int(), (int) CFFBot::UNDEFINED, (int) CFFBot::EXPERT );
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputCommandGotoActionPoint( inputdata_t &inputdata )
{
	CFFBotActionPoint *pActionPoint = dynamic_cast<CFFBotActionPoint *>( gEntList.FindEntityByName( NULL, inputdata.value.String() ) );
	if ( pActionPoint == NULL )
	{
		return;
	}
	for ( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CHandle< CFFBot > hBot = m_spawnedBotVector[i];
		if ( hBot == NULL )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		if ( hBot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		hBot->SetActionPoint( pActionPoint );
		hBot->OnCommandString( "goto action point" );
		++i;
	}
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputSetAttentionFocus( inputdata_t &inputdata )
{
	CBaseEntity *focus = gEntList.FindEntityByName( NULL, inputdata.value.String() );

	if ( focus == NULL )
	{
		return;
	}

	for( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CFFBot *bot = m_spawnedBotVector[i];

		if ( !bot || bot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}

		bot->SetAttentionFocus( focus );

		++i;
	}
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputClearAttentionFocus( inputdata_t &inputdata )
{
	for( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CFFBot *bot = m_spawnedBotVector[i];

		if ( !bot || bot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}

		bot->ClearAttentionFocus();

		++i;
	}
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputSpawnBot( inputdata_t &inputdata )
{
	if ( m_bEnabled )
	{
		SpawnBot();
	}
}

//------------------------------------------------------------------------------
void CFFBotGenerator::InputRemoveBots( inputdata_t &inputdata )
{
	for( int i = 0; i < m_spawnedBotVector.Count(); i++ )
	{
		CFFBot *pBot = m_spawnedBotVector[i];
		if ( pBot )
		{
			pBot->Remove();
			engine->ServerCommand( UTIL_VarArgs( "kickid %d\n", pBot->GetUserID() ) );
		}

		m_spawnedBotVector.FastRemove(i);
	}
}

//------------------------------------------------------------------------------
void CFFBotGenerator::OnBotKilled( CFFBot *pBot )
{
	m_onBotKilled.FireOutput( pBot, this );
}

//------------------------------------------------------------------------------

void CFFBotGenerator::Activate()
{
	BaseClass::Activate();
	m_bBotChoosesClass = FStrEq( m_className.ToCStr(), "auto" );
	m_moveGoal = gEntList.FindEntityByName( NULL, m_actionPointName.ToCStr() );
}

//------------------------------------------------------------------------------
void CFFBotGenerator::GeneratorThink( void )
{
	// still waiting for the real game to start?
	gamerules_roundstate_t roundState = FFGameRules()->State_Get();
	if ( roundState >= GR_STATE_TEAM_WIN || roundState < GR_STATE_PREROUND ||  FFGameRules()->IsInWaitingForPlayers() )
	{
		SetNextThink( gpGlobals->curtime + 1.0f );
		return;
	}

	// create the bot finally...
	if ( !m_bSpawnOnlyWhenTriggered )
	{
		SpawnBot();
	}
}

//------------------------------------------------------------------------------
void CFFBotGenerator::SpawnBot( void )
{
	// did we exceed the max active count?
	for ( int i = 0; i < m_spawnedBotVector.Count(); )
	{
		CHandle< CFFBot > hBot = m_spawnedBotVector[i];
		if ( hBot == NULL )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		if ( hBot->GetTeamNumber() == TEAM_SPECTATOR )
		{
			m_spawnedBotVector.FastRemove(i);
			continue;
		}
		++i;
	}

	if ( m_spawnedBotVector.Count() >= m_maxActiveCount )
	{
		SetNextThink( gpGlobals->curtime + 0.1f );
		return;
	}

	char name[256];
	CFFBot *bot = TheTFBots().GetAvailableBotFromPool();
	if ( bot == NULL )
	{
		CreateBotName( TEAM_UNASSIGNED, CLASS_UNDEFINED, (CFFBot::DifficultyType)m_difficulty, name, sizeof(name) );
		bot = NextBotCreatePlayerBot< CFFBot >( name );
	}

	if ( bot ) 
	{										   
		m_spawnedBotVector.AddToTail( bot );


		bot->SetSpawner( this );

		if ( m_bUseTeamSpawnpoint == false )
		{
			bot->SetSpawnPoint( this );		
		}

		if ( m_bSuppressFire )
		{
			bot->SetAttribute( CFFBot::SUPPRESS_FIRE );
		}

		if ( m_bRetainBuildings )
		{
			bot->SetAttribute( CFFBot::RETAIN_BUILDINGS );
		}

		if ( m_bDisableDodge )
		{
			bot->SetAttribute( CFFBot::DISABLE_DODGE );
		}

		if ( m_difficulty != CFFBot::UNDEFINED )
		{
			bot->SetDifficulty( (CFFBot::DifficultyType )m_difficulty );
		}

		// propagate the generator's spawn flags into all bots generated
		bot->ClearBehaviorFlag( TFBOT_ALL_BEHAVIOR_FLAGS );
		bot->SetBehaviorFlag( m_spawnflags );

		switch ( m_iOnDeathAction )
		{
		case kOnDeath_RemoveSelf:
			bot->SetAttribute( CFFBot::REMOVE_ON_DEATH );
			break;
		case kOnDeath_MoveToSpectatorTeam:
			bot->SetAttribute( CFFBot::BECOME_SPECTATOR_ON_DEATH );
			break;
		} // switch

		bot->SetActionPoint( dynamic_cast<CFFBotActionPoint *>( m_moveGoal.Get() ) );

		// pick a team and force the team change
		// HandleCommand_JoinTeam() may fail, but this should always succeed
		int iTeam = TEAM_UNASSIGNED;
		if ( FStrEq( m_teamName.ToCStr(), "auto" ) )
		{
			iTeam = bot->GetAutoTeam();
		}
		else if ( FStrEq( m_teamName.ToCStr(), "spectate" ) )
		{
			iTeam = TEAM_SPECTATOR;
		}
		else
		{
			for ( int i = 0; i < FF_TEAM_COUNT; ++i )
			{
				COMPILE_TIME_ASSERT( FF_TEAM_COUNT == ARRAYSIZE( g_aTeamNames ) );
				if ( FStrEq( m_teamName.ToCStr(), g_aTeamNames[i] ) )
				{
					iTeam = i;
					break;
				}
			}
		}
		if ( iTeam == TEAM_UNASSIGNED )
		{
			iTeam = bot->GetAutoTeam();
		}
		bot->ChangeTeam( iTeam, false, false );
		
		const char* pClassName =  m_bBotChoosesClass ? bot->GetNextSpawnClassname() : m_className.ToCStr();
		bot->HandleCommand_JoinClass( pClassName );

		// in training, reset the after the bot joins the class
		if ( FFGameRules()->IsInTraining() )
		{
			CFFBot::DifficultyType skill = bot->GetDifficulty();
			CreateBotName( iTeam, bot->GetPlayerClass()->GetClassIndex(), skill, name, sizeof(name) );
			engine->SetFakeClientConVarValue( bot->edict(), "name", name );
		}

		if ( bot->IsAlive() == false )
		{
			bot->ForceRespawn();
		}

		// make sure the bot is facing the right way.
		// @todo Tom Bui: for some reason it is still turning towards another direction...need to investigate
		bot->SnapEyeAngles( GetAbsAngles() );

		if ( FStrEq( m_initialCommand.ToCStr(), "" ) == false )
		{
			// @note Tom Bui: we call Update() once here to make sure the bot is ready to receive commands
			bot->Update();
			bot->OnCommandString( m_initialCommand.ToCStr() );
		}
		m_onSpawned.FireOutput( bot, this );

		--m_spawnCountRemaining;
		if ( m_spawnCountRemaining )
		{
			SetNextThink( gpGlobals->curtime + m_spawnInterval );
		}
		else
		{
			SetThink( NULL );
			m_onExpended.FireOutput( this, this );
			m_bExpended = true;
		}
	}
}

//------------------------------------------------------------------------------

BEGIN_DATADESC( CFFBotActionPoint )
	DEFINE_KEYFIELD( m_stayTime,			FIELD_FLOAT,	"stay_time" ),
	DEFINE_KEYFIELD( m_desiredDistance,		FIELD_FLOAT,	"desired_distance" ),
	DEFINE_KEYFIELD( m_nextActionPointName,	FIELD_STRING,	"next_action_point" ),
	DEFINE_KEYFIELD( m_command,				FIELD_STRING,	"command" ),
	DEFINE_OUTPUT( m_onReachedActionPoint, "OnBotReached" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( bot_action_point, CFFBotActionPoint );

//------------------------------------------------------------------------------

CFFBotActionPoint::CFFBotActionPoint()
: m_stayTime( 0.0f )
, m_desiredDistance( 1.0f )

{

}

//------------------------------------------------------------------------------

void CFFBotActionPoint::Activate()
{
	BaseClass::Activate();
	m_moveGoal = gEntList.FindEntityByName( NULL, m_nextActionPointName.ToCStr() );
}

//------------------------------------------------------------------------------

bool CFFBotActionPoint::IsWithinRange( CBaseEntity *entity )
{
	return ( entity->GetAbsOrigin() - GetAbsOrigin() ).IsLengthLessThan( m_desiredDistance );
}

//------------------------------------------------------------------------------

void CFFBotActionPoint::ReachedActionPoint( CFFBot* pBot )
{
	if ( FStrEq( m_command.ToCStr(), "" ) == false )
	{
		pBot->OnCommandString( m_command.ToCStr() );
	}
	m_onReachedActionPoint.FireOutput( pBot, this );
}

//------------------------------------------------------------------------------
