//==================================//
//
// Purpose: Fortress Forever's CPlayerResource
//
// $NoKeywords: $
//==================================//
#include "cbase.h"
#include "ff_player.h"
#include "player_resource.h"
#include "ff_player_resource.h"
#include "ff_gamerules.h"
#include <coordsize.h>

// Datatable
IMPLEMENT_SERVERCLASS_ST( CFFPlayerResource, DT_FFPlayerResource )
	SendPropArray3(SENDINFO_ARRAY3(m_iScore), SendPropInt(SENDINFO_ARRAY(m_iScore), 15)),	// |- Mirv: Upped transmission bits from 12->15
	SendPropArray3(SENDINFO_ARRAY3(m_iFortPoints), SendPropInt(SENDINFO_ARRAY(m_iFortPoints), 20)),	// |- Shock: Upped transmission bits from 15->20 (needs to be big!)
	SendPropArray3(SENDINFO_ARRAY3(m_iArmor), SendPropInt(SENDINFO_ARRAY(m_iArmor), 9, SPROP_UNSIGNED)),
	SendPropArray3(SENDINFO_ARRAY3(m_iClass), SendPropInt(SENDINFO_ARRAY(m_iClass), 5)),	// |-- Mirv: Current class

	SendPropArray3(SENDINFO_ARRAY3(m_iChannel), SendPropInt(SENDINFO_ARRAY(m_iChannel), 4)), // |-- Mirv: Channel info

	SendPropArray3(SENDINFO_ARRAY3(m_iAssists), SendPropInt(SENDINFO_ARRAY(m_iAssists), 12)),

	SendPropBool(SENDINFO(m_bIsIntermission)),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( ff_player_manager, CFFPlayerResource );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFPlayerResource::CFFPlayerResource( void )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayerResource::UpdatePlayerData( void )
{
	BaseClass::UpdatePlayerData();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayerResource::UpdateConnectedPlayer( int iIndex, CBasePlayer *pPlayer )
{
	BaseClass::UpdateConnectedPlayer( iIndex, pPlayer );

	CFFPlayer *pFFPlayer = ToFFPlayer( pPlayer );	// |-- Mirv: Use our class instead

	m_iFortPoints.Set( iIndex, pFFPlayer->FortPointsCount() );
	m_iArmor.Set( iIndex, MAX( 0, pFFPlayer->GetArmor() ) );
	m_iClass.Set( iIndex, pFFPlayer->GetClassSlot() );	// |-- Mirv: Update our class
	m_iAssists.Set( iIndex, pFFPlayer->AssistsCount() );

	// Don't update the player's channel everytime
	if ( !(m_nUpdateCounter%20) )
	{
		// --> Mirv: Update the player's channel
		m_iChannel.Set( iIndex, pFFPlayer->m_iChannel );
		// <-- Mirv: Update the player's channel
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayerResource::UpdateDisconnectedPlayer( int iIndex )
{
	BaseClass::UpdateDisconnectedPlayer( iIndex );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayerResource::Spawn( void )
{
	BaseClass::Spawn();
}
extern bool Server_IsIntermission();

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayerResource::Init( int iIndex )
{
	BaseClass::Init( iIndex );

	m_iFortPoints.Set( iIndex, 0 );
	m_iClass.Set( iIndex, 0 );	// |-- Mirv: Current class
	m_iChannel.Set( iIndex, 0 );	// |-- Mirv: Channel info
	m_iAssists.Set( iIndex, 0 );

	m_bIsIntermission = false;
}


//-----------------------------------------------------------------------------
// Purpose: Wrapper for the virtual GrabPlayerData Think function
//-----------------------------------------------------------------------------
void CFFPlayerResource::ResourceThink( void )
{
	BaseClass::ResourceThink();

	m_bIsIntermission = Server_IsIntermission();
}
