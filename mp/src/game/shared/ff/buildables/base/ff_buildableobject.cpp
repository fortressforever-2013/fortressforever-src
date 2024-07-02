// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobject.cpp
// @author Patrick O'Leary (Mulchman)
// @date 12/15/2005
// @brief BuildableObject class
//
// REVISIONS
// ---------
// 12/15/2005, Mulchman: 
//		First created
//
// 12/23-25/2005, Mulchman: 
//		A bunch of modifications (explosions, gibs, fire, building checking)
//
// 12/28/2004, Mulchman:
//		Bunch of mods - shares network values correctly. Officially a base 
//		class for other buildables
//
// 01/20/2004, Mulchman: 
//		Having no sounds (build/explode) won't cause problems
//
// 05/09/2005, Mulchman: 
//		Tons of additions - better checking of build area, lots of 
//		cleanup... basically an overhaul
//
//	06/30/2006, Mulchman:
//		This thing has been through tons of changes and additions.
//		The latest thing is the doorblockers
//
//	05/10/2006, Mulchman:
//		Messing w/ the explode function and dealing better damage

#include "cbase.h"
#include "ff_buildableobject.h"
#include "ff_buildableinfo.h"
#include "ff_gamerules.h"
#include "ff_utils.h"

#include "ff_buildable_mancannon.h"

#include "tier0/vprof.h"

#ifdef CLIENT_DLL
	#include "c_playerresource.h"
	#include "c_ff_team.h"
	#include "c_ff_player.h"

	// for DrawSprite
	#include "beamdraw.h"
#elif GAME_DLL
	#include "gib.h"
	#include "EntityFlame.h"

	#include "ff_team.h"
	#include "ff_player.h"
	#include "ff_luacontext.h"
	#include "ff_scriptman.h"
	#include "ff_entity_system.h"

	#include "omnibot_interface.h"
#endif

#ifdef _DEBUG
	#include "Color.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( FFBuildableObject, DT_FFBuildableObject )

BEGIN_NETWORK_TABLE( CFFBuildableObject, DT_FFBuildableObject )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hOwner ) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
	RecvPropBool( RECVINFO( m_bBuilt ) ),
	RecvPropFloat( RECVINFO( m_flSabotageTime ) ),
	RecvPropInt( RECVINFO( m_iSaboteurTeamNumber ) ),
#elif GAME_DLL
	SendPropEHandle( SENDINFO( m_hOwner ) ),
	SendPropInt( SENDINFO( m_iHealth ), 10 ),	// AfterShock: this can probably be limited to ~9 bits? max 180ish health on a SG? Do we ever use hp <0 for checking death?
	SendPropInt( SENDINFO( m_iMaxHealth ), 10 ), // The client should work this out using Classify() and getLevel() 
	SendPropBool( SENDINFO( m_bBuilt ) ), // can we encode this in the health? perhaps when health is 0?
	SendPropFloat( SENDINFO( m_flSabotageTime ) ), // Do we need to send these for jumppads + detpacks? maybe make new datatables?
	SendPropInt( SENDINFO( m_iSaboteurTeamNumber ) ),
#endif
END_NETWORK_TABLE()

// Start of our data description for the class
BEGIN_DATADESC( CFFBuildableObject )
#ifdef GAME_DLL
	DEFINE_THINKFUNC(DetonateThink),
	//DEFINE_THINKFUNC( OnObjectThink ),
#endif
END_DATADESC( )

LINK_ENTITY_TO_CLASS( FF_BuildableObject_entity, CFFBuildableObject );
PRECACHE_REGISTER( FF_BuildableObject_entity );

#ifdef CLIENT_DLL

	// Define all the sprites to precache
	CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorNoRoom )
	CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_NOROOM )
	CLIENTEFFECT_REGISTER_END()

	CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorTooSteep )
	CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_TOOSTEEP )
	CLIENTEFFECT_REGISTER_END()

	CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorTooFar )
	CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_TOOFAR )
	CLIENTEFFECT_REGISTER_END()

	CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorOffGround )
	CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_OFFGROUND )
	CLIENTEFFECT_REGISTER_END()

	CLIENTEFFECT_REGISTER_BEGIN( PrecacheBuildErrorMoveable )
	CLIENTEFFECT_MATERIAL( FF_BUILD_ERROR_MOVEABLE )
	CLIENTEFFECT_REGISTER_END()
#endif

const char *g_pszFFModels[ ] =
{
	NULL
};

const char *g_pszFFGibModels[ ] =
{
	NULL
};

const char *g_pszFFSounds[ ] =
{
	// BUILD SOUND HAS TO COME FIRST,
	// EXPLODE SOUND HAS TO COME SECOND,
	// Other sounds go here,
	// NULL if no sounds
	NULL
};

// Generic gib models used for every buildable object, yay
const char *g_pszFFGenGibModels[ ] =
{
	FF_BUILDABLE_GENERIC_GIB_MODEL_01,
	FF_BUILDABLE_GENERIC_GIB_MODEL_02,
	FF_BUILDABLE_GENERIC_GIB_MODEL_03,
	FF_BUILDABLE_GENERIC_GIB_MODEL_04,
	FF_BUILDABLE_GENERIC_GIB_MODEL_05,
	NULL
};

extern short	g_sModelIndexFireball;

//-----------------------------------------------------------------------------
// Purpose: Get health as a percentage
//-----------------------------------------------------------------------------
int CFFBuildableObject::GetHealthPercent( void ) const
{
	float flPercent = ( ( float )GetHealth() / ( float )GetMaxHealth() ) * 100.0f;

	// Don't display 0% it looks stupid
	if (flPercent < 1.0f)
		return 1;
	else
		return ( int )flPercent;
}

//-----------------------------------------------------------------------------
// Purpose: Team accessor [mirv]
//-----------------------------------------------------------------------------
int CFFBuildableObject::GetTeamNumber()
{
	CFFPlayer *pOwner = GetOwnerPlayer();

	if (!pOwner)
		return TEAM_UNASSIGNED;

	return pOwner->GetTeamNumber();
}

//-----------------------------------------------------------------------------
// Purpose: Get a buildables owner
//-----------------------------------------------------------------------------
CFFPlayer *CFFBuildableObject::GetOwnerPlayer( void )
{
	if( m_hOwner.Get() )
		return ToFFPlayer( m_hOwner.Get() );

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get a buildables team
//-----------------------------------------------------------------------------
CFFTeam *CFFBuildableObject::GetOwnerTeam( void )
{
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
	{
#ifdef _DEBUG
		Assert( dynamic_cast< CFFTeam * >( pOwner->GetTeam() ) != 0 );
#endif
		return static_cast< CFFTeam * >( pOwner->GetTeam() );
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get a buildables team id
//-----------------------------------------------------------------------------
int CFFBuildableObject::GetOwnerTeamId( void )
{
	CFFPlayer *pOwner = GetOwnerPlayer();
	if( pOwner )
		return pOwner->GetTeamNumber();

	return TEAM_UNASSIGNED;
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_FFBuildableObject::C_FFBuildableObject( void )
{
	// Initialize
	m_bClientSideOnly = false;
	m_flSabotageTime = 0.0f;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
C_FFBuildableObject::~C_FFBuildableObject( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_FFBuildableObject::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: To smooth the build helpers
//-----------------------------------------------------------------------------
void C_FFBuildableObject::ClientThink( void )
{
	// This is to "smooth" the build helper models
	if( m_bClientSideOnly )
	{
		C_FFPlayer *pPlayer = ToFFPlayer( m_hOwner.Get() );
		if( !pPlayer )
			return;

		float flBuildDist = 0.0f;

		switch( Classify() )
		{
			case CLASS_DISPENSER: flBuildDist = FF_BUILD_DISP_BUILD_DIST; break;
			case CLASS_SENTRYGUN: flBuildDist = FF_BUILD_SG_BUILD_DIST; break;
			case CLASS_DETPACK: flBuildDist = FF_BUILD_DET_BUILD_DIST; break;
			case CLASS_MANCANNON: flBuildDist = FF_BUILD_MC_BUILD_DIST; break;
			default: return; break;
		}

		Vector vecForward;
		pPlayer->EyeVectors( &vecForward );
		vecForward.z = 0.0f;
		VectorNormalize( vecForward );

		// Need to save off the z value before setting new origin
		Vector vecOrigin = GetAbsOrigin();

		// Compute a new origin in front of the player
		Vector vecNewOrigin = pPlayer->GetAbsOrigin() + ( vecForward * ( flBuildDist + 16.0f ) );
		vecNewOrigin.z = vecOrigin.z;

		SetAbsOrigin( vecNewOrigin );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Two pass so that the player icons can be drawn
//-----------------------------------------------------------------------------
RenderGroup_t C_FFBuildableObject::GetRenderGroup()
{
	if ( m_flSabotageTime > 0.0f )
		return RENDER_GROUP_TWOPASS;

	return BaseClass::GetRenderGroup();
}

//-----------------------------------------------------------------------------
// Purpose: Using this to draw any "can't build" type glyphs
//-----------------------------------------------------------------------------
int C_FFBuildableObject::DrawModel( int flags )
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	CMatRenderContextPtr pMatRenderContext(g_pMaterialSystem);

	// render a spy icon during the transparency pass
	if ( flags & STUDIO_TRANSPARENCY && pPlayer && !pPlayer->IsObserver() && m_flSabotageTime > 0.0f )
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( pPlayer->GetTeamNumber(), m_iSaboteurTeamNumber ) == GR_TEAMMATE )
		{
			// Thanks mirv!
			IMaterial *pMaterial = materials->FindMaterial( "sprites/ff_sprite_spy", TEXTURE_GROUP_CLIENT_EFFECTS );
			if( pMaterial )
			{
				pMatRenderContext->Bind( pMaterial );

				// The color is based on the saboteur's team
				int iAlpha = 255;
				Color clr = Color( 255, 255, 255, iAlpha );

				if( g_PR )
				{
					float flSabotageTime = clamp( m_flSabotageTime - gpGlobals->curtime, 0, FF_BUILD_SABOTAGE_TIMEOUT );
					iAlpha = 64 + (191 * (flSabotageTime / FF_BUILD_SABOTAGE_TIMEOUT) );
					clr = g_PR->GetTeamColor( m_iSaboteurTeamNumber );
				}

				color32 c = { clr.r(), clr.g(), clr.b(), iAlpha };
				DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z + 64.0f ), 15.0f, 15.0f, c );
			}
		}
	}

	if( m_bClientSideOnly )
	{
		// Draw our glyphs

		// See if there's even an error
		if( m_hBuildError != BUILD_ALLOWED )
		{
			float flOffset = 0.0f;
			
			// Get an offset for drawing (relative to GetAbsOrigin)
			const int iEntityClass = Classify();
			switch( iEntityClass )
			{
				case CLASS_DISPENSER: flOffset = 32.0f; break;
				case CLASS_SENTRYGUN: flOffset = 32.0f; break;
				case CLASS_DETPACK: flOffset = 0.0f; break;
				case CLASS_MANCANNON: flOffset = 0.0f; break;
				default: return BaseClass::DrawModel( flags ); break;
			}

			// Find out which error we're showing
			const char *pszMaterial = NULL;
			switch( m_hBuildError )
			{
				case BUILD_NOROOM: pszMaterial = FF_BUILD_ERROR_NOROOM; break;
				case BUILD_TOOSTEEP: pszMaterial = FF_BUILD_ERROR_TOOSTEEP; break;
				case BUILD_TOOFAR: pszMaterial = FF_BUILD_ERROR_TOOFAR; break;
				case BUILD_PLAYEROFFGROUND: pszMaterial = FF_BUILD_ERROR_OFFGROUND; break;
				case BUILD_MOVEABLE: pszMaterial = FF_BUILD_ERROR_MOVEABLE; break;
				case BUILD_NEEDAMMO: pszMaterial = FF_BUILD_ERROR_NEEDAMMO; break;
				case BUILD_ALREADYBUILT:
				{
					switch ( iEntityClass )
					{
					case CLASS_DISPENSER: pszMaterial = FF_BUILD_ERROR_ALREADYBUILTDISP; break;
					case CLASS_SENTRYGUN: pszMaterial = FF_BUILD_ERROR_ALREADYBUILTSG; break;
					case CLASS_MANCANNON: pszMaterial = FF_BUILD_ERROR_ALREADYBUILTMANCANNON; break;
					}

					break;
				}
			}

			// If a valid material...
			if( pszMaterial )
			{
				// Draw!
				IMaterial *pMaterial = materials->FindMaterial( pszMaterial, TEXTURE_GROUP_OTHER );
				if( pMaterial )
				{
					pMatRenderContext->Bind( pMaterial );
					color32 c = { 255, 255, 255, 255 };
					DrawSprite( Vector( GetAbsOrigin().x, GetAbsOrigin().y, EyePosition().z + flOffset ), 15.0f, 15.0f, c );
				}
			}

			// This just looks bad, even if it is a cvar. 
			// We're already drawing the sprite noculled.
			// UNDONE:
			// Finally, there was a build error, so don't actually draw the real model!
			//if(!ffdev_buildabledrawonerror.GetBool())
			return 0;
		}
	}
		
	
	return BaseClass::DrawModel( flags );
}

#elif GAME_DLL


/**
@fn CFFBuildableObject
@brief Constructor
@return N/A
*/
CFFBuildableObject::CFFBuildableObject( void )
{
	// Point these to stubs (super class re-sets these)
	m_ppszModels = g_pszFFModels;
	m_ppszGibModels = g_pszFFGibModels;
	m_ppszSounds = g_pszFFSounds;

	// Default values
	m_iExplosionMagnitude = 50;
	m_flExplosionMagnitude = 50.0f;
	m_flExplosionRadius = 3.5f * m_flExplosionMagnitude;
	m_iExplosionRadius = ( int )m_flExplosionRadius;	
	m_flExplosionForce = 100.0f;
	// TODO: for now - change this later? remember to update in dispenser.cpp as well
	m_flExplosionDamage = m_flExplosionForce;
	m_flExplosionDuration = 0.5f;
	m_iExplosionFireballScale = 1.1f;

	// Default think time
	m_flThinkTime = 0.2f;

	// Duration between sending hints for sentry/disp damage
	m_flOnTakeDamageHintTime = 0.0f;

	// Defaults for derived classes
	m_iShockwaveExplosionTexture = -1;
	m_bShockWave = false;
	
	m_bBuilt = false;
	m_bTakesDamage = true;
	m_bHasSounds = false;
	m_bTranslucent = true; // by default
	m_bUsePhysics = false;

	// Set to null
	m_pFlickerer = NULL;

	m_BuildableLocation[0] = 0;

	m_flSabotageTime = 0;
	m_hSaboteur = NULL;
	m_bMaliciouslySabotaged = false;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;

	m_bMarkedForDetonation = false;
}

/**
@fn ~CFFBuildableObject
@brief Destructor
@return N/A
*/
CFFBuildableObject::~CFFBuildableObject( void )
{
	// Remove the flickerer
	if( m_pFlickerer )
	{
		m_pFlickerer->SetBuildable( NULL );
		m_pFlickerer = NULL;
	}
}

/**
@fn void Spawn( )
@brief Do some generic stuff at spawn time (play sounds)
@return void
*/
void CFFBuildableObject::Spawn( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Set a team
	if( GetOwnerPlayer() )
		ChangeTeam( GetOwnerPlayer()->GetTeamNumber() );

	if( m_bUsePhysics )
	{
		SetSolid( SOLID_VPHYSICS );
		SetMoveType( MOVETYPE_VPHYSICS );
	}
	else
	{
		// So that doors collide with it
		SetSolid( SOLID_VPHYSICS );		
		AddSolidFlags( FSOLID_FORCE_WORLD_ALIGNED );
		SetMoveType( MOVETYPE_FLY );

		VPhysicsInitStatic();
	}

	SetCollisionGroup( COLLISION_GROUP_BUILDABLE_BUILDING );
		
	// Make sure it has a model
	Assert( m_ppszModels[ 0 ] != NULL );

	// Give it a model
	SetModel( m_ppszModels[ 0 ] );

	SetBlocksLOS( false );
	m_takedamage = DAMAGE_EVENTS_ONLY;

	// Play the build sound (if there is one)
	if( m_bHasSounds )
	{
		// Detpack sound is local only
		if( Classify() == CLASS_DETPACK )
		{
			if( GetOwnerPlayer() )
			{
				CSingleUserRecipientFilter sndFilter( GetOwnerPlayer() );
				EmitSound( sndFilter, entindex(), m_ppszSounds[ 0 ] );
			}			
		}
		else
		{
			CPASAttenuationFilter sndFilter( this );
			//sndFilter.AddRecipientsByPAS( GetAbsOrigin() );
			EmitSound( sndFilter, entindex(), m_ppszSounds[ 0 ] );
		}		
	}

	// Start making it drop and/or flash (if applicable)
	if( m_bTranslucent )
	{
		SetRenderMode( kRenderTransAlpha );
		SetRenderColorA( ( byte )110 );
	}

	// Don't let the model react to physics just yet
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if( pPhysics )
	{
		pPhysics->EnableCollisions( false );
		pPhysics->EnableMotion( false );
		pPhysics->EnableGravity( false );
		pPhysics->EnableDrag( false );
	}

	m_bBuilt = false;	// |-- Mirv: Make sure we're in a state of not built
}

/**
@fn void GoLive()
@brief Object is built and ready to do it's thing
@return void
*/
void CFFBuildableObject::GoLive( void )
{
	VPROF_BUDGET( "CFFBuildableObject::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Object is now built
	m_bBuilt = true;


	//Testing new collision group -Green Mushy
	//SetCollisionGroup( COLLISION_GROUP_PLAYER );
	SetCollisionGroup( COLLISION_GROUP_BUILDABLE );

	// Object is built and can take damage if it is supposed to
	if( m_bTakesDamage )
		m_takedamage = DAMAGE_YES;

	// Make opaque
	if( m_bTranslucent )
	{
		// Make sure the model is drawn normally now (if "flashing")
		SetRenderMode( kRenderNormal );
	}

	/*
	// React to physics!
	if( m_bUsePhysics )
	{
		IPhysicsObject *pPhysics = VPhysicsGetObject();
		if( pPhysics )
		{
			pPhysics->Wake();
			pPhysics->EnableCollisions( true );
			pPhysics->EnableMotion( true );
			pPhysics->EnableGravity( true );
			pPhysics->EnableDrag( true );			
		}
	}
	//*/

	//*
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if( pPhysics )
	{
		pPhysics->Wake();
		pPhysics->EnableCollisions( true );
		pPhysics->EnableMotion( m_bUsePhysics );
		pPhysics->EnableGravity( m_bUsePhysics );
		pPhysics->EnableDrag( m_bUsePhysics );

		if( Classify() == CLASS_DETPACK )
			pPhysics->SetMass( 500.0f );
		else if( Classify() == CLASS_MANCANNON)
			pPhysics->SetMass( 5000.0f );
	}
	//*/

	m_flSabotageTime = 0;
	m_hSaboteur = NULL;
	m_bMaliciouslySabotaged = false;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;

	// AfterShock: PUT SG BUILD SOUND SOMEWHERE HERE

	//if( m_bHasSounds )

		// Detpack sound is local only
		//if( Classify() == CLASS_SENTRYGUN )
		//{
		//	CPASAttenuationFilter sndFilter( this );
			//sndFilter.AddRecipientsByPAS( GetAbsOrigin() );
		//	EmitSound( sndFilter, entindex(), m_ppszSounds[ 0 ] );
		//}		

	SetContextThink(&CFFBuildableObject::DetonateThink, TICK_NEVER_THINK, "DetonateThink");
}

/**
@fn void Precache( )
@brief Precache's the model
@return void
*/
void CFFBuildableObject::Precache( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Precache", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Precache normal models
	int iCount = 0;
	while( m_ppszModels[ iCount ] != NULL )
	{
		PrecacheModel( m_ppszModels[ iCount ] );
		iCount++;
	}

	// Precache gib models
	iCount = 0;
	while( m_ppszGibModels[ iCount ] != NULL )
	{
		PrecacheModel( m_ppszGibModels[ iCount ] );
		iCount++;
	}

	// Precache the random always used gib models
	iCount = 0;
	while( g_pszFFGenGibModels[ iCount ] != NULL )
	{
		PrecacheModel( g_pszFFGenGibModels[ iCount ] );
		iCount++;
	}

	// See if we've got any sounds to precache
	if( m_ppszSounds[ 0 ] != NULL )
		if( m_ppszSounds[ 1 ] != NULL )
			m_bHasSounds = true;

	// Precache sound files
	if( m_bHasSounds )
	{
		iCount = 0;
		while( m_ppszSounds[ iCount ] != NULL )
		{		
			PrecacheScriptSound( m_ppszSounds[ iCount ] );
			iCount++;
		}
	}

	// Precache the shockwave
	if( m_bShockWave )
		m_iShockwaveExplosionTexture = PrecacheModel( "sprites/lgtning.vmt" );	

	// Call base class
	BaseClass::Precache();	
}


void CFFBuildableObject::DetonateThink(void)
{
	Detonate();
}

/**
@fn void Detonate( )
@brief The user wants to blow up their own object
@return void
*/
void CFFBuildableObject::Detonate( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Do the explosion and radius damage
	Explode();
}

void  CFFBuildableObject::UpdateOnRemove( void )
{
	RemoveSaboteur();
	BaseClass::UpdateOnRemove();
}

void CFFBuildableObject::RemoveSaboteur( bool bSuppressNotification )
{
	if ( !m_hSaboteur )
		return;

	if ( m_bMaliciouslySabotaged )
		bSuppressNotification = true;

	if ( !bSuppressNotification )
	{
		CSingleUserRecipientFilter filter(m_hSaboteur);
		CFFEntitySystemHelper* pHelperInst = CFFEntitySystemHelper::GetInstance();
		if(pHelperInst)
			pHelperInst->EmitSound( filter, pHelperInst->entindex(), "Player.SabotageTimedOut" );
	}

	if( Classify() == CLASS_SENTRYGUN )
	{
		if ( !bSuppressNotification )
			ClientPrint(m_hSaboteur, HUD_PRINTCENTER, "#FF_SENTRYSABOTAGERESET");
		m_hSaboteur->m_iSabotagedSentries--;
	}
	else if( Classify() == CLASS_DISPENSER )
	{
		if ( !bSuppressNotification )
			ClientPrint(m_hSaboteur, HUD_PRINTCENTER, "#FF_DISPENSERSABOTAGERESET");
		m_hSaboteur->m_iSabotagedDispensers--;
	}

	m_flSabotageTime = 0;
	m_hSaboteur = NULL;
	m_bMaliciouslySabotaged = false;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;
}

/**
@fn void RemoveQuietly( )
@brief The user died during build process
@return void
*/
void CFFBuildableObject::RemoveQuietly( void )
{
	VPROF_BUDGET( "CFFBuildableObject::RemoveQuietly", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// MUST DO THIS or CreateExplosion crashes HL2
	m_takedamage = DAMAGE_NO;

	// Remove bounding box
	SetSolid( SOLID_NONE );

	// Stop playing the build sound
	if( m_bHasSounds )
		StopSound( m_ppszSounds[ 0 ] );

	// Remove the flickerer
	if( m_pFlickerer )
	{
		m_pFlickerer->SetBuildable( NULL );
		m_pFlickerer = NULL;
	}

	// Notify player to tell them they can build
	// again and remove current owner
	m_hOwner = NULL;

	// Remove entity from game
	UTIL_Remove( this );
}

CFFBuildableObject *CFFBuildableObject::AttackerInflictorBuildable(CBaseEntity *pAttacker, CBaseEntity *pInflictor)
{
	if ( FF_IsBuildableObject( pAttacker ) )
		return FF_ToBuildableObject( pAttacker );

	if ( FF_IsBuildableObject( pInflictor ) )
		return FF_ToBuildableObject( pInflictor );

	if ( pInflictor && FF_IsBuildableObject( pInflictor->GetOwnerEntity() ) )
		return FF_ToBuildableObject( pInflictor->GetOwnerEntity() );

	if ( pAttacker && FF_IsBuildableObject( pAttacker->GetOwnerEntity() ) )
		return FF_ToBuildableObject( pAttacker->GetOwnerEntity() );

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: If already sabotaged then don't try and sabotage again
//-----------------------------------------------------------------------------
bool CFFBuildableObject::CanSabotage() const
{
	VPROF_BUDGET( "CFFBuildableObject::CanSabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if (!m_bBuilt)
		return false;

	return !IsSabotaged();
}

//-----------------------------------------------------------------------------
// Purpose: Is this buildable in level 1 sabotage (not maliciously)
//-----------------------------------------------------------------------------
bool CFFBuildableObject::IsSabotaged() const
{
	VPROF_BUDGET( "CFFBuildableObject::IsSabotaged", VPROF_BUDGETGROUP_FF_BUILDABLE );

	return (m_hSaboteur && m_flSabotageTime > gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Is this buildable in level 2 sabotage (maliciously)
//-----------------------------------------------------------------------------
bool CFFBuildableObject::IsMaliciouslySabotaged() const
{
	VPROF_BUDGET( "CFFBuildableObject::IsMaliciouslySabotaged", VPROF_BUDGETGROUP_FF_BUILDABLE );

	return (IsSabotaged() && m_bMaliciouslySabotaged);
}

/**
@fn void OnThink
@brief Think function (modifies our network health var for now)
@return void
*/
void CFFBuildableObject::OnObjectThink( void )
{
	VPROF_BUDGET( "CFFBuildableObject::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	float flTimeLeft = m_flSabotageTime - gpGlobals->curtime;

	if ( m_flSabotageTime <= gpGlobals->curtime )
		RemoveSaboteur(); // don't worry , this function checks if saboteur even exists
	else if ( m_hSaboteur && flTimeLeft > 0.0f && flTimeLeft <= 6.66f && !IsMaliciouslySabotaged() )
	{
		char sztimeleft[10];
		Q_snprintf( sztimeleft, sizeof( sztimeleft ), "%.2f", flTimeLeft );

		switch( Classify() )
		{
		case CLASS_DISPENSER:
			ClientPrint( m_hSaboteur, HUD_PRINTCENTER, "#FF_DISPENSERSABOTAGERESETTING", sztimeleft );
			break;

		case CLASS_SENTRYGUN:
			ClientPrint( m_hSaboteur, HUD_PRINTCENTER, "#FF_SENTRYSABOTAGERESETTING", sztimeleft );
			break;
		}
	}

	// Check for "malfunctions"
	if( HasMalfunctioned() )
	{
		CFFPlayer *pOwner = GetOwnerPlayer();
		if( pOwner )
		{
			switch( Classify() )
			{
				case CLASS_DISPENSER:
					ClientPrint( pOwner, HUD_PRINTCENTER, "#FF_DISPENSER_MALFUNCTIONED" );
				break;

				case CLASS_SENTRYGUN:
					ClientPrint( pOwner , HUD_PRINTCENTER, "#FF_SENTRYGUN_MALFUNCTIONED" );
				break;
			}
		}

		Detonate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: See if we've malfunctioned
//-----------------------------------------------------------------------------
bool CFFBuildableObject::HasMalfunctioned( void ) const
{
	VPROF_BUDGET( "CFFBuildableObject::HasMalfunctioned", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// This test can be flakey hence switching to distance and giving
	// it a little fluff. If part of the bbox is inside something the
	// origin will jump around (in really really small values though
	// but enough so that m_vecGroundOrigin != GetAbsOrigin()
	if( m_bBuilt && ( m_vecGroundOrigin.DistTo( GetAbsOrigin() ) > 3.0f ) )
		return true;

	return false;
}

/**
@fn void Event_Killed( const CTakeDamageInfo& info )
@brief Called automatically when an object dies
@param info - CTakeDamageInfo structure
@return void
*/
void CFFBuildableObject::Event_Killed( const CTakeDamageInfo& info )
{
	VPROF_BUDGET( "CFFBuildableObject::Event_Killed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Remove the flickerer
	if( m_pFlickerer )
	{
		m_pFlickerer->SetBuildable( NULL );
		m_pFlickerer = NULL;
	}

	CFFLuaSC hBuildableKilled;
	hBuildableKilled.Push(this);
	hBuildableKilled.Push(&info);
	_scriptman.RunPredicates_LUA( NULL, &hBuildableKilled, "buildable_killed" );

	// Can't kill detpacks
	if( Classify() != CLASS_DETPACK )
	{
		// Send to game rules to then fire event and send out hud death notice
		FFGameRules()->BuildableKilled( this, info );
	}

	// Do the explosion and radius damage last, because it clears the owner.
	Explode();
}

/**
@fn CFFBuildableObject *Create( const Vector& vecOrigin, const QAngle& vecAngles, edict_t *pentOwner )
@brief Creates a new CFFBuildableObject at a particular location
@param vecOrigin - origin of the object to be created
@param vecAngles - view angles of the object to be created
@param pentOwner - edict_t of the owner creating the object (usually keep NULL)
@return a _new_ CFFBuildableObject object
*/ 
CFFBuildableObject *CFFBuildableObject::Create( const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity *pentOwner )
{
	// Create the object
	CFFBuildableObject *pObject = ( CFFBuildableObject * )CBaseEntity::Create( "FF_BuildableObject_entity", vecOrigin, vecAngles, pentOwner );

	// Set the real owner. We're not telling CBaseEntity::Create that we have an owner so that
	// touch functions [and possibly other items] will work properly when activated by us
	pObject->m_hOwner = pentOwner;

	// Spawn the object
	pObject->Spawn();

	return pObject;
}

//
// VPhysicsTakeDamage
//		So weapons like the railgun won't effect building
//
int CFFBuildableObject::VPhysicsTakeDamage( const CTakeDamageInfo &info )
{
	VPROF_BUDGET( "CFFBuildableObject::VPhysicsTakeDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	return 0;
}

/**
@fn void Explode( )
@brief For when the object blows up
@return void
*/
void CFFBuildableObject::Explode( void )
{
	VPROF_BUDGET( "CFFBuildableObject::Explode", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// MUST DO THIS or CreateExplosion crashes HL2
	m_takedamage = DAMAGE_NO;

	// Remove bounding box (other models follow this pattern...)
	SetSolid( SOLID_NONE );

	// Do the explosion
	DoExplosion();

	// Notify player to tell them they can build
	// again and remove current owner
	m_hOwner = NULL;

	// Remove entity from game 
	UTIL_Remove( this );
}

/**
@fn void SpawnGib( const char *szGibModel )
@brief Creates a gib and tosses it randomly
@param szGibModel - path to a model for the gib to toss
@return void
*/
void CFFBuildableObject::SpawnGib( const char *szGibModel, bool bFlame, bool bDieGroundTouch )
{
	VPROF_BUDGET( "CFFBuildableObject::SpawnGib", VPROF_BUDGETGROUP_FF_BUILDABLE );

	
	// Create some gibs! MMMMM CHUNKY
	CGib *pChunk = CREATE_ENTITY( CGib, "gib" );
	pChunk->Spawn( szGibModel );
	pChunk->SetBloodColor( DONT_BLEED );

	QAngle vecSpawnAngles;
	vecSpawnAngles.Random( -90, 90 );
	pChunk->SetAbsOrigin( GetAbsOrigin( ) );
	pChunk->SetAbsAngles( vecSpawnAngles );

	pChunk->SetOwnerEntity( this );
	if( bDieGroundTouch )
		pChunk->m_lifeTime = 0.0f;
	else
		pChunk->m_lifeTime = random->RandomFloat( 1.0f, 3.0f );
	pChunk->SetCollisionGroup( COLLISION_GROUP_DEBRIS );

	IPhysicsObject *pPhysicsObject = pChunk->VPhysicsInitNormal( SOLID_VPHYSICS, pChunk->GetSolidFlags(), false );
	if( pPhysicsObject )
	{
		pPhysicsObject->EnableMotion( true );
		Vector vecVelocity;

		QAngle angles;
		angles.x = random->RandomFloat( -40, 0 );
		angles.y = random->RandomFloat( 0, 360 );
		angles.z = 0.0f;
		AngleVectors( angles, &vecVelocity );

		vecVelocity *= random->RandomFloat( 5, 50 );
		vecVelocity += GetAbsVelocity( );

		AngularImpulse angImpulse;
		angImpulse = RandomAngularImpulse( -180, 180 );

		pChunk->SetAbsVelocity( vecVelocity );
		pPhysicsObject->SetVelocity( &vecVelocity, &angImpulse );
	}

	if( bFlame )
	{
		// Add a flame to the gib
		CEntityFlame *pFlame = CEntityFlame::Create( pChunk, false );
		if( pFlame != NULL )
		{
			pFlame->SetLifetime( pChunk->m_lifeTime );
		}
	}

	// Make the next think function the die one (to get rid of the damn things)
	if( bDieGroundTouch )
		pChunk->SetThink( &CGib::SUB_FadeOut );
	else
		pChunk->SetThink( &CGib::DieThink );

}

/**
@fn virtual void SpawnGibs()
@brief Creates gibs
@return void
*/
void CFFBuildableObject::SpawnGibs()
{
	int iGib = 0;
	while (m_ppszGibModels[iGib])
	{
		SpawnGib(m_ppszGibModels[iGib], false, false);
		++iGib;
	}
}

/**
@fn void DoExplosion( )
@brief For when the object blows up (the actual explosion)
@return void
*/
void CFFBuildableObject::DoExplosion( void )
{
	VPROF_BUDGET( "CFFBuildableObject::DoExplosion", VPROF_BUDGETGROUP_FF_BUILDABLE );

	//CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );

	// Explosion!

	// Don't play sound if the buildable has an explode sound.
	int flags = TE_EXPLFLAG_NONE;
	if(m_ppszSounds[1])
		flags = TE_EXPLFLAG_NOSOUND;

	Vector vecAbsOrigin = GetAbsOrigin() + Vector( 0, 0, 32.0f ); // Bring off the ground a little 
	CPASFilter filter( vecAbsOrigin );
	te->Explosion( filter,			// Filter
		0.0f,						// Delay
		&vecAbsOrigin,				// Origin (position)
		g_sModelIndexFireball,		// Model index
		m_iExplosionFireballScale,	// scale
		random->RandomInt( 8, 15 ),	// framerate
		flags,			// flags
		m_iExplosionRadius,			// radius
		m_iExplosionMagnitude		// magnitude
	);

	// Play the explosion sound
	if( m_bHasSounds )
	{
		// m_ppszSounds[1] is the explosion sound

		CPASAttenuationFilter sndFilter( this );
		EmitSound( sndFilter, entindex(), m_ppszSounds[ 1 ] );
	}
	else
		Warning( "CFFBuildableObject::DoExplosion - ERROR - NO EXPLOSION SOUND (might want to add one)!\n" );

	if ( m_ppszGibModels[0] ) 
	{
		SpawnGibs();
	}

	// Mirv: Moved explosion damage logic into the derived classes
	DoExplosionDamage();
}

int CFFBuildableObject::OnTakeDamage( const CTakeDamageInfo &info )
{
	VPROF_BUDGET( "CFFBuildableObject::OnTakeDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Bug #0000333: Buildable Behavior (non build slot) while building
	// If we're not live yet don't take damage
	if( !m_bBuilt )
		return 0;

	// Sentry gun seems to take about 110% of damage, going to assume its the same
	// for all others for now -mirv
	CTakeDamageInfo adjustedDamage = info;
	adjustedDamage.SetDamage( adjustedDamage.GetDamage() * 1.1f );

	// Sorry trepids, not putting this one check in LUA
	// Bug #0000333: Buildable Behavior (non build slot) while building
	if(( adjustedDamage.GetAttacker() == m_hOwner.Get() ) && ( friendlyfire.GetInt() == 0 ))
		return 0;

	// Run through LUA!
	CFFLuaSC hContext;
	hContext.Push( this );
	hContext.PushRef( adjustedDamage );
	_scriptman.RunPredicates_LUA( NULL, &hContext, "buildable_ondamage" );

	// Bug #0000333: Buildable Behavior (non build slot) while building
	// Depending on the teamplay value, take damage
	
	//if( !FFGameRules()->FCanTakeDamage( ToFFPlayer( m_hOwner.Get() ), adjustedDamage.GetAttacker() ) )
	//	return 0;

	// we now pass the buildable itself instead of the owner.  The FCanTakeDamage function sorts it all out for us.
	if( !FFGameRules()->FCanTakeDamage( this, adjustedDamage.GetAttacker() ) )
		return 0;

	// DrEvil: The following is fucking wrong, don't add it back.
	// It's not as simple as checking player relationship. FCanTakeDamage compensates for sabotage and shit.
	/*if( ( FFGameRules()->PlayerRelationship( ToFFPlayer( m_hOwner.Get() ), adjustedDamage.GetAttacker() ) == GR_TEAMMATE ) && ( friendlyfire.GetInt() == 0 ) )
		return 0;*/

	// If we haven't taken any damage, no need to flicker or report to bots
	if( adjustedDamage.GetDamage() <= 0 )
		return 0;

	// Lets flicker since we're taking damage
	if( m_pFlickerer )
		m_pFlickerer->Flicker();
	
	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());

	// Jiggles: Added hint event for buildables taking damage
	//				There's a 10-second delay to avoid spamming the hint
	if( pOwner && ( gpGlobals->curtime > m_flOnTakeDamageHintTime ) )
	{
		switch( Classify() )
		{
			case CLASS_DISPENSER: 
				FF_SendHint( pOwner, ENGY_DISPDAMAGED, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_DISPDAMAGED" ); 
				m_flOnTakeDamageHintTime = gpGlobals->curtime + 10.0f; 
				break;
			case CLASS_SENTRYGUN: 
				FF_SendHint( pOwner, ENGY_SGDAMAGED, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_SGDAMAGED" ); 
				m_flOnTakeDamageHintTime = gpGlobals->curtime + 10.0f;
				break;
		}
	}
	// End hint code

	// Bug #0000772: Buildable hud information doesn't update... good
	// This will force an update of this variable for the client	
	NetworkStateChanged( ( int * )&m_iHealth );

	if ( Classify() == CLASS_MANCANNON )
	{
		CFFManCannon *pManCannon = FF_ToManCannon( this );
		pManCannon->m_iCombatState = JUMPPAD_INCOMBAT;
		pManCannon->m_flLastDamage = gpGlobals->curtime;
	}

	int res = CBaseEntity::OnTakeDamage( adjustedDamage );

	// Send hit indicator to attacker
	if ( Classify() == CLASS_SENTRYGUN ||  Classify() == CLASS_DISPENSER || Classify() == CLASS_MANCANNON )
	{
		CFFPlayer *pAttacker = ToFFPlayer( adjustedDamage.GetAttacker() );
		if( pAttacker )
		{
			pAttacker->m_flHitTime = gpGlobals->curtime;
		}
	}

	// Just extending this to send events to the bots.
	if(pOwner)
	{
		Omnibot::Notify_BuildableDamaged(pOwner, Classify(), this);
		SendStatsToBot();
	}

	return res;
}

void CFFBuildableObject::SetLocation(const char *_loc)
{
	Q_strncpy(m_BuildableLocation, _loc?_loc:"", sizeof(m_BuildableLocation));
}

#endif // CLIENT_DLL : GAME_DLL

// Utils
//-----------------------------------------------------------------------------
// Purpose: Is the entity a buildable?
//-----------------------------------------------------------------------------
bool FF_IsBuildableObject( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return( ( pEntity->Classify() == CLASS_DISPENSER ) ||
		( pEntity->Classify() == CLASS_SENTRYGUN ) ||
		( pEntity->Classify() == CLASS_DETPACK ) ||
		( pEntity->Classify() == CLASS_MANCANNON ) );
}

//-----------------------------------------------------------------------------
// Purpose: Is the entity a dispenser?
//-----------------------------------------------------------------------------
bool FF_IsDispenser( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return pEntity->Classify() == CLASS_DISPENSER;
}

//-----------------------------------------------------------------------------
// Purpose: Is the entity a sentrygun?
//-----------------------------------------------------------------------------
bool FF_IsSentrygun( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return pEntity->Classify() == CLASS_SENTRYGUN;
}

//-----------------------------------------------------------------------------
// Purpose: Is the entity a detpack?
//-----------------------------------------------------------------------------
bool FF_IsDetpack( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return pEntity->Classify() == CLASS_DETPACK;
}

//-----------------------------------------------------------------------------
// Purpose: Is the entity a man cannon?
//-----------------------------------------------------------------------------
bool FF_IsManCannon( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	return pEntity->Classify() == CLASS_MANCANNON;
}

//-----------------------------------------------------------------------------
// Purpose: Try and convert entity to a buildable
//-----------------------------------------------------------------------------
CFFBuildableObject *FF_ToBuildableObject( CBaseEntity *pEntity )

{
	if( !pEntity || !FF_IsBuildableObject( pEntity ) )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast< CFFBuildableObject * >( pEntity ) != 0 );
#endif

	return static_cast< CFFBuildableObject * >( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose: Try and convert entity to a dispenser
//-----------------------------------------------------------------------------
CFFDispenser *FF_ToDispenser( CBaseEntity *pEntity )
{
	if( !pEntity || !FF_IsDispenser( pEntity ) )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast< CFFDispenser * >( pEntity ) != 0 );
#endif

	return static_cast< CFFDispenser * >( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose: Try and convert entity to a sentrygun
//-----------------------------------------------------------------------------
CFFSentryGun *FF_ToSentrygun( CBaseEntity *pEntity )
{
	if( !pEntity || !FF_IsSentrygun( pEntity ) )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast< CFFSentryGun * >( pEntity ) != 0);
#endif

	return static_cast< CFFSentryGun * >( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose: Try and convert entity to a detpack
//-----------------------------------------------------------------------------
CFFDetpack *FF_ToDetpack( CBaseEntity *pEntity )
{
	if( !pEntity || !FF_IsDetpack( pEntity ) )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast< CFFDetpack * >( pEntity ) != 0 );
#endif

	return static_cast< CFFDetpack * >( pEntity );
}

//-----------------------------------------------------------------------------
// Purpose: Try and convert entity to a man cannon
//-----------------------------------------------------------------------------
CFFManCannon *FF_ToManCannon( CBaseEntity *pEntity )
{
	if( !pEntity || !FF_IsManCannon( pEntity ) )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast< CFFManCannon * >( pEntity ) != 0);
#endif

	return static_cast< CFFManCannon * >( pEntity );
}