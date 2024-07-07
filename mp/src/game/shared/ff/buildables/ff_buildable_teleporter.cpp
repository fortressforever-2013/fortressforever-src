// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildable_teleporter.cpp
// @author Patrick O'Leary (Mulchman), -_YoYo178_-
// @date 12/6/2007
// @brief Man cannon thing
//
// REVISIONS
// ---------
// 12/6/2007, Mulchman: 
//		First created
//		Added man cannon stuff
// 
// 4/7/2024, -_YoYo178_-: 
//		Created teleporters based off
//			of mancannon
#include "cbase.h"
#include "ff_buildableobject.h"
#include "ff_buildable_teleporter.h"
#include "const.h"
#include "ff_gamerules.h"
#include "ff_utils.h"
#include "IEffects.h"

#include "tier0/vprof.h"

#ifdef CLIENT_DLL
	#include "c_playerresource.h"

	// for DrawSprite
	#include "beamdraw.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
//	class CFFTeleporter
//
//=============================================================================
IMPLEMENT_NETWORKCLASS_ALIASED( FFTeleporter, DT_FFTeleporter )

BEGIN_NETWORK_TABLE( CFFTeleporter, DT_FFTeleporter )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flLastTeleport ) ),
	RecvPropFloat( RECVINFO( m_flNextTeleport ) ),
#elif GAME_DLL
	SendPropFloat( SENDINFO( m_flLastTeleport ) ),
	SendPropFloat( SENDINFO( m_flNextTeleport ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFTeleporter )
	DEFINE_PRED_FIELD( m_flLastTeleport, FIELD_TIME, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flNextTeleport, FIELD_TIME, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

// Start of our data description for the class
BEGIN_DATADESC( CFFTeleporter )
#ifdef GAME_DLL
	DEFINE_THINKFUNC( OnThink ),
	DEFINE_ENTITYFUNC( OnObjectTouch ),
#endif
END_DATADESC()

LINK_ENTITY_TO_CLASS( FF_Teleporter, CFFTeleporter );
PRECACHE_REGISTER( FF_Teleporter );

extern const char *g_pszFFTeleporterModels[];
extern const char *g_pszFFTeleporterGibModels[];
extern const char *g_pszFFTeleporterSounds[];

extern const char *g_pszFFGenGibModels[];

//ConVar ffdev_teleporter_health( "ffdev_teleporter_health", "150", FCVAR_FF_FFDEV_REPLICATED );
#define TELEPORTER_HEALTH 150 //ffdev_teleporter_health.GetInt()
//ConVar ffdev_teleporter_glowduration( "ffdev_teleporter_glowduration", "11", FCVAR_FF_FFDEV_REPLICATED );
#define TELEPORTER_GLOW_DURATION 11.0f //ffdev_teleporter_glowduration.GetFloat()
//ConVar ffdev_teleporter_cooldown( "ffdev_teleporter_cooldown", "10", FCVAR_FF_FFDEV_REPLICATED );
#define TELEPORTER_COOLDOWN 10.0f //ffdev_teleporter_cooldown.GetFloat()

//ConVar ffdev_teleporter_cleartime("ffdev_teleporter_cleartime", "0.1", FCVAR_FF_FFDEV_REPLICATED);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFTeleporter::CFFTeleporter( void )
{
#ifdef GAME_DLL
	// Overwrite the base class stubs
	m_ppszModels = g_pszFFTeleporterModels;
	m_ppszGibModels = g_pszFFTeleporterGibModels;
	m_ppszSounds = g_pszFFTeleporterSounds;

	m_flLastClientUpdate = 0;
	m_iLastHealth = 0;

	m_flLastTeleport = 0;
	m_flNextTeleport = 0;
#endif

	// Health
	m_iMaxHealth = m_iHealth = TELEPORTER_HEALTH;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFTeleporter::~CFFTeleporter( void )
{
#ifdef GAME_DLL
	// unlink entrances and exits properly
	if ( GetOther() )
		GetOther()->SetOther( NULL );
#endif
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFTeleporter::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates a client side entity using the teleporter model
//-----------------------------------------------------------------------------
CFFTeleporter *CFFTeleporter::CreateClientSideTeleporter( const Vector& vecOrigin, const QAngle& vecAngles )
{
	CFFTeleporter *pTeleporter = new CFFTeleporter;

	if( !pTeleporter )
		return NULL;

	if( !pTeleporter->InitializeAsClientEntity( FF_TELEPORTER_MODEL, RENDER_GROUP_TRANSLUCENT_ENTITY ) )
	{
		pTeleporter->Release();
		return NULL;
	}

	pTeleporter->SetAbsOrigin( vecOrigin );
	pTeleporter->SetLocalAngles( vecAngles );
	pTeleporter->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	pTeleporter->SetRenderMode( kRenderTransAlpha );
	pTeleporter->SetRenderColorA( ( byte )110 );
	
	if( FFDEV_PULSEBUILDABLE )
		pTeleporter->m_nRenderFX = g_BuildableRenderFx;

	// Since this is client side only, give it an owner just in case
	// someone accesses the m_hOwner.Get() and wants to return something
	// that isn't NULL!
	pTeleporter->m_hOwner = (C_BaseEntity *)C_BasePlayer::GetLocalPlayer();
	//Team Coloring -GreenMushy
	// slightly modified by Dexter to use the member just set.. :)	
	pTeleporter->m_nSkin = ( pTeleporter->m_hOwner->GetTeamNumber() - 1 );
	pTeleporter->SetClientSideOnly( true );
	pTeleporter->SetNextClientThink( CLIENT_THINK_ALWAYS );

	return pTeleporter;
}

#elif GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFTeleporter::Spawn( void )
{
	VPROF_BUDGET( "CFFTeleporter::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	Precache();
	CFFBuildableObject::Spawn();

	// Sets the team color -GreenMushy
	CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() );
	if( pOwner ) 
		m_nSkin = ( pOwner->GetTeamNumber() - 1 ); 

	m_bTakesDamage = true; // Making the teleporter take damage -GreenMushy

	SetState( TELEPORTER_INCOMPLETE );
	SetType( TELEPORTER_ENTRANCE ); // we set this while building, but still providing a default value just in case
	SetTeleportState( TELEPORTER_INCOOLDOWN ); // HACK: allow playing teleporter ready sound when exit is built, see CFFTeleporter::OnThink

	// Set the current and max health to the same values -Green Mushy
	m_iHealth = m_iMaxHealth = TELEPORTER_HEALTH;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFTeleporter::GoLive( void )
{
	VPROF_BUDGET( "CFFTeleporter::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CFFBuildableObject::GoLive();

	m_bBuilt = true;

	SetTouch( &CFFTeleporter::OnObjectTouch );

	m_flThinkTime = 0.1f;
	SetThink( &CFFTeleporter::OnThink );
	SetNextThink( gpGlobals->curtime + m_flThinkTime );

	// Create our flickerer
	m_pFlickerer = ( CFFBuildableFlickerer * )CreateEntityByName( "ff_buildable_flickerer" );
	if( !m_pFlickerer )
	{
		Warning( "[Teleporter] Failed to create flickerer!\n" );
		m_pFlickerer = NULL;
	}
	else
	{		
		m_pFlickerer->SetBuildable( this );
		m_pFlickerer->Spawn();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Generic think function
//-----------------------------------------------------------------------------
void CFFTeleporter::OnThink( void )
{
	if ( ( gpGlobals->curtime - m_flPlayerLastTouch ) > /*ffdev_teleporter_cleartime.GetFloat()*/ 0.1f )
		m_hTouchingPlayer = NULL;

	if ( IsEntrance() )
	{
		if ( m_hTouchingPlayer )
			DevWarning("[Teleporter 1] Has player\n");
		else
			DevWarning("[Teleporter 1] No player\n");
	}
	else
	{
		if ( m_hTouchingPlayer )
			DevWarning("[Teleporter 2] Has player\n");
		else
			DevWarning("[Teleporter 2] No player\n");
	}
	// teleporter buildstate
	if ( !IsComplete() && GetOther() )
	{
		SetState( TELEPORTER_COMPLETE );
	}
	else if ( IsComplete() && !GetOther() )
	{
		SetState( TELEPORTER_INCOMPLETE );
		SetTeleportState( TELEPORTER_INCOOLDOWN ); // will allow playing sound when it's complete again
	}

	if( IsComplete() )
	{
		// teleporter teleport state
		if ( IsInCooldown() && ( gpGlobals->curtime > m_flNextTeleport ) )
		{
			SetTeleportState( TELEPORTER_READY );

			if( IsEntrance() )
				EmitSound("Teleporter.Ready");
		}
		else if ( !IsInCooldown() && ( gpGlobals->curtime < m_flNextTeleport ) )
		{
			SetTeleportState( TELEPORTER_INCOOLDOWN );
		}
	}

	// link entrances and exits properly
	if ( GetOther() )
	{
		if ( !GetOther()->GetOther() )
			GetOther()->SetOther( this );
	}

	SetNextThink( gpGlobals->curtime + m_flThinkTime );
}

//-----------------------------------------------------------------------------
// Purpose: Launch guy
//-----------------------------------------------------------------------------
void CFFTeleporter::OnObjectTouch( CBaseEntity *pOther )
{
	VPROF_BUDGET( "CFFTeleporter::OnObjectTouch", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	if( !IsBuilt() )
		return;

	if( !pOther )
		return;

	if( !pOther->IsPlayer() )
		return;

	CFFPlayer *pPlayer = ToFFPlayer( pOther );

	if( !pPlayer )
		return;

	// a player is in queue already
	if( m_hTouchingPlayer && pPlayer->entindex() != m_hTouchingPlayer->entindex())
		return;

	m_hTouchingPlayer = pPlayer;
	m_flPlayerLastTouch = gpGlobals->curtime;

	// we don't teleport _from_ an exit, but we still store
	// the touching player so we can telefrag them :D
	if ( IsExit() )
		return;

	// only teammates can use the entrance (should disguised spies also be able to use it?)
	if( g_pGameRules->PlayerRelationship( GetOwnerPlayer(), pPlayer ) == GR_NOTTEAMMATE ) // Team orients it -GreenMushy
		return;

	if ( !IsComplete() )
		return;
	
	// can only use it once per 10 seconds
	if ( IsInCooldown() )
	{
		DevMsg("Teleporter ready in %f\n", (m_flNextTeleport - gpGlobals->curtime));
		return;
	}

	Vector playerVel = m_hTouchingPlayer->GetAbsVelocity();

	// TFC teleporters don't teleport unless the player completely stops
	if ( playerVel.x != 0 || playerVel.y != 0 )
		return;

	Vector pDest = GetOther()->GetAbsOrigin();
	pDest.z += 50;

	QAngle pDestAngles = m_hTouchingPlayer->GetAbsAngles();

	// TODO: actually teleport the player
	pPlayer->Teleport(&pDest, &pDestAngles, &playerVel);
	OnTeleport(pPlayer);

	// Emit sound at the entrance and clear the touching player
	// (this part can be moved to OnTeleport() but i prefer having it here)
	EmitSound( "Teleporter.TeleportIn" );
	m_hTouchingPlayer = NULL;

	//pPlayer->m_flMancannonTime = gpGlobals->curtime;

	m_flLastTeleport = gpGlobals->curtime;
	m_flNextTeleport = m_flLastTeleport + TELEPORTER_COOLDOWN;

	SetTeleportState( TELEPORTER_INCOOLDOWN );

	DevMsg( "[Teleporter] You have been teleported, teleport time is %f and next teleport time is %f\n", m_flLastTeleport.Get(), m_flNextTeleport.Get());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFTeleporter *CFFTeleporter::Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner )
{
	CFFTeleporter *pObject = (CFFTeleporter *)CBaseEntity::Create( "FF_Teleporter", vecOrigin, vecAngles, NULL );

	pObject->m_hOwner.GetForModify() = pentOwner;
	pObject->Spawn();

	return pObject;
}

//-----------------------------------------------------------------------------
// Purpose: Update the client
//-----------------------------------------------------------------------------
void CFFTeleporter::PhysicsSimulate()
{
	BaseClass::PhysicsSimulate();

	// Update the client every 0.2 seconds
	if (gpGlobals->curtime > m_flLastClientUpdate + 0.2f)
	{
		m_flLastClientUpdate = gpGlobals->curtime;

		CFFPlayer *pPlayer = ToFFPlayer( m_hOwner.Get() );

		if (!pPlayer)
			return;

		int iHealth = ( GetHealth() / TELEPORTER_HEALTH ) * 100;
		int iRecharge = ( IsEntrance() && GetOther() )
			? clamp( (int) ( 100 - ( ( m_flNextTeleport - gpGlobals->curtime - 0.2f ) / TELEPORTER_COOLDOWN ) * 100.0f ), 0, 100 )
			: 0;

		// send updates to client only if our health changed, or is in cooldown
		if ( m_iLastHealth == iHealth && !IsInCooldown() )
			return;

		if ( !IsComplete() )
			iRecharge = 0;

		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "TeleporterMsg");
		WRITE_BYTE(iHealth);
		WRITE_BYTE(iRecharge);
		WRITE_BYTE(GetType());
		MessageEnd();

		m_iLastHealth = iHealth;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFTeleporter::Detonate( void )
{
	VPROF_BUDGET( "CFFTeleporter::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "teleporter_detonated" );
	if( pEvent )
	{
		CFFPlayer *pOwner = GetOwnerPlayer();
		if( pOwner )
		{
			pEvent->SetInt( "userid", pOwner->GetUserID() );
			gameeventmanager->FireEvent( pEvent, true );
		}		
	}

	if ( GetOther() )
		GetOther()->SetOther( NULL );

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFTeleporter::DoExplosionDamage( void )
{
	VPROF_BUDGET( "CFFTeleporter::DoExplosionDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );
	//Jiggles: Actually, we'd rather this not do any damage
	float flDamage = 140.0f;

	//if( m_hOwner.Get() )
	//{
	//	CTakeDamageInfo info( this, m_hOwner, vec3_origin, GetAbsOrigin(), flDamage, DMG_BLAST );
	//	RadiusDamage( info, GetAbsOrigin(), 625, CLASS_NONE, NULL );
		
		UTIL_ScreenShake( GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START );
	//}
}

// this will ONLY be called from the entrance
void CFFTeleporter::OnTeleport( CBaseEntity* pEntity )
{
	CFFPlayer* pPlayer = ToFFPlayer( pEntity );
	CFFTeleporter* pExit = GetOther();

	// neither player nor the exit will be NULL here
	// if any of them are NULL for some reason
	// something REALLY BAD has happened
	if ( !pPlayer || !pExit )
	{
		ASSERT( false );
		return;
	}

	// Emit sound at the exit
	pExit->EmitSound( "Teleporter.TeleportOut" );

	// Is a player touching the exit?
	if ( pExit->m_hTouchingPlayer && pExit->m_hTouchingPlayer->entindex() != pPlayer->entindex() )
	{
		// make sure it's not one of our teammates, TakeDamage() does handle
		// teammates just fine based on 'mp_friendlyfire' CVAR
		// but it would fire a false 'player_ondamage' event to LUA
		if ( g_pGameRules->PlayerRelationship( pExit->m_hTouchingPlayer, pPlayer ) == GR_NOTTEAMMATE )
		{
			CTakeDamageInfo info( this, pPlayer, 999, DMG_CRUSH | DMG_ALWAYSGIB );
			//info.SetDamageCustom( DAMAGETYPE_TELEFRAG );

			// Telefrag!
			pExit->m_hTouchingPlayer->TakeDamage(info);

			// it is possible that the player wouldn't die even after calling TakeDamage()
			if ( !pExit->m_hTouchingPlayer->IsAlive() )
				pExit->m_hTouchingPlayer = NULL;
		}
	}
}

CFFTeleporter* CFFTeleporter::GetOther( void )
{
	if ( IsEntrance() )
		return m_hExit;
	else
		return m_hEntrance;
}

void CFFTeleporter::SetOther( CFFTeleporter* pOther )
{
	if ( IsEntrance() )
		m_hExit = pOther;
	else
		m_hEntrance = pOther;
}
#endif // CLIENT_DLL : GAME_DLL