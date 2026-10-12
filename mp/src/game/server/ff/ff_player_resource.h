//==================================//
//
// Purpose: Fortress Forever's CPlayerResource
//
// $NoKeywords: $
//==================================//

#ifndef FF_PLAYER_RESOURCE_H
#define FF_PLAYER_RESOURCE_H
#ifdef _WIN32
#pragma once
#endif

class CFFPlayerResource : public CPlayerResource
{
	DECLARE_CLASS( CFFPlayerResource, CPlayerResource );

public:
	DECLARE_SERVERCLASS();

	CFFPlayerResource();

	void ResourceThink( void );
	void Init( int iIndex );
	void UpdatePlayerData( void );
	void Spawn( void );

protected:
	void UpdateConnectedPlayer( int iIndex, CBasePlayer *pPlayer );
	void UpdateDisconnectedPlayer( int iIndex );

	CNetworkArray( int, m_iFortPoints, MAX_PLAYERS_ARRAY_SAFE );
	CNetworkArray( int, m_iArmor, MAX_PLAYERS_ARRAY_SAFE );
	CNetworkArray( int, m_iClass, MAX_PLAYERS_ARRAY_SAFE );	// |-- Mirv: Class info

	CNetworkArray( int, m_iChannel, MAX_PLAYERS_ARRAY_SAFE );	// |-- Mirv: Channel info
	CNetworkArray( int, m_iAssists, MAX_PLAYERS_ARRAY_SAFE );
	CNetworkVar( bool, m_bIsIntermission );
};

#endif // FF_PLAYER_RESOURCE_H
