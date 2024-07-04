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
END_NETWORK_TABLE()

// Start of our data description for the class
BEGIN_DATADESC( CFFTeleporter )
#ifdef GAME_DLL
	DEFINE_ENTITYFUNC( OnObjectTouch ),
	DEFINE_THINKFUNC( OnTeleporterThink ),
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
	m_iLastState = 0;

	m_flLastTeleport = 0;
	m_flNextTeleport = 0;
#endif

	// Health
	m_iMaxHealth = m_iHealth = 100;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFTeleporter::~CFFTeleporter( void )
{
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

//-------------------------------------------------------------------------
// Purpose: Sentryguns will sometimes appear as the wrong model when
//			the local player is hallucinating
//-------------------------------------------------------------------------
int CFFTeleporter::DrawModel(int flags)
{
	int nRet = BaseClass::DrawModel(flags);
	
	if( gpGlobals->curtime < m_flLastDamage + MANCANNON_COMBATCOOLDOWN )
	{
		// Thanks mirv!
		IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_combat", TEXTURE_GROUP_CLIENT_EFFECTS );
		if( pMaterial )
		{
			CMatRenderContextPtr pMatRenderContext(g_pMaterialSystem);
			pMatRenderContext->Bind( pMaterial );

			// The color is based on the owner's team
			int iAlpha = 255;
			Color clr = Color( 255, 255, 255, iAlpha );

			if( g_PR )
			{
				int teamnumber = GetTeamNumber();
				float flCombatTime = clamp( gpGlobals->curtime - m_flLastDamage, 0, MANCANNON_COMBATCOOLDOWN );
				iAlpha = (255 * ( 1.0f - (flCombatTime / MANCANNON_COMBATCOOLDOWN) ) );
				clr = g_PR->GetTeamColor( teamnumber );
			}

			color32 c = { clr.r(), clr.g(), clr.b(), iAlpha };
			DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z + 48.0f ), 32.0f, 32.0f, c );
		}
	}

	return nRet;
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

	//Sets the team color -GreenMushy
	CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() ); //static_cast< CFFPlayer * >( m_hOwner.Get() );
	if( pOwner ) 
		m_nSkin = ( pOwner->GetTeamNumber() - 1 ); 

	m_bTakesDamage = true; // Making the teleporter take damage -GreenMushy
	m_iState = TELEPORTER_INCOMPLETE;
	m_iType = TELEPORTER_ENTRANCE;
	m_iTeleportState = TELEPORTER_READY;

	// Set the current and max health to the same values -Green Mushy
	m_iMaxHealth = TELEPORTER_HEALTH;
	m_iHealth = m_iMaxHealth;
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
	SetThink( &CFFTeleporter::OnTeleporterThink );
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
void CFFTeleporter::OnTeleporterThink( void )
{
	// teleporter buildstate
	if ( m_iState != TELEPORTER_INCOMPLETE && ( ( m_iType == TELEPORTER_ENTRANCE && !m_hExit ) || ( m_iType == TELEPORTER_EXIT && !m_hEntrance ) ) )
	{
		m_iState = TELEPORTER_INCOMPLETE;
		return;
	}
	else if ( m_iState != TELEPORTER_COMPLETE && ( ( m_iType == TELEPORTER_ENTRANCE && m_hExit ) || ( m_iType == TELEPORTER_EXIT && m_hEntrance ) ) )
	{
		m_iState = TELEPORTER_COMPLETE;
	}

	// teleporter teleport state
	if ( m_iTeleportState != TELEPORTER_READY && ( gpGlobals->curtime > m_flNextTeleport ) )
	{
		m_iTeleportState = TELEPORTER_READY;
		EmitSound("Teleporter.Ready");
	}
	else if ( m_iTeleportState != TELEPORTER_INCOOLDOWN && ( gpGlobals->curtime < m_flNextTeleport ) )
	{
		m_iTeleportState = TELEPORTER_INCOOLDOWN;
	}

	if (m_iType == TELEPORTER_ENTRANCE && m_hExit)
	{
		if (!m_hExit->m_hEntrance)
			m_hExit->m_hEntrance = this;
	}
	else if (m_iType == TELEPORTER_EXIT && m_hEntrance)
	{
		if (!m_hEntrance->m_hExit)
			m_hEntrance->m_hExit = this;
	}

	SetNextThink(gpGlobals->curtime + m_flThinkTime);
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

	if( g_pGameRules->PlayerRelationship( GetOwnerPlayer(), pPlayer ) == GR_NOTTEAMMATE ) // Team orients it -GreenMushy
		return;

	// a player is in queue already, respectfully fuck off
	if( m_hTouchingPlayer && m_hTouchingPlayer->entindex() != pPlayer->entindex() )
		return;

	m_hTouchingPlayer = pPlayer;

	// we don't teleport _from_ an exit, but we still store
	// the touching player so we can telefrag them :D
	if (m_iType == TELEPORTER_EXIT)
		return;

	if (m_iState != TELEPORTER_COMPLETE || m_iTeleportState != TELEPORTER_READY)
		return;
	
	// can only use it once per 10 seconds
	if (m_iTeleportState == TELEPORTER_INCOOLDOWN)
	{
		DevMsg("Teleporter ready in %f\n", (m_flNextTeleport - gpGlobals->curtime));
		return;
	}

	Vector playerVel = m_hTouchingPlayer->GetAbsVelocity();

	// TFC teleporters don't teleport unless the player completely stops
	if ( playerVel.x != 0 || playerVel.y != 0 )
		return;

	Vector pDest = m_hExit->GetAbsOrigin();
	pDest.z += 20;

	QAngle pDestAngles = m_hTouchingPlayer->GetAbsAngles();

	// TODO: actually teleport the player
	pPlayer->Teleport(&pDest, &pDestAngles, &playerVel);

	EmitSound("Teleporter.TeleportIn");

	pPlayer->m_flMancannonTime = gpGlobals->curtime;

	m_flLastTeleport = gpGlobals->curtime;
	m_flNextTeleport = m_flLastTeleport + TELEPORTER_COOLDOWN;

	m_iTeleportState = TELEPORTER_INCOOLDOWN;

	DevMsg( "[Teleporter] You have been teleported, teleport time is %f and next teleport time is %f\n", m_flLastTeleport, m_flNextTeleport );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFTeleporter *CFFTeleporter::Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner )
{
	CFFTeleporter *pObject = (CFFTeleporter *)CBaseEntity::Create( "FF_Teleporter", vecOrigin, vecAngles, NULL );

	pObject->m_hOwner.GetForModify() = pentOwner;
	//pObject->VPhysicsInitNormal( SOLID_VPHYSICS, pObject->GetSolidFlags(), true );
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

		int iHealth = (int) (100.0f * GetHealth() / TELEPORTER_HEALTH);
		//int iArmor = (int) ( 100.0f * m_iSGArmor / m_iMaxSGArmor); // no more armor, just reduce max health - shok
		//int iAmmo = (int) (100.0f * (float) m_iShells / m_iMaxShells);

		// Last bit of ammo signifies whether the SG needs rockets
		//if (m_iMaxRockets && !m_iRockets) 
		//	m_iAmmoPercent += 128;

		// If things haven't changed then do nothing more
		int iState = iHealth;
		if (m_iLastState == iState)
			return;

		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "TeleporterMsg");
			WRITE_BYTE(iHealth);
		MessageEnd();

		m_iLastState = iState;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFTeleporter::Detonate( void )
{
	VPROF_BUDGET( "CFFTeleporter::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "mancannon_detonated" );
	if( pEvent )
	{
		CFFPlayer *pOwner = GetOwnerPlayer();
		if( pOwner )
		{
			pEvent->SetInt( "userid", pOwner->GetUserID() );
			gameeventmanager->FireEvent( pEvent, true );
		}		
	}

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

void CFFTeleporter::SetEntrance( CFFTeleporter* pTeleporter )
{
	if (m_iType == TELEPORTER_ENTRANCE)
		return;

	m_hEntrance = pTeleporter;
}

void CFFTeleporter::SetExit( CFFTeleporter* pTeleporter )
{
	if (m_iType == TELEPORTER_EXIT)
		return;

	m_hExit = pTeleporter;
}

#endif // CLIENT_DLL : GAME_DLL