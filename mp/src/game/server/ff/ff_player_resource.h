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

	virtual void ResourceThink( void );
	virtual void UpdatePlayerData( void );
	virtual void Spawn( void );

protected:
	CNetworkArray( int, m_iFortPoints, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iArmor, MAX_PLAYERS + 1 );
	CNetworkArray( int, m_iClass, MAX_PLAYERS + 1 );	// |-- Mirv: Class info

	CNetworkArray( int, m_iChannel, MAX_PLAYERS + 1 );	// |-- Mirv: Channel info
	CNetworkArray( int, m_iAssists, MAX_PLAYERS + 1 );
	CNetworkVar( bool, m_bIsIntermission );
};

#endif // FF_PLAYER_RESOURCE_H
