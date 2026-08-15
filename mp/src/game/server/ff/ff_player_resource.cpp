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
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CFFPlayer* pPlayer = (CFFPlayer*)UTIL_PlayerByIndex(i);	// |-- Mirv: Use our class instead

		if (pPlayer && pPlayer->IsConnected())
		{
			m_iScore.Set(i, pPlayer->FragCount());
			m_iFortPoints.Set(i, pPlayer->FortPointsCount());
			m_iDeaths.Set(i, pPlayer->DeathCount());
			m_bConnected.Set(i, 1);
			m_iTeam.Set(i, pPlayer->GetTeamNumber());
			m_bAlive.Set(i, pPlayer->IsAlive() ? 1 : 0);
			m_iHealth.Set(i, MAX(0, pPlayer->GetHealth()));
			m_iArmor.Set(i, MAX(0, pPlayer->GetArmor()));
			m_iClass.Set(i, pPlayer->GetClassSlot());	// |-- Mirv: Update our class
			m_iAssists.Set(i, pPlayer->AssistsCount());

			// Don't update ping / packetloss everytime

			if (!(m_nUpdateCounter % 20))
			{
				// update ping all 20 think ticks = (20*0.1=2seconds)
				int ping, packetloss;
				UTIL_GetPlayerConnectionInfo(i, ping, packetloss);

				// calc avg for scoreboard so it's not so jittery
				ping = 0.8f * m_iPing.Get(i) + 0.2f * ping;


				m_iPing.Set(i, ping);
				// m_iPacketloss.Set( i, packetloss );

				// --> Mirv: Update the player's channel
				CFFPlayer* plyr = (CFFPlayer*)pPlayer;
				m_iChannel.Set(i, plyr->m_iChannel);
				// <-- Mirv: Update the player's channel
			}
		}
		else
		{
			m_bConnected.Set(i, 0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayerResource::Spawn( void )
{
	BaseClass::Spawn();
	for ( int i=0; i < MAX_PLAYERS+1; i++ )
	{
		m_iFortPoints.Set(i, 0);
		m_iClass.Set(i, 0);	// |-- Mirv: Current class

		m_iChannel.Set(i, 0);	// |-- Mirv: Channel info

		m_iAssists.Set(i, 0);
	}

	m_bIsIntermission = false;
}
extern bool Server_IsIntermission();

//-----------------------------------------------------------------------------
// Purpose: Wrapper for the virtual GrabPlayerData Think function
//-----------------------------------------------------------------------------
void CFFPlayerResource::ResourceThink( void )
{
	m_nUpdateCounter++;

	UpdatePlayerData();

	m_bIsIntermission = Server_IsIntermission();

	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CFFPlayerResource::UpdatePlayerData( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = ( CFFPlayer* )UTIL_PlayerByIndex( i );	// |-- Mirv: Use our class instead
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
			m_iScore.Set( i, pPlayer->FragCount() );
			m_iFortPoints.Set(i, pPlayer->FortPointsCount());
			m_iDeaths.Set( i, pPlayer->DeathCount() );
			m_bConnected.Set( i, 1 );
			m_iTeam.Set( i, pPlayer->GetTeamNumber() );
			m_bAlive.Set( i, pPlayer->IsAlive()?1:0 );
			m_iHealth.Set(i, MAX( 0, pPlayer->GetHealth() ) );
			m_iArmor.Set(i, MAX( 0, pPlayer->GetArmor() ) );
			m_iClass.Set(i, pPlayer->GetClassSlot() );	// |-- Mirv: Update our class
			m_iAssists.Set( i, pPlayer->AssistsCount() );

			// Don't update ping / packetloss everytime

			if ( !(m_nUpdateCounter%20) )
			{
				// update ping all 20 think ticks = (20*0.1=2seconds)
				int ping, packetloss;
				UTIL_GetPlayerConnectionInfo( i, ping, packetloss );
				
				// calc avg for scoreboard so it's not so jittery
				ping = 0.8f * m_iPing.Get(i) + 0.2f * ping;

				
				m_iPing.Set( i, ping );
				// m_iPacketloss.Set( i, packetloss );

				// --> Mirv: Update the player's channel
				CFFPlayer* plyr = (CFFPlayer*)pPlayer;
				m_iChannel.Set(i, plyr->m_iChannel);
				// <-- Mirv: Update the player's channel
			}
		}
		else
		{
			m_bConnected.Set( i, 0 );
		}
	}
}
