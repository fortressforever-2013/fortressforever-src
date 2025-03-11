
// ff_lualib_globals.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_scriptman.h"
#include "ff_entity_system.h"
#include "ff_gamerules.h"
#include "ff_grenade_base.h"
#include "ff_info_script.h"
#include "ff_triggerclip.h"
#include "ff_player.h"
#include "ff_utils.h"
#include "ff_team.h"

#include "beam_shared.h"
#include "buttons.h"
#include "doors.h"
#include "recipientfilter.h"
#include "triggers.h"
#include "filesystem.h"
#include "sharedInterface.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#undef MINMAX_H
#undef min
#undef max

#define MAX_MENU_LEN 240

#include "ff_scheduleman.h"
#include "ff_timerman.h"
#include "ff_menuman.h"

#include "ff_luacontext.h"
#include "ff_scriptman.h"

#include "omnibot_interface.h"

#include "LuaBridge/LuaBridge.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool g_Disable_Timelimit;

//----------------------------------------------------------------------------
// defines
#define temp_max(a,b) (((a)>(b))?(a):(b))

//---------------------------------------------------------------------------
using namespace luabridge;

//---------------------------------------------------------------------------
class CClassLimits
{
public:
	CClassLimits()
	{
		int def = -1;
		scout = def;
		sniper = def;
		soldier = def;
		demoman = def;
		medic = def;
		hwguy = def;
		pyro = def;
		engineer = def;
		civilian = def;
	}

public:
	int scout;
	int sniper;
	int soldier;
	int demoman;
	int medic;
	int hwguy;
	int pyro;
	int spy;
	int engineer;
	int civilian;
};

class CPlayerLimits
{
public:
	CPlayerLimits()
	{
		blue = 0;
		red = 0;
		yellow = -1;
		green = -1;
	}

public:
	int blue;
	int red;
	int yellow;
	int green;
};

class CHudBoxBorder
{
public:
	CHudBoxBorder()
	{
		clr = Color();
		width = 0;
	}
	CHudBoxBorder(Color _clr)
	{
		clr = _clr;
		width = 1;
	}
	CHudBoxBorder(Color _clr, int _width)
	{
		clr = _clr;
		width = _width;
	}

public:
	Color clr;
	int width;
};

//---------------------------------------------------------------------------
// FFLib Namespace
//---------------------------------------------------------------------------
namespace FFLib
{
	// returns if the entity of the specified type
	// uses the Classify function for evaluation
	bool IsOfClass(CBaseEntity* pEntity, int classType)
	{
		if( !pEntity )
			return false;

		return ( pEntity->Classify() == classType );
	}

	// is entity a dispenser
	bool IsDispenser(CBaseEntity* pEntity)
	{
		return IsOfClass(pEntity, CLASS_DISPENSER);
	}

	// is entity a sentry gun
	bool IsSentrygun(CBaseEntity* pEntity)
	{
		return IsOfClass(pEntity, CLASS_SENTRYGUN);
	}

	// is entity a detpack
	bool IsDetpack( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_DETPACK );
	}

	// is entity a jump pad
	bool IsJumpPad( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_MANCANNON );
	}

	// is entity a buildable
	bool IsBuildable( CBaseEntity *pEntity )
	{
		return IsDispenser(pEntity) || IsSentrygun(pEntity) || IsDetpack(pEntity) || IsJumpPad(pEntity);
	}

	// is the entity a grenade
	bool IsGrenade(CBaseEntity* pEntity)
	{
		// Yeah, the simple life, man
		return !!( pEntity->GetFlags() & FL_GRENADE );
		/*
		return (IsOfClass(pEntity, CLASS_GREN) ||
				IsOfClass(pEntity, CLASS_GREN_EMP) ||
				IsOfClass(pEntity, CLASS_GREN_NAIL) ||
				IsOfClass(pEntity, CLASS_GREN_MIRV) ||
				IsOfClass(pEntity, CLASS_GREN_MIRVLET) ||
				IsOfClass(pEntity, CLASS_GREN_NAPALM) ||
				IsOfClass(pEntity, CLASS_GREN_GAS) ||
				IsOfClass(pEntity, CLASS_GREN_CONC)
				*/
	}

	// is the entity a projectile
	bool IsProjectile(CBaseEntity* pEntity)
	{
		return dynamic_cast< CFFProjectileBase * >( pEntity ) != NULL;
	}

	// is entity an infoscript
	bool IsInfoScript( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_INFOSCRIPT );
	}

	bool IsBaseTrigger( CBaseEntity* pEntity )
	{
		return IsOfClass( pEntity, CLASS_TRIGGER );
	}

	bool IsTriggerMultiple( CBaseEntity* pEntity )
	{
		return IsOfClass( pEntity, CLASS_TRIGGER_MULTIPLE );
	}

	bool IsTriggerHurt( CBaseEntity* pEntity )
	{
		return IsOfClass( pEntity, CLASS_TRIGGER_HURT );
	}

	// is entity a trigger ff script
	bool IsTriggerScript(CBaseEntity* pEntity)
	{
		return IsOfClass(pEntity, CLASS_TRIGGERSCRIPT);
	}

	// is entity a trigger ff clip
	bool IsTriggerClip(CBaseEntity* pEntity)
	{
		return IsOfClass(pEntity, CLASS_TRIGGER_CLIP);
	}

	// is the entity a miniturret
	bool IsTurret( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_TURRET );
	}

	// is the entity a backpack
	bool IsBackpack( CBaseEntity* pEntity )
	{
		return IsOfClass( pEntity, CLASS_BACKPACK );
	}

	void ChatToAll(const char *szMessage)
	{
		CBroadcastRecipientFilter filter;
		UTIL_SayText2Filter(filter, 0, true, "FF_Chat_LUA", szMessage );
	}

	void ChatToPlayer(CFFPlayer *pPlayer, const char *szMessage)
	{
		if (!pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UTIL_SayText2Filter(filter, 0, true, "FF_Chat_LUA", szMessage );
	}

	void BroadcastMessage(const char* szMessage)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE);
			WRITE_STRING(szMessage);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}

	void BroadcastMessage(const char* szMessage, float fDuration)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_DURATION);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}

	void BroadcastMessage(const char* szMessage, float fDuration, int iColorID)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(iColorID);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}
	

	void BroadcastMessage(const char* szMessage, float fDuration, const char* szColor )
	{
		int r, g, b;
		if(sscanf( szColor, "%i %i %i", &r, &g, &b ) != 3)
			r=g=b=255;
		
		r = clamp(r,0,255);
		g = clamp(g,0,255);
		b = clamp(b,0,255);

		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR_CUSTOM);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(r);
			WRITE_SHORT(g);
			WRITE_SHORT(b);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE);
			WRITE_STRING(szMessage);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage, float fDuration)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_DURATION);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage, float fDuration, int iColorID)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(iColorID);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage, float fDuration, const char* szColor)
	{
		if(NULL == pPlayer)
			return;
		
		int r, g, b;
		if(sscanf( szColor, "%i %i %i", &r, &g, &b ) != 3)
			r=g=b=255;
		
		r = clamp(r,0,255);
		g = clamp(g,0,255);
		b = clamp(b,0,255);

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR_CUSTOM);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(r);
			WRITE_SHORT(g);
			WRITE_SHORT(b);
		MessageEnd();
	}

	void BroadcastSound(const char* szSound)
	{
		CBroadcastRecipientFilter filter;

		CFFEntitySystemHelper* pHelperInst = CFFEntitySystemHelper::GetInstance();
		if(pHelperInst)
			pHelperInst->EmitSound( filter, pHelperInst->entindex(), szSound);

		
		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_snd: %s", szSound),"broadcast_snd");
	}

	void SendPlayerSound(CFFPlayer* pPlayer, const char* szSound)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);

		CFFEntitySystemHelper* pHelperInst = CFFEntitySystemHelper::GetInstance();
		if(pHelperInst)
			pHelperInst->EmitSound( filter, pHelperInst->entindex(), szSound);
	}

	void SetGlobalRespawnDelay(float delay)
	{
		mp_respawndelay.SetValue( temp_max( 0.0f, delay ) );
	}

	void GoToIntermission()
	{
		if (FFGameRules())
			FFGameRules()->GoToIntermission();
	}

	void DisableTimeLimit(bool _b)
	{
		g_Disable_Timelimit = _b;
	}

	void DisableTimeLimit(void)
	{
		DisableTimeLimit(true);
	}

	bool HasGameStarted()
	{
		if (FFGameRules())
			return FFGameRules()->HasGameStarted();
		else
			return true;
	}

	bool ApplyToParseFlags( const LuaRef& table, bool *pbFlags )
	{
		if( table.isValid() && ( table.isTable() ) )
		{
			// Iterate through the table
			for( Iterator ib( table ); !ib.isNil(); ++ib )
			{
				//std::string strKey = object_cast< std::string >( ib.key() );

				LuaRef val = table[ib.key()];

				if( val.isNumber() )
				{
					int iIndex =  - 1;
					
					try
					{
						iIndex = (int) val;
					}
					catch( ... )
					{
					}

					// Make sure within bounds
					if( ( iIndex >= 0 ) && ( iIndex < AT_MAX_FLAG ) )
						pbFlags[ iIndex ] = true;
				}
				else
				{
					// Warning( "[ResetParseFlags] Only handles integers in the table!\n" );
				}
			}

			return true;
		}

		return false;
	}

	void ApplyToAll( const LuaRef& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags );
		}
	}

	void ApplyToTeam( CFFTeam *pTeam, const LuaRef& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) && pTeam )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags, pTeam->GetTeamNumber() );
		}
	}

	void ApplyToPlayer( CFFPlayer *pPlayer, const LuaRef& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) && pPlayer )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags, TEAM_UNASSIGNED, pPlayer );
		}
	}

	void ResetMap( const LuaRef& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags, TEAM_UNASSIGNED, NULL, true );
		}
	}

/*
	void RespawnAllPlayers( void )
	{		
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );

				pPlayer->RemoveItems();
				pPlayer->Spawn();
				//pPlayer->KillAndRemoveItems();
			}
		}
	}

	void KillAndRespawnAllPlayers( void )
	{
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				pPlayer->KillAndRemoveItems();
			}
		}
	}
*/

	void ConsoleToAll(const char* szMessage)
	{
		Msg( "%s\n", szMessage );
	}

	void ServerCommand(const char* command)
	{
		if ( !command[0] )
			return;

		// deny any string that includes rcon_password
		// can't just check for the start because ' rcon_password blah' and 'dummy; rcon_password blah' are also valid
		if ( Q_stristr(command, "rcon_password") )
			return;

		engine->ServerCommand( UTIL_VarArgs( "%s\n", command ) );
	}

	LuaRef IncludeScript(const char* script)
	{
		lua_State *L = _scriptman.GetLuaState();
		lua_getglobal(L, "require");
		lua_pushvalue(L, -1);  /* function to be called */
		lua_pushstring(L, script);
		lua_call(L, 1, 1);
		LuaRef objReturnValue(L, -1);
		lua_pop(L, 2);  /* pop result and function */
		return objReturnValue;
	}

	void RemoveEntity(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return;
		
		UTIL_Remove(pEntity);
	}

	CFFTeam* GetTeam(int teamId)
	{
		if( teamId <= TEAM_INVALID )
			return NULL;
		if( teamId >= TEAM_COUNT )
			return NULL;

		return dynamic_cast<CFFTeam*>(g_Teams[teamId]);
	}

	CBaseEntity* GetEntity(int item_id)
	{
		return UTIL_EntityByIndex(item_id);
	}

	CBaseEntity* GetEntityByName(const char* szName)
	{
		return gEntList.FindEntityByName(NULL, szName, NULL);
	}

	LuaRef GetEntitiesByName(const char *szName)
	{
		LuaRef luatblEntities = newTable(_scriptman.GetLuaState());

		int iTableKey = 1;
		CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, szName );
		while( pEntity )
		{
			luatblEntities[iTableKey++] = LuaRef(_scriptman.GetLuaState(), pEntity);
			pEntity = gEntList.FindEntityByName( pEntity, szName );
		}

		return luatblEntities;
	}

	LuaRef GetEntitiesInSphere(const Vector& vecOrigin, float flRadius, bool bIgnoreWalls)
	{
		LuaRef luatblEntities = newTable(_scriptman.GetLuaState());

		int iTableKey = 1;
		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( vecOrigin, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( !pEntity )
				continue;

			if (!bIgnoreWalls)
			{
				trace_t tr;
				UTIL_TraceLine( vecOrigin, pEntity->GetAbsOrigin(), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr );

				if( FF_TraceHitWorld( &tr ) )
					continue;
			}

			luatblEntities[iTableKey++] = LuaRef(_scriptman.GetLuaState(), pEntity);
		}

		return luatblEntities;
	}

	CFFPlayer* GetPlayer(CBaseEntity *pEntity)
	{
		// ToFFPlayer checks for NULL & IsPlayer()
		// so this is safe to do.
		return ToFFPlayer( pEntity );
	}

	LuaRef GetPlayers()
	{
		LuaRef luatblPlayers = newTable(_scriptman.GetLuaState());

		int iTableKey = 1;
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			luatblPlayers[iTableKey++] = LuaRef(_scriptman.GetLuaState(), pPlayer);
		}

		return luatblPlayers;
	}

	CFFGrenadeBase *GetGrenade( int ent_id )
	{
		CBaseEntity *pEnt = GetEntity( ent_id );

		if( !pEnt )
			return NULL;

		if( !IsGrenade( pEnt ) )
			return NULL;

		return dynamic_cast< CFFGrenadeBase * >( pEnt );
	}

	bool IsEntity( const LuaRef &obj )
	{
		return obj.isValid() && obj.isUserdata() && obj.isInstance<CBaseEntity>();
	}

	bool IsPlayer( CBaseEntity *pEntity )
	{
		return GetPlayer( pEntity ) != NULL;
	}

	CFFInfoScript* GetInfoScriptByName(const char* entityName)
	{
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );

		while( pEnt != NULL )
		{
			if ( FStrEq( STRING(pEnt->GetEntityName()), entityName ) )
				return pEnt;

			// Next!
			pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( pEnt, CLASS_INFOSCRIPT );
		}

		return NULL;
	}

	CFuncFFScript *GetTriggerScriptByName( const char *pszEntityName )
	{
		CFuncFFScript *pEntity = ( CFuncFFScript * )gEntList.FindEntityByClassT( NULL, CLASS_TRIGGERSCRIPT );

		while( pEntity )
		{
			if( FStrEq( STRING( pEntity->GetEntityName() ), pszEntityName ) )
				return pEntity;

			pEntity = ( CFuncFFScript * )gEntList.FindEntityByClassT( pEntity, CLASS_TRIGGERSCRIPT );
		}

		return NULL;
	}

	CFFInfoScript* GetInfoScriptById(int item_id)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( item_id );
		if( pEntity && ( pEntity->Classify() == CLASS_INFOSCRIPT ) )
			return dynamic_cast< CFFInfoScript * >( pEntity );
		
		return NULL;
	}

	CFFPlayer* CastToPlayer( CBaseEntity* pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsPlayer( pEntity ) )
			return NULL;

		return dynamic_cast< CFFPlayer * >( pEntity );
	}

	CFFGrenadeBase *CastToGrenade( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsGrenade( pEntity ) )
			return NULL;

		return dynamic_cast< CFFGrenadeBase * >( pEntity );
	}

	CFFProjectileBase *CastToProjectile( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsProjectile( pEntity ) )
			return NULL;

		return dynamic_cast< CFFProjectileBase * >( pEntity );
	}

	CBeam* CastToBeam(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return NULL;

		if( !FStrEq( pEntity->GetClassname(), "env_beam") )
			return NULL;

		return dynamic_cast< CBeam * >( pEntity );
	}

	CFFInfoScript* CastToItemFlag(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return NULL;

		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return NULL;

		return dynamic_cast< CFFInfoScript * >( pEntity );
	}

	CFuncFFScript *CastToTriggerScript( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( pEntity->Classify() != CLASS_TRIGGERSCRIPT )
			return NULL;

		return dynamic_cast< CFuncFFScript * >( pEntity );
	}

	CFFTriggerClip *CastToTriggerClip( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( pEntity->Classify() != CLASS_TRIGGER_CLIP )
			return NULL;

		return dynamic_cast< CFFTriggerClip * >( pEntity );
	}

	CFFDispenser *CastToDispenser( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsDispenser( pEntity ) )
			return NULL;

		return dynamic_cast< CFFDispenser * >( pEntity );
	}

	CFFSentryGun *CastToSentrygun( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsSentrygun( pEntity ) )
			return NULL;

		return dynamic_cast< CFFSentryGun * >( pEntity );
	}

	CFFDetpack *CastToDetpack( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsDetpack( pEntity ) )
			return NULL;

		return dynamic_cast< CFFDetpack * >( pEntity );
	}

	CFFManCannon *CastToJumpPad( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsJumpPad( pEntity ) )
			return NULL;

		return dynamic_cast< CFFManCannon * >( pEntity );
	}

	CFFBuildableObject *CastToBuildable( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsBuildable( pEntity ) )
			return NULL;

		return dynamic_cast< CFFBuildableObject * >( pEntity );
	}

	bool AreTeamsAllied(CTeam* pTeam1, CTeam* pTeam2)
	{
		if(NULL == pTeam1 || NULL == pTeam2)
			return false;

		int iTeam1 = pTeam1->GetTeamNumber();
		int iTeam2 = pTeam2->GetTeamNumber();

		if( ( iTeam1 >= TEAM_BLUE ) && ( iTeam1 <= TEAM_GREEN ) &&
			( iTeam2 >= TEAM_BLUE ) && ( iTeam2 <= TEAM_GREEN ) )
		{
			if( FFGameRules()->IsTeam1AlliedToTeam2( iTeam1, iTeam2 ) == GR_TEAMMATE )
				return true;
		}

		return false;
	}

	bool AreTeamsAllied(int teamA, int teamB)
	{
		CFFTeam* pTeamA = GetGlobalFFTeam(teamA);
		CFFTeam* pTeamB = GetGlobalFFTeam(teamB);

		if( !pTeamA || !pTeamB )
			return false;

		return AreTeamsAllied(pTeamA, pTeamB);
	}

	int RandomInt(int min, int max)
	{
		return random->RandomInt(min, max);
	}

	float RandomFloat(float min, float max)
	{
		return random->RandomFloat(min, max);
	}

	void SmartClassLimits(unsigned int teamId, CClassLimits& limits )
	{
		// get team
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set team's class limits
		pTeam->SetClassLimit(CLASS_SCOUT, limits.scout);
		pTeam->SetClassLimit(CLASS_SNIPER, limits.sniper);
		pTeam->SetClassLimit(CLASS_SOLDIER, limits.soldier);
		pTeam->SetClassLimit(CLASS_DEMOMAN, limits.demoman);
		pTeam->SetClassLimit(CLASS_MEDIC, limits.medic);
		pTeam->SetClassLimit(CLASS_HWGUY, limits.hwguy);
		pTeam->SetClassLimit(CLASS_PYRO, limits.pyro);
		pTeam->SetClassLimit(CLASS_SPY, limits.spy);
		pTeam->SetClassLimit(CLASS_ENGINEER, limits.engineer);
		pTeam->SetClassLimit(CLASS_CIVILIAN, limits.civilian);
	}

	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg);

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg);

			else
				SendPlayerMessage(pTestPlayer, otherMsg);
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int iPlayerMsgColor, int iTeamMsgColor, int iOtherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, iPlayerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, iTeamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, iOtherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, const char* playerMsgColor, const char* teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int playerMsgColor, const char* teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int playerMsgColor, int teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int playerMsgColor, const char* teamMsgColor, int otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, const char* playerMsgColor, int teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, const char* playerMsgColor, int teamMsgColor, int otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}

	void SmartSound(CBaseEntity *pEntity, const char* playerSound, const char* teamSound, const char* otherSound)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerSound(pTestPlayer, playerSound);

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerSound(pTestPlayer, teamSound);

			else
				SendPlayerSound(pTestPlayer, otherSound);
		}
	}

	void SmartSpeak(CBaseEntity *pEntity, const char* playerSentence, const char* teamSentence, const char* otherSentence)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		int iPlayerSentence = engine->SentenceIndexFromName(playerSentence);
		int iTeamSentence = engine->SentenceIndexFromName(teamSentence);
		int iOtherSentence = engine->SentenceIndexFromName(otherSentence);

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SENTENCEG_PlaySentenceIndex(pTestPlayer->edict(), iPlayerSentence, 1.0f, SNDLVL_TALKING, 0, 100);

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SENTENCEG_PlaySentenceIndex(pTestPlayer->edict(), iTeamSentence, 1.0f, SNDLVL_TALKING, 0, 100);

			else
				SENTENCEG_PlaySentenceIndex(pTestPlayer->edict(), iOtherSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}
	}

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg);
			else
				SendPlayerMessage(pPlayer, otherMsg);
		}
	}

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, int iTeamMsgColor, int iOtherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, iTeamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, iOtherMsgColor);
		}
	}

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, const char* teamMsgColor, const char* otherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, teamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, otherMsgColor);
		}
	}
	
	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, int teamMsgColor, const char* otherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, teamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, otherMsgColor);
		}
	}
	
	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, const char* teamMsgColor, int otherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, teamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, otherMsgColor);
		}
	}

	void SmartTeamSpeak(CFFTeam *pTeam, const char* teamSentence, const char* otherSentence)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		int iTeamSentence = engine->SentenceIndexFromName(teamSentence);
		int iOtherSentence = engine->SentenceIndexFromName(otherSentence);

		// set the appropriate sound to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iTeamSentence, 1.0f, SNDLVL_TALKING, 0, 100);
			else
				SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iOtherSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}
	}

	void SmartTeamSound(CFFTeam *pTeam, const char* teamSound, const char* otherSound)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate sound to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerSound(pPlayer, teamSound);
			else
				SendPlayerSound(pPlayer, otherSound);
		}
	}

	// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name = "Generic_Lua_Event" )	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			gameeventmanager->FireEvent( pEvent, true );
		}
	}

			// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name,	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
		const char* field0,				// "Extra" fields
		const char* field1)				// "Extra" fields

	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			pEvent->SetString( "key0", field0 );
			pEvent->SetString( "value0", field1 );

			gameeventmanager->FireEvent( pEvent, true );
		}
	}

		// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name = "Generic_Lua_Event",	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
		const char* field0 = NULL,				// "Extra" fields
		const char* field1 = NULL,				// "Extra" fields
		const char* field2 = NULL,				// "Extra" fields
		const char* field3 = NULL )				// "Extra" fields

	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			pEvent->SetString( "key0", field0 );
			pEvent->SetString( "value0", field1 );
			pEvent->SetString( "key1", field2 );
			pEvent->SetString( "value1", field3 );

			gameeventmanager->FireEvent( pEvent, true );
		}
	}

	// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name = "Generic_Lua_Event",	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
		const char* field0 = NULL,				// "Extra" fields
		const char* field1 = NULL,				// "Extra" fields
		const char* field2 = NULL,				// "Extra" fields
		const char* field3 = NULL,				// "Extra" fields
		const char* field4 = NULL,				// "Extra" fields
		const char* field5 = NULL )

	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			pEvent->SetString( "key0", field0 );
			pEvent->SetString( "value0", field1 );
			pEvent->SetString( "key1", field2 );
			pEvent->SetString( "value1", field3 );
			pEvent->SetString( "key2", field4 );
			pEvent->SetString( "value2", field5 );

			gameeventmanager->FireEvent( pEvent, true );
		}
	}
	
	// For adding an objective-related death notice
	void ObjectiveNotice( CFFPlayer *player, const char *text )
	{
		if (!player)
			return;

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "objective_event" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", player->GetUserID() );
			//pEvent->SetString( "eventname", name );
			pEvent->SetString( "eventtext", text );

			// pass on to lua to do "stuff"
			bool bAllowedLUA = true;
			
			CFFLuaSC hContext( 0 );
			hContext.Push( pEvent );
			
			if ( _scriptman.RunPredicates_LUA( NULL, &hContext, "ondeathnotice" ) )
			{
				bAllowedLUA = hContext.GetBool();
			}
			
			if ( bAllowedLUA )
			{
				gameeventmanager->FireEvent( pEvent );
			}
		}
	}

	void SendHintToPlayer( CFFPlayer *pPlayer, const char* message )
	{
		if ( pPlayer )
			FF_SendHint( pPlayer, MAP_HINT, -1, PRIORITY_HIGH, message );
	}

	void SendHintToTeam( CFFTeam *pTeam, const char* message )
	{
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CFFPlayer *pPlayer = GetPlayer(UTIL_EntityByIndex(i));
			if ( pPlayer && pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
				SendHintToPlayer( pPlayer, message );
		}
	}

	void SendHintToAll( const char* message )
	{
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			SendHintToPlayer( GetPlayer(UTIL_EntityByIndex(i)), message );
		}
	}

	// Updates which entity the HUD objective icon is attached to
	// Unfortunately, it seems certain entities (like trigger_scripts) do not network properly (they have invalid indexes)
	// So, we also have to network the entity's position as a fallback
	void UpdateObjectiveIcon( CFFPlayer *pPlayer, CBaseEntity *pEntity )
	{
		if ( pPlayer )
		{
			pPlayer->SetObjectiveEntity( pEntity );
		}
	}

	// Updates the position of the Objective Icon (the entity it's attached to) for a whole team
	void UpdateTeamObjectiveIcon( CFFTeam *pTeam, CBaseEntity *pEntity )
	{
		if( !pTeam )
			return;

		// set the objective entity for each player on the team
		for( int i = 1 ; i <= gpGlobals->maxClients; i++ )
		{
			CFFPlayer* pPlayer = GetPlayer( UTIL_EntityByIndex(i) );

			if( !pPlayer )
				continue;

			if( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
			{
				pPlayer->SetObjectiveEntity( pEntity );
			}
		}
	}

	CFFPlayer *GetPlayerByID( int player_id )
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( player_id );
		return GetPlayer( pEntity );
	}

	CFFPlayer *GetPlayerByName( const char *_name )
	{
		CBaseEntity *pEntity = UTIL_PlayerByName( _name );
		return GetPlayer( pEntity );
	}
	
	CFFPlayer *GetPlayerBySteamID( const char *steamid )
	{
		CBaseEntity *pEntity = UTIL_PlayerBySteamID( steamid );
		return GetPlayer( pEntity );
	}

	void SetPlayerLimits(CPlayerLimits& limits)
	{
		CFFTeam* pTeam = GetTeam(TEAM_BLUE);
		pTeam->SetTeamLimits(limits.blue);

		pTeam = GetTeam(TEAM_RED);
		pTeam->SetTeamLimits(limits.red);

		pTeam = GetTeam(TEAM_YELLOW);
		pTeam->SetTeamLimits(limits.yellow);

		pTeam = GetTeam(TEAM_GREEN);
		pTeam->SetTeamLimits(limits.green);
	}

	void SetPlayerLimit(int teamId, int limit)
	{
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL != pTeam)
			pTeam->SetTeamLimits(limit);
	}

	void SetTeamName(int teamId, const char* szTeamName)
	{
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL != pTeam)
			pTeam->SetName(szTeamName);
	}

	void SetTeamClassLimit(int teamId, int classId, int limit)
	{
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL != pTeam)
			pTeam->SetClassLimit(classId, limit);
	}

	float GetServerTime( void )
	{
		return gpGlobals->curtime;
	}

	void AddPlayerSpeedeffect( CFFPlayer *pPlayer, float flDuration, float flPercent )
	{
	}

	float GetConvar( const char *pszConvarName )
	{
		if( !pszConvarName )
			return -1.0f;

		if( !cvar )
			return -1.0f;

		ConVar *pConvar = cvar->FindVar( pszConvarName );
		if( !pConvar )
			return -1.0f;

		if( pConvar->IsFlagSet( FCVAR_CHEAT ) )
			return -1.0f;

		return pConvar->GetFloat();
	}

	void SetConvar( const char *pszConvarName, float flValue )
	{
		if( !pszConvarName )
			return;

		// Don't allow sv_cheats setting
		if( !Q_stricmp( pszConvarName, "sv_cheats" ) )
			return;

		// protect rcon_password
		if ( Q_stricmp( pszConvarName, "rcon_password" ) == 0 )
			return;

		if( !cvar )
			return;

		ConVar *pConvar = cvar->FindVar( pszConvarName );
		if( !pConvar )
			return;

		if( pConvar->IsFlagSet( FCVAR_CHEAT ) )
			return;

		static char		string[1024];
		Q_snprintf (string, sizeof( string ), "%s %f\n", pszConvarName, flValue);

		engine->ServerCommand( string );
	}
	
	void SetConvar( const char *pszConvarName, const char *pszValue )
	{
		if( !pszConvarName )
			return;

		// Don't allow sv_cheats setting
		if( !Q_stricmp( pszConvarName, "sv_cheats" ) )
			return;

		// protect rcon_password
		if ( Q_stricmp( pszConvarName, "rcon_password" ) == 0 )
			return;

		if( !cvar )
			return;

		ConVar *pConvar = cvar->FindVar( pszConvarName );
		if( !pConvar )
			return;

		if( pConvar->IsFlagSet( FCVAR_CHEAT ) )
			return;

		static char		string[1024];
		Q_snprintf (string, sizeof( string ), "%s %s\n", pszConvarName, pszValue);

		engine->ServerCommand( string );
	}

	const char *GetSteamID( CFFPlayer *pPlayer )
	{
		if( pPlayer )
			return pPlayer->GetSteamID();

		return "\0";
	}

	int GetPing( CFFPlayer *pPlayer )
	{
		if( pPlayer )
			return pPlayer->GetPing();

		return 0;
	}

	int GetPacketloss( CFFPlayer *pPlayer )
	{
		if( pPlayer )
			return pPlayer->GetPacketloss();

		return 0;
	}

	const char *PrintBool( bool bValue )
	{
		return bValue ? "True" : "False";
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s",
				   szTargetEntityName,
				   szTargetInputName);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL);

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName,
					const char* szParameter)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s,%s",
				   szTargetEntityName,
				   szTargetInputName,
				   szParameter);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL);

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName,
					const char* szParameter,
					float delay)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s,%s,%f",
				   szTargetEntityName,
				   szTargetInputName,
				   szParameter,
				   delay);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL, delay);

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName,
					const char* szParameter,
					float delay,
					unsigned int nRepeat)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s,%s,%f,%d",
				   szTargetEntityName,
				   szTargetInputName,
				   szParameter,
				   delay,
				   nRepeat);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL, delay);

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
	}

	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, float x, float y )
	{
		if( !pPlayer || !pszImage || !pszIdentifier )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, 0, 0 );
	}

	// default alignment
	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight )
	{
		if( !pPlayer || !pszImage || !pszIdentifier )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight );
	}

	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, int iAlign )
	{
		if( !pPlayer || !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight, iAlign );
	}

	// added y alignment
	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, float flAlignX, float flAlignY )
	{
		if( !pPlayer || !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight, flAlignX, flAlignY );
	}

	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, float x, float y )
	{
		if( !pszImage || !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, 0, 0);
			}
		}
	}

	// default alignment
	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight )
	{
		if( !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight);
			}
		}
	}

	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, int iAlign )
	{
		if( !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight, iAlign);
			}
		}
	}

	// added y alignment
	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, float flAlignX, float flAlignY )
	{
		if( !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight, flAlignX, flAlignY);
			}
		}
	}

	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, float x, float y )
	{
		if( !pszImage || !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, 0, 0);
			}
		}
	}

	// default alignment
	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight )
	{
		if( !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight);
			}
		}
	}

	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, int iAlign )
	{
		if( !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight, iAlign);
			}
		}
	}

	// added y alignment
	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, float flAlignX, float flAlignY )
	{
		if( !pszImage || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, flWidth, flHeight, flAlignX, flAlignY);
			}
		}
	}

	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border, float flAlignX, float flAlignY )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, border.clr, border.width, flAlignX, flAlignY );
	}
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border, float flAlignX )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, border, flAlignX, -1 );
	}
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, border, -1, -1 );
	}

	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), -1, -1 );
	}
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, float flAlignX )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), flAlignX, -1 );
	}
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, float flAlignX, float flAlignY )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), flAlignX, flAlignY );
	}
	
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr), -1, -1 );
	}
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr, float flAlignX )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr), flAlignX, -1 );
	}
	void AddHudBox( CFFPlayer *pPlayer, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr, float flAlignX, float flAlignY )
	{
		AddHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr), flAlignX, flAlignY );
	}

	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border, float flAlignX, float flAlignY )
	{
		if( !pTeam || !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, border.clr, border.width, flAlignX, flAlignY );
			}
		}
	}
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border, float flAlignX )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, border, flAlignX, -1 );
	}
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, border, -1, -1 );
	}

	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), -1, -1 );
	}
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, float flAlignX )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), flAlignX, -1 );
	}
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, float flAlignX, float flAlignY )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), flAlignX, flAlignY );
	}
	
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr), -1, -1 );
	}
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr, float flAlignX )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr), flAlignX, -1 );
	}
	void AddHudBoxToTeam( CFFTeam *pTeam, const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr, float flAlignX, float flAlignY )
	{
		AddHudBoxToTeam( pTeam, pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr), flAlignX, flAlignY );
	}

	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier || ( flWidth < 0 ) || ( flHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudBox( pPlayer, pszIdentifier, x, y, flWidth, flHeight, clr, border.clr, border.width, flAlignX, flAlignY );
			}
		}
	}
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border, float flAlignX )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, border, flAlignX, -1 );
	}
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, CHudBoxBorder border )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, border, -1, -1 );
	}

	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), -1, -1 );
	}
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, float flAlignX )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), flAlignX, -1 );
	}
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, float flAlignX, float flAlignY )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(), flAlignX, flAlignY );
	}
	
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr, 1), -1, -1 );
	}
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr, float flAlignX )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr, 1), flAlignX, -1 );
	}
	void AddHudBoxToAll( const char *pszIdentifier, float x, float y, float flWidth, float flHeight, Color clr, Color borderclr, float flAlignX, float flAlignY )
	{
		AddHudBoxToAll( pszIdentifier, x, y, flWidth, flHeight, clr, CHudBoxBorder(borderclr, 1), flAlignX, flAlignY );
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, float x, float y )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, float x, float y, int iAlign )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlign );
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, flAlignX, flAlignY );
	}
	
	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, flAlignX, flAlignY, iSize );
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, float x, float y )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
			}
		}
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, float x, float y, int iAlign )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlign );
			}
		}
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, flAlignX, flAlignY );
			}
		}
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, flAlignX, flAlignY, iSize );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, float x, float y )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, float x, float y, int iAlign )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlign );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, flAlignX, flAlignY );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, flAlignX, flAlignY, iSize );
			}
		}
	}

	void AddColoredHudText(CFFPlayer* pPlayer, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y)
	{
		if (!pPlayer || !pszIdentifier || !pszText)
			return;

		FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText);
	}

	void AddColoredHudText(CFFPlayer* pPlayer, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, int iAlign)
	{
		if (!pPlayer || !pszIdentifier || !pszText)
			return;

		FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, iAlign);
	}

	void AddColoredHudText(CFFPlayer* pPlayer, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, float flAlignX, float flAlignY)
	{
		if (!pPlayer || !pszIdentifier || !pszText)
			return;

		FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, flAlignX, flAlignY);
	}

	void AddColoredHudText(CFFPlayer* pPlayer, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, float flAlignX, float flAlignY, int iSize)
	{
		if (!pPlayer || !pszIdentifier || !pszText)
			return;

		FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, flAlignX, flAlignY, iSize);
	}

	void AddColoredHudTextToTeam(CFFTeam* pTeam, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				if (pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
					FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText);
			}
		}
	}

	void AddColoredHudTextToTeam(CFFTeam* pTeam, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, int iAlign)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				if (pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
					FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, iAlign);
			}
		}
	}

	void AddColoredHudTextToTeam(CFFTeam* pTeam, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, float flAlignX, float flAlignY)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				if (pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
					FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, flAlignX, flAlignY);
			}
		}
	}

	void AddColoredHudTextToTeam(CFFTeam* pTeam, const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, float flAlignX, float flAlignY, int iSize)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				if (pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
					FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, flAlignX, flAlignY, iSize);
			}
		}
	}

	void AddColoredHudTextToAll(const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText);
			}
		}
	}

	void AddColoredHudTextToAll(const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, int iAlign)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, iAlign);
			}
		}
	}

	void AddColoredHudTextToAll(const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, float flAlignX, float flAlignY)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, flAlignX, flAlignY);
			}
		}
	}

	void AddColoredHudTextToAll(const char* pszIdentifier, const char* pszText, const char* pszColor, float x, float y, float flAlignX, float flAlignY, int iSize)
	{
		if (!pszIdentifier || !pszText)
			return;

		// loop through each player
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* ent = UTIL_PlayerByIndex(i);
			if (ent && ent->IsPlayer())
			{
				CFFPlayer* pPlayer = ToFFPlayer(ent);
				FF_LuaHudTextColored(pPlayer, pszIdentifier, pszColor, x, y, pszText, flAlignX, flAlignY, iSize);
			}
		}
	}
	
	void AddTimer( const char *pszIdentifier, float flStartValue, float flSpeed )
	{
		if( !pszIdentifier )
			return;

		_timerman.AddTimer( pszIdentifier, flStartValue, flSpeed );
	}
	
	void RemoveTimer( const char *pszIdentifier )
	{
		if( !pszIdentifier )
			return;

		_timerman.RemoveTimer( pszIdentifier );
	}

	float GetTimerTime( const char *pszIdentifier )
	{
		if( !pszIdentifier )
			return 0;

		return _timerman.GetTime( pszIdentifier );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ) );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, int iAlign )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlign );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), flAlignX, flAlignY );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), flAlignX, flAlignY, iSize );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, int iAlign )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, iAlign );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, flAlignX, flAlignY );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, flAlignX, flAlignY, iSize );
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ) );
			}
		}
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlign );
			}
		}
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), flAlignX, flAlignY );
			}
		}
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), flAlignX, flAlignY, iSize );
			}
		}
	}

	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed );
			}
		}
	}

	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, iAlign );
			}
		}
	}

	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, flAlignX, flAlignY );
			}
		}
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, flAlignX, flAlignY, iSize );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ) );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlign );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), flAlignX, flAlignY );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), flAlignX, flAlignY, iSize );
			}
		}
	}

	void AddHudTimerToAll( const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed );
			}
		}
	}

	void AddHudTimerToAll( const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, iAlign );
			}
		}
	}

	void AddHudTimerToAll( const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, float flAlignX, float flAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, flAlignX, flAlignY );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, float flStartValue, float flSpeed, float x, float y, float flAlignX, float flAlignY, int iSize )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, flStartValue, flSpeed, flAlignX, flAlignY, iSize );
			}
		}
	}

	void RemoveHudItem( CFFPlayer *pPlayer, const char *pszIdentifier )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudRemove( pPlayer, pszIdentifier );
	}

	void RemoveHudItemFromTeam( CFFTeam *pTeam, const char *pszIdentifier )
	{
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudRemove( pPlayer, pszIdentifier );
			}
		}
	}

	void RemoveHudItemFromAll( const char *pszIdentifier )
	{
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudRemove( pPlayer, pszIdentifier );
			}
		}
	}
	
	void AddSchedule(const char* szScheduleName, float time, const LuaRef& fn)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn);
	}

	void AddSchedule(const char* szScheduleName,
					 float time,
					 const LuaRef& fn,
					 const LuaRef& param)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param);
	}

	void AddSchedule(const char* szScheduleName,
					 float time,
					 const LuaRef& fn,
					 const LuaRef& param1,
					 const LuaRef& param2)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param1, param2);
	}

	void AddSchedule(const char* szScheduleName,
					float time,
					const LuaRef& fn,
					const LuaRef& param1,
					const LuaRef& param2,
					const LuaRef& param3)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param1, param2, param3);
	}

	void AddSchedule(const char* szScheduleName,
					float time,
					const LuaRef& fn,
					const LuaRef& param1,
					const LuaRef& param2,
					const LuaRef& param3,
					const LuaRef& param4)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param1, param2, param3, param4);
	}

	void AddScheduleRepeating(const char* szScheduleName, float time, const LuaRef& fn)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							  float time,
							  const LuaRef& fn,
							  const LuaRef& param)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							  float time,
							  const LuaRef& fn,
							  const LuaRef& param1,
							  const LuaRef& param2)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param1, param2);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							float time,
							const LuaRef& fn,
							const LuaRef& param1,
							const LuaRef& param2,
							const LuaRef& param3)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param1, param2, param3);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							float time,
							const LuaRef& fn,
							const LuaRef& param1,
							const LuaRef& param2,
							const LuaRef& param3,
							const LuaRef& param4)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param1, param2, param3, param4);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName, float time, const LuaRef& fn, int nRepeat)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const LuaRef& fn,
											int nRepeat,
											const LuaRef& param)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const LuaRef& fn,
											int nRepeat,
											const LuaRef& param1,
											const LuaRef& param2)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param1, param2);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const LuaRef& fn,
											int nRepeat,
											const LuaRef& param1,
											const LuaRef& param2,
											const LuaRef& param3)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param1, param2, param3);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const LuaRef& fn,
											int nRepeat,
											const LuaRef& param1,
											const LuaRef& param2,
											const LuaRef& param3,
											const LuaRef& param4)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param1, param2, param3, param4);
	}

	void DeleteSchedule(const char* szScheduleName)
	{
		_scheduleman.RemoveSchedule(szScheduleName);
	}

	void RemoveSchedule(const char* szScheduleName)
	{
		_scheduleman.RemoveSchedule(szScheduleName);
	}

	void SpeakAll(const char* szSentenceName)
	{
		// lookup the id of the sentence
		if(!szSentenceName)
			return;

		int iSentence = engine->SentenceIndexFromName(szSentenceName);

		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("speak: %s", szSentenceName),"speak_all");
	}

	void SpeakPlayer(CFFPlayer *pPlayer, const char* szSentenceName)
	{
		// lookup the id of the sentence
		if(!pPlayer || !szSentenceName)
			return;

		int iSentence = engine->SentenceIndexFromName(szSentenceName);

		// send the sentence to the client
		SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iSentence, 1.0f, SNDLVL_TALKING, 0, 100);
	}

	void SpeakTeam(int iTeam, const char* szSentenceName)
	{
		// lookup the id of the sentence
		if(!szSentenceName)
			return;

		int iSentence = engine->SentenceIndexFromName(szSentenceName);

		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeamNumber() == iTeam)
				SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}
	}

	void DropToFloor( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return;

		UTIL_DropToFloor( pEntity, MASK_SOLID );
	}
	
	void SetGameDescription( const char *szGameDescription )
	{
		if ( FFGameRules() ) // this function may be called before the world has spawned, and the game rules initialized
			FFGameRules()->SetGameDescription( szGameDescription );
	}
	
	const char* GetGameDescription()
	{
		if (FFGameRules()) // this function may be called before the world has spawned, and the game rules initialized
			return FFGameRules()->GetGameDescription();
		else
			return "";
	}

	void ShowMenuToPlayer( CFFPlayer *pPlayer, const char *szMenuName )
	{
		if (!pPlayer)
			return;

		CSingleUserRecipientFilter filter( pPlayer );
		_menuman.DisplayLuaMenu( filter, szMenuName );
	}

	void ShowMenuToTeam( int iTeam, const char *szMenuName )
	{
		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;
			
			if(pPlayer->GetTeamNumber() == iTeam)
				ShowMenuToPlayer( pPlayer, szMenuName );
		}
	}

	void ShowMenu( const char *szMenuName )
	{
		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			ShowMenuToPlayer( pPlayer, szMenuName );
		}
	}
	
	void CreateMenu( const char *szMenuName )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenu( szMenuName );
	}
	
	void CreateMenu( const char *szMenuName, float flDisplayTime )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenu( szMenuName, flDisplayTime );
	}
	
	void CreateMenu( const char *szMenuName, const char *szMenuTitle )
	{
		if (!szMenuName[0] || !szMenuTitle[0])
			return;

		_menuman.AddLuaMenu( szMenuName, szMenuTitle );
	}
	
	void CreateMenu( const char *szMenuName, const char *szMenuTitle, float flDisplayTime )
	{
		if (!szMenuName[0] || !szMenuTitle[0])
			return;

		_menuman.AddLuaMenu( szMenuName, szMenuTitle, flDisplayTime );
	}
	
	void DestroyMenu( const char *szMenuName )
	{
		if (!szMenuName[0])
			return;

		_menuman.RemoveLuaMenu( szMenuName );
	}
	
	void SetMenuTitle( const char *szMenuName, const char *szMenuTitle )
	{
		if (!szMenuName[0] || !szMenuTitle[0])
			return;

		_menuman.SetLuaMenuTitle( szMenuName, szMenuTitle );
	}
	
	void AddMenuOption( const char *szMenuName, int iSlot, const char *szOptionText )
	{
		if (!szMenuName[0] || !szOptionText[0])
			return;

		_menuman.AddLuaMenuOption( szMenuName, iSlot, szOptionText );
	}

	void RemoveMenuOption( const char *szMenuName, int iSlot )
	{
		if (!szMenuName[0])
			return;

		_menuman.RemoveLuaMenuOption( szMenuName, iSlot );
	}

	CBaseEntity* SpawnEntity(const char *szEntityClassName, const char *szEntityName)
	{
		CBaseEntity *pEntity = CreateEntityByName(szEntityClassName);
		if (szEntityName && pEntity)
		{
			pEntity->SetName( MAKE_STRING(szEntityName) );
		}
		int status = DispatchSpawn(pEntity);
		return status == 0 ? pEntity : NULL;
	}

	CBaseEntity* SpawnEntity(const char *szEntityClassName)
	{
		return SpawnEntity(szEntityClassName, NULL);
	}

	const char* GetMapName()
	{
		return STRING(gpGlobals->mapname);
	}

	int FFPrecacheModel(const char* name)
	{
		return CBaseEntity::PrecacheModel(name, true);
	}
} // namespace FFLib

//---------------------------------------------------------------------------
// FFLib Namespace
//---------------------------------------------------------------------------
void CFFLuaLib::InitGlobals(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)

		.beginClass<CClassLimits>("ClassLimits")
			.addConstructor<void()>()
			.addProperty("Scout",		&CClassLimits::scout)
			.addProperty("Sniper",		&CClassLimits::sniper)
			.addProperty("Soldier",		&CClassLimits::soldier)
			.addProperty("Demoman",		&CClassLimits::demoman)
			.addProperty("Medic",		&CClassLimits::medic)
			.addProperty("Hwguy",		&CClassLimits::hwguy)
			.addProperty("Pyro",		&CClassLimits::pyro)
			.addProperty("Engineer",	&CClassLimits::engineer)
			.addProperty("Spy",			&CClassLimits::spy)
			.addProperty("Civilian",	&CClassLimits::civilian)
		.endClass()

		.beginClass<CPlayerLimits>("PlayerLimits")
			.addConstructor<void()>()
			.addProperty("Blue",		&CPlayerLimits::blue)
			.addProperty("Red",			&CPlayerLimits::red)
			.addProperty("Yellow",		&CPlayerLimits::yellow)
			.addProperty("Green",		&CPlayerLimits::green)
		.endClass()
			
		.beginClass<CHudBoxBorder>("CustomBorder")
			.addConstructor<void(),
							void(Color),
							void(Color, int)
							>()
			.addProperty("clr",			&CHudBoxBorder::clr)
			.addProperty("width",		&CHudBoxBorder::width)
		.endClass()

		// global functions
		.addFunction("AddHudIcon",
			overload<CFFPlayer*, const char*, const char*, float, float>(&FFLib::AddHudIcon),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float>(&FFLib::AddHudIcon),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float, int>(&FFLib::AddHudIcon),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float, float, float>(&FFLib::AddHudIcon)
		)

		.addFunction("AddHudIconToTeam",
			overload<CFFTeam*, const char*, const char*, float, float>(&FFLib::AddHudIconToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float>(&FFLib::AddHudIconToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float, int>(&FFLib::AddHudIconToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float, float, float>(&FFLib::AddHudIconToTeam)
		)

		.addFunction("AddHudIconToAll",
			overload<const char*, const char*, float, float>(&FFLib::AddHudIconToAll),
			overload<const char*, const char*, float, float, float, float>(&FFLib::AddHudIconToAll),
			overload<const char*, const char*, float, float, float, float, int>(&FFLib::AddHudIconToAll),
			overload<const char*, const char*, float, float, float, float, float, float>(&FFLib::AddHudIconToAll)
		)

		.addFunction("AddHudBox",
			overload<CFFPlayer*, const char*, float, float, float, float, Color>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, float>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, float, float>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, Color>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, Color, float>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, Color, float, float>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, CHudBoxBorder>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, CHudBoxBorder, float>(&FFLib::AddHudBox),
			overload<CFFPlayer*, const char*, float, float, float, float, Color, CHudBoxBorder, float, float>(&FFLib::AddHudBox)
		)

		.addFunction("AddHudBoxToTeam",
			overload<CFFTeam*, const char*, float, float, float, float, Color>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, float>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, float, float>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, Color>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, Color, float>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, Color, float, float>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, CHudBoxBorder>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, CHudBoxBorder, float>(&FFLib::AddHudBoxToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, Color, CHudBoxBorder, float, float>(&FFLib::AddHudBoxToTeam)
		)

		.addFunction("AddHudBoxToAll",
			overload<const char*, float, float, float, float, Color>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, float>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, float, float>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, Color>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, Color, float>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, Color, float, float>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, CHudBoxBorder>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, CHudBoxBorder, float>(&FFLib::AddHudBoxToAll),
			overload<const char*, float, float, float, float, Color, CHudBoxBorder, float, float>(&FFLib::AddHudBoxToAll)
		)

		.addFunction("AddHudText",
			overload<CFFPlayer*, const char*, const char*, float, float>(&FFLib::AddHudText),
			overload<CFFPlayer*, const char*, const char*, float, float, int>(&FFLib::AddHudText),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float>(&FFLib::AddHudText),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float, int>(&FFLib::AddHudText)
		)

		.addFunction("AddHudTextToTeam",
			overload<CFFTeam*, const char*, const char*, float, float>(&FFLib::AddHudTextToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, int>(&FFLib::AddHudTextToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float>(&FFLib::AddHudTextToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float, int>(&FFLib::AddHudTextToTeam)
		)

		.addFunction("AddHudTextToAll",
			overload<const char*, const char*, float, float>(&FFLib::AddHudTextToAll),
			overload<const char*, const char*, float, float, int>(&FFLib::AddHudTextToAll),
			overload<const char*, const char*, float, float, float, float>(&FFLib::AddHudTextToAll),
			overload<const char*, const char*, float, float, float, float, int>(&FFLib::AddHudTextToAll)
		)

		.addFunction("AddColoredHudText",
			overload<CFFPlayer*, const char*, const char*, const char*, float, float>(&FFLib::AddColoredHudText),
			overload<CFFPlayer*, const char*, const char*, const char*, float, float, int>(&FFLib::AddColoredHudText),
			overload<CFFPlayer*, const char*, const char*, const char*, float, float, float, float>(&FFLib::AddColoredHudText),
			overload<CFFPlayer*, const char*, const char*, const char*, float, float, float, float, int>(&FFLib::AddColoredHudText)
		)

		.addFunction("AddColoredHudTextToTeam",
			overload<CFFTeam*, const char*, const char*, const char*, float, float>(&FFLib::AddColoredHudTextToTeam),
			overload<CFFTeam*, const char*, const char*, const char*, float, float, int>(&FFLib::AddColoredHudTextToTeam),
			overload<CFFTeam*, const char*, const char*, const char*, float, float, float, float>(&FFLib::AddColoredHudTextToTeam),
			overload<CFFTeam*, const char*, const char*, const char*, float, float, float, float, int>(&FFLib::AddColoredHudTextToTeam)
		)

		.addFunction("AddColoredHudTextToAll",
			overload<const char*, const char*, const char*, float, float>(&FFLib::AddColoredHudTextToAll),
			overload<const char*, const char*, const char*, float, float, int>(&FFLib::AddColoredHudTextToAll),
			overload<const char*, const char*, const char*, float, float, float, float>(&FFLib::AddColoredHudTextToAll),
			overload<const char*, const char*, const char*, float, float, float, float, int>(&FFLib::AddColoredHudTextToAll)
		)

		.addFunction("AddTimer",				&FFLib::AddTimer)
		.addFunction("RemoveTimer",				&FFLib::RemoveTimer)
		.addFunction("GetTimerTime",			&FFLib::GetTimerTime)

		.addFunction("AddHudTimer",
			overload<CFFPlayer*, const char*, const char*, float, float>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, const char*, float, float, int>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, const char*, float, float, float, float, int>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, float, float, float, float>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, float, float, float, float, int>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, float, float, float, float, float, float>(&FFLib::AddHudTimer),
			overload<CFFPlayer*, const char*, float, float, float, float, float, float, int>(&FFLib::AddHudTimer)
		)
		.addFunction("AddHudTimerToTeam",
			overload<CFFTeam*, const char*, const char*, float, float>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, int>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, const char*, float, float, float, float, int>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, float, float, float, float>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, int>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, float, float>(&FFLib::AddHudTimerToTeam),
			overload<CFFTeam*, const char*, float, float, float, float, float, float, int>(&FFLib::AddHudTimerToTeam)
		)
		.addFunction("AddHudTimerToAll",
			overload<const char*, const char*, float, float>(&FFLib::AddHudTimerToAll),
			overload<const char*, const char*, float, float, int>(&FFLib::AddHudTimerToAll),
			overload<const char*, const char*, float, float, float, float>(&FFLib::AddHudTimerToAll),
			overload<const char*, const char*, float, float, float, float, int>(&FFLib::AddHudTimerToAll),
			overload<const char*, float, float, float, float>(&FFLib::AddHudTimerToAll),
			overload<const char*, float, float, float, float, int>(&FFLib::AddHudTimerToAll),
			overload<const char*, float, float, float, float, float, float>(&FFLib::AddHudTimerToAll),
			overload<const char*, float, float, float, float, float, float, int>(&FFLib::AddHudTimerToAll)
		)
		.addFunction("AddSchedule",
			overload<const char*, float, const LuaRef&>(&FFLib::AddSchedule),
			overload<const char*, float, const LuaRef&, const LuaRef&>(&FFLib::AddSchedule),
			overload<const char*, float, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddSchedule),
			overload<const char*, float, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddSchedule),
			overload<const char*, float, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddSchedule)
		)
		.addFunction("AddScheduleRepeating",
			overload<const char*, float, const LuaRef&>(&FFLib::AddScheduleRepeating),
			overload<const char*, float, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeating),
			overload<const char*, float, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeating),
			overload<const char*, float, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeating),
			overload<const char*, float, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeating)
		)
		.addFunction("AddScheduleRepeatingNotInfinitely",
			overload<const char*, float, const LuaRef&, int>(&FFLib::AddScheduleRepeatingNotInfinitely),
			overload<const char*, float, const LuaRef&, int, const LuaRef&>(&FFLib::AddScheduleRepeatingNotInfinitely),
			overload<const char*, float, const LuaRef&, int, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeatingNotInfinitely),
			overload<const char*, float, const LuaRef&, int, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeatingNotInfinitely),
			overload<const char*, float, const LuaRef&, int, const LuaRef&, const LuaRef&, const LuaRef&, const LuaRef&>(&FFLib::AddScheduleRepeatingNotInfinitely)
		)
		.addFunction("ApplyToAll",				&FFLib::ApplyToAll)
		.addFunction("ApplyToTeam",				&FFLib::ApplyToTeam)
		.addFunction("ApplyToPlayer",			&FFLib::ApplyToPlayer)

		.addFunction("AreTeamsAllied",
			overload<CTeam*, CTeam*>(&FFLib::AreTeamsAllied),
			overload<int, int>(&FFLib::AreTeamsAllied)
		)

		.addFunction("ChatToAll",				&FFLib::ChatToAll)
		.addFunction("ChatToPlayer",			&FFLib::ChatToPlayer)

		.addFunction("BroadCastMessage",
			overload<const char*>(&FFLib::BroadcastMessage),
			overload<const char*, float>(&FFLib::BroadcastMessage),
			overload<const char*, float, int>(&FFLib::BroadcastMessage),
			overload<const char*, float, const char*>(&FFLib::BroadcastMessage)
		)

		.addFunction("BroadCastMessageToPlayer",
			overload<CFFPlayer*, const char*>(&FFLib::SendPlayerMessage),
			overload<CFFPlayer*, const char*, float>(&FFLib::SendPlayerMessage),
			overload<CFFPlayer*, const char*, float, int>(&FFLib::SendPlayerMessage),
			overload<CFFPlayer*, const char*, float, const char*>(&FFLib::SendPlayerMessage)
		)

		.addFunction("BroadCastSound",			&FFLib::BroadcastSound)
		.addFunction("BroadCastSoundToPlayer",	&FFLib::SendPlayerSound)
		.addFunction("CastToBeam",				&FFLib::CastToBeam)
		.addFunction("CastToPlayer",			&FFLib::CastToPlayer)
		.addFunction("CastToInfoScript",		&FFLib::CastToItemFlag)
		.addFunction("CastToTriggerScript",		&FFLib::CastToTriggerScript)
		.addFunction("CastToTriggerClip",		&FFLib::CastToTriggerClip)
		.addFunction("CastToGrenade",			&FFLib::CastToGrenade)
		.addFunction("CastToDispenser",			&FFLib::CastToDispenser)
		.addFunction("CastToSentrygun",			&FFLib::CastToSentrygun)
		.addFunction("CastToDetpack",			&FFLib::CastToDetpack)
		.addFunction("CastToJumpPad",			&FFLib::CastToJumpPad)
		.addFunction("CastToBuildable",			&FFLib::CastToBuildable)
		.addFunction("CastToProjectile",		&FFLib::CastToProjectile)
		.addFunction("ConsoleToAll",			&FFLib::ConsoleToAll)
		.addFunction("DeleteSchedule",			&FFLib::DeleteSchedule)
		.addFunction("DropToFloor",				&FFLib::DropToFloor)
		.addFunction("RemoveSchedule",			&FFLib::RemoveSchedule)
		.addFunction("GetConvar",				&FFLib::GetConvar)
		.addFunction("GetEntity",				&FFLib::GetEntity)
		.addFunction("GetEntityByName",			&FFLib::GetEntityByName)
		.addFunction("GetEntitiesByName",		&FFLib::GetEntitiesByName)
		.addFunction("GetEntitiesInSphere",		&FFLib::GetEntitiesInSphere)
		.addFunction("GetInfoScriptById",		&FFLib::GetInfoScriptById)
		.addFunction("GetInfoScriptByName",		&FFLib::GetInfoScriptByName)
		.addFunction("GetGrenade",				&FFLib::GetGrenade)
		.addFunction("GetPacketloss",			&FFLib::GetPacketloss)
		.addFunction("GetPing",					&FFLib::GetPing)
		.addFunction("GetPlayer",				&FFLib::GetPlayer)
		.addFunction("GetPlayerByID",			&FFLib::GetPlayerByID)	// TEMPORARY
		.addFunction("GetPlayerByName",			&FFLib::GetPlayerByName)
		.addFunction("GetPlayerBySteamID",		&FFLib::GetPlayerBySteamID)
		.addFunction("GetServerTime",			&FFLib::GetServerTime)
		.addFunction("GetSteamID",				&FFLib::GetSteamID)
		.addFunction("GetTeam",					&FFLib::GetTeam)
		.addFunction("GetTriggerScriptByName",	&FFLib::GetTriggerScriptByName)

		.addFunction("DisableTimeLimit",
			overload<>(&FFLib::DisableTimeLimit),
			overload<bool>(&FFLib::DisableTimeLimit)
		)

		.addFunction("HasGameStarted",			&FFLib::HasGameStarted)
		.addFunction("ServerCommand",			&FFLib::ServerCommand)
		.addFunction("GoToIntermission",		&FFLib::GoToIntermission)
		.addFunction("IncludeScript",			&FFLib::IncludeScript)
		.addFunction("IsEntity",				&FFLib::IsEntity)
		.addFunction("IsPlayer",				&FFLib::IsPlayer)
		.addFunction("IsDispenser",				&FFLib::IsDispenser)
		.addFunction("IsSentrygun",				&FFLib::IsSentrygun)
		.addFunction("IsDetpack",				&FFLib::IsDetpack)
		.addFunction("IsJumpPad",				&FFLib::IsJumpPad)
		.addFunction("IsBuildable",				&FFLib::IsBuildable)
		.addFunction("IsGrenade",				&FFLib::IsGrenade)
		.addFunction("IsTurret",				&FFLib::IsTurret)
		.addFunction("IsProjectile",			&FFLib::IsProjectile)
		.addFunction("IsInfoScript",			&FFLib::IsInfoScript)
		.addFunction("IsTriggerScript",			&FFLib::IsTriggerScript)
		.addFunction("IsTriggerClip",			&FFLib::IsTriggerClip)
		//.addFunction("KillAndRespawnAllPlayers",	&FFLib::KillAndRespawnAllPlayers)
		.addFunction("NumPlayers",				&FF_NumPlayers)
		.addFunction("GetPlayers",				&FFLib::GetPlayers)

		.addFunction("OutputEvent",
			overload<const char*, const char*>(&FFLib::FireOutput),
			overload<const char*, const char*, const char*>(&FFLib::FireOutput),
			overload<const char*, const char*, const char*, float>(&FFLib::FireOutput),
			overload<const char*, const char*, const char*, float, unsigned int>(&FFLib::FireOutput)
		)

		.addFunction("PrecacheModel",
			overload<const char*>(&FFLib::FFPrecacheModel), // FF backwards compatibility
			overload<const char*, bool>(& CBaseEntity::PrecacheModel)
		)

		.addFunction("PrecacheSound",			&CBaseEntity::PrecacheScriptSound)
		.addFunction("PrintBool",				&FFLib::PrintBool)
		.addFunction("RandomFloat",				&FFLib::RandomFloat)
		.addFunction("RandomInt",				&FFLib::RandomInt)
		.addFunction("RemoveEntity",			&FFLib::RemoveEntity)
		.addFunction("RemoveHudItem",			&FFLib::RemoveHudItem)
		.addFunction("RemoveHudItemFromTeam",	&FFLib::RemoveHudItemFromTeam)
		.addFunction("RemoveHudItemFromAll",	&FFLib::RemoveHudItemFromAll)
		//.addFunction("RespawnAllPlayers",		&FFLib::RespawnAllPlayers)
		.addFunction("ResetMap",				&FFLib::ResetMap)
		.addFunction("SetGlobalRespawnDelay",	&FFLib::SetGlobalRespawnDelay)
		.addFunction("SetPlayerLimit",			&FFLib::SetPlayerLimit)
		.addFunction("SetPlayerLimits",			&FFLib::SetPlayerLimits)
		.addFunction("SetClassLimits",			&FFLib::SmartClassLimits)

		.addFunction("SetConvar",
			overload<const char*, float>(&FFLib::SetConvar),
			overload<const char*, const char*>(&FFLib::SetConvar)
		)

		.addFunction("SetTeamClassLimit",		&FFLib::SetTeamClassLimit)
		.addFunction("SetTeamName",				&FFLib::SetTeamName)

		.addFunction("SmartMessage",
			overload<CBaseEntity*, const char*, const char*, const char*>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, int, int, int>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, const char*, const char*, const char*>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, int, const char*, const char*>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, int, int, const char*>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, int, const char*, int>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, const char*, int, const char*>(&FFLib::SmartMessage),
			overload<CBaseEntity*, const char*, const char*, const char*, const char*, int, int>(&FFLib::SmartMessage)
		)

		.addFunction("SmartSound",				&FFLib::SmartSound)
		.addFunction("SmartSpeak",				&FFLib::SmartSpeak)

		.addFunction("SmartTeamMessage",
			overload<CFFTeam*, const char*, const char*>(&FFLib::SmartTeamMessage),
			overload<CFFTeam*, const char*, const char*, int, int>(&FFLib::SmartTeamMessage),
			overload<CFFTeam*, const char*, const char*, const char*, const char*>(&FFLib::SmartTeamMessage),
			overload<CFFTeam*, const char*, const char*, int, const char*>(&FFLib::SmartTeamMessage),
			overload<CFFTeam*, const char*, const char*, const char*, int>(&FFLib::SmartTeamMessage)
		)

		.addFunction("SmartTeamSound",			&FFLib::SmartTeamSound)
		.addFunction("SmartTeamSpeak",			&FFLib::SmartTeamSpeak)
		.addFunction("SpeakAll",				&FFLib::SpeakAll)
		.addFunction("SpeakPlayer",				&FFLib::SpeakPlayer)
		.addFunction("SpeakTeam",				&FFLib::SpeakTeam)

		.addFunction("LogLuaEvent",
			overload<int, int, const char*>(&FFLib::LogLuaEvent),
			overload<int, int, const char*, const char*, const char*>(&FFLib::LogLuaEvent),
			overload<int, int, const char*, const char*, const char*, const char*, const char*>(&FFLib::LogLuaEvent),
			overload<int, int, const char*, const char*, const char*, const char*, const char*, const char*, const char*>(&FFLib::LogLuaEvent)
		)

		.addFunction("ObjectiveNotice",			&FFLib::ObjectiveNotice)
		.addFunction("UpdateObjectiveIcon",		&FFLib::UpdateObjectiveIcon)
		.addFunction("UpdateTeamObjectiveIcon",	&FFLib::UpdateTeamObjectiveIcon)
		.addFunction("DisplayMessage",			&FFLib::SendHintToPlayer)
		.addFunction("SendHintToPlayer",		&FFLib::SendHintToPlayer)
		.addFunction("SendHintToTeam",			&FFLib::SendHintToTeam)
		.addFunction("SendHintToAll",			&FFLib::SendHintToAll)
		.addFunction("SetGameDescription",		&FFLib::SetGameDescription)
		.addFunction("GetGameDescription",		&FFLib::GetGameDescription)
		.addFunction("ShowMenuToPlayer",		&FFLib::ShowMenuToPlayer)
		.addFunction("ShowMenuToTeam",			&FFLib::ShowMenuToTeam)
		.addFunction("ShowMenu",				&FFLib::ShowMenu)

		.addFunction("CreateMenu",
			overload<const char*>(&FFLib::CreateMenu),
			overload<const char*, float>(&FFLib::CreateMenu),
			overload<const char*, const char*>(&FFLib::CreateMenu),
			overload<const char*, const char*, float>(&FFLib::CreateMenu)
		)

		.addFunction("DestroyMenu",				&FFLib::DestroyMenu)
		.addFunction("SetMenuTitle",			&FFLib::SetMenuTitle)
		.addFunction("AddMenuOption",			&FFLib::AddMenuOption)
		.addFunction("RemoveMenuOption",		&FFLib::RemoveMenuOption)

		.addFunction("SpawnEntity",
			overload<const char*>(&FFLib::SpawnEntity),
			overload<const char*, const char*>(&FFLib::SpawnEntity)
		)

		.addFunction("GetMapName",				&FFLib::GetMapName);
}
