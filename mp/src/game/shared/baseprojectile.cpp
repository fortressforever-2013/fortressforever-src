//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "baseprojectile.h"


IMPLEMENT_NETWORKCLASS_ALIASED( BaseProjectile, DT_BaseProjectile )

BEGIN_NETWORK_TABLE( CBaseProjectile, DT_BaseProjectile )
#if !defined( CLIENT_DLL )
	SendPropEHandle( SENDINFO( m_hOriginalLauncher ) ),
#else
	RecvPropEHandle( RECVINFO( m_hOriginalLauncher ) ),
#endif // CLIENT_DLL
END_NETWORK_TABLE()


#ifndef CLIENT_DLL
IMPLEMENT_AUTO_LIST( IBaseProjectileAutoList );
#endif // !CLIENT_DLL


#ifdef TF_DLL
CBaseEntity* GetAttackerEntity( CBaseProjectile* pProjectile )
{
	CBaseEntity *pAttacker = pProjectile->GetOriginalLauncher();
	IScorer *pScorerInterface = dynamic_cast<IScorer*>( pAttacker );
	if ( pScorerInterface )
	{
		pAttacker = pScorerInterface->GetScorer();
	}
	else if ( pAttacker && pAttacker->GetOwnerEntity() )
	{
		pAttacker = pAttacker->GetOwnerEntity();
	}

	return pAttacker;
}
#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor.
//-----------------------------------------------------------------------------
CBaseProjectile::CBaseProjectile()
{
#ifdef GAME_DLL
	m_iDestroyableHitCount = 0;
	m_bCanCollideWithTeammates = false;
#endif
	m_hOriginalLauncher = NULL;

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseProjectile::~CBaseProjectile()
{
#ifdef TF_DLL
	IGameEvent *event = gameeventmanager->CreateEvent( "projectile_removed" );
	if ( event )
	{
		item_definition_index_t ownerWeaponDefIndex = INVALID_ITEM_DEF_INDEX;

		CBaseCombatWeapon *pWeapon = dynamic_cast< CBaseCombatWeapon * >( GetOriginalLauncher() );
		if ( pWeapon )
		{
			ownerWeaponDefIndex = pWeapon->GetAttributeContainer()->GetItem()->GetItemDefIndex();
		}

		CBaseEntity *pAttacker = GetAttackerEntity( this );

		if ( !pAttacker || ownerWeaponDefIndex == INVALID_ITEM_DEF_INDEX )
		{
			delete event;
			return;
		}

		event->SetInt( "attacker", pAttacker->entindex() );
		event->SetInt( "weapon_def_index", ownerWeaponDefIndex );
		event->SetInt( "num_hit", m_vecEntsHit.Count() );
		event->SetInt( "num_direct_hit", m_vecEntsDirectHit.Count() );

		gameeventmanager->FireEvent( event, true );
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseProjectile::SetLauncher( CBaseEntity *pLauncher )
{
	if ( m_hOriginalLauncher == NULL )
	{
		m_hOriginalLauncher = pLauncher;
	}

#ifdef GAME_DLL
	ResetCollideWithTeammates();
#endif // GAME_DLL
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseProjectile::Spawn()
{
	BaseClass::Spawn();

#ifdef GAME_DLL
	ResetCollideWithTeammates();
#endif // GAME_DLL
}


#ifdef GAME_DLL

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseProjectile::CollideWithTeammatesThink()
{
	m_bCanCollideWithTeammates = true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseProjectile::ResetCollideWithTeammates()
{
	// Don't collide with players on the owner's team for the first bit of our life
	m_bCanCollideWithTeammates = false;

	
	
	
	
	
	
	
	
	
	
	
		
	
		SetContextThink( &CBaseProjectile::CollideWithTeammatesThink, gpGlobals->curtime + GetCollideWithTeammatesDelay(), "CollideWithTeammates" );
}

#endif // GAME_DLL


#ifdef TF_DLL
//-----------------------------------------------------------------------------
// Purpose: Fire an event that we hit someone, and tally up how many we've hit
//			so we can fire another event when we're deleted that says how many
//			we've hit in our life
//-----------------------------------------------------------------------------
void CBaseProjectile::RecordEnemyPlayerHit( const CBaseEntity* pHitPlayer, bool bDirect )
{
	Assert( pHitPlayer->IsPlayer() );
	if ( pHitPlayer->GetTeamNumber() == GetTeamNumber() )
		return;

	// Record another hit
	if ( m_vecEntsHit.Find( pHitPlayer->entindex() ) == m_vecEntsHit.InvalidIndex() )
	{
		m_vecEntsHit.AddToTail( pHitPlayer->entindex() );
	}

	if ( bDirect )
	{
		// Record another direct hit
		if ( m_vecEntsDirectHit.Find( pHitPlayer->entindex() ) == m_vecEntsHit.InvalidIndex() )
		{
			m_vecEntsDirectHit.AddToTail( pHitPlayer->entindex() );
		}

		// Fire an event about us direct hitting
		IGameEvent *event = gameeventmanager->CreateEvent( "projectile_direct_hit" );
		if ( event )
		{
			item_definition_index_t ownerWeaponDefIndex = INVALID_ITEM_DEF_INDEX;
			CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>(GetOriginalLauncher());
			if (pWeapon)
			{
				ownerWeaponDefIndex = pWeapon->GetAttributeContainer()->GetItem()->GetItemDefIndex();
			}

			CBaseEntity *pAttacker = GetAttackerEntity( this );

			if ( !pAttacker )
			{
				delete event;
				return;
			}

			event->SetInt( "attacker", pAttacker->entindex() );
			event->SetInt( "victim", pHitPlayer->entindex() );
			event->SetInt( "weapon_def_index", ownerWeaponDefIndex );

			gameeventmanager->FireEvent( event, true );
		}
	}
}
#endif // TF_DLL

