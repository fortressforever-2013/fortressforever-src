// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableflickerer.cpp
// @author Patrick O'Leary (Mulchman)
// @date ?/?/2005
// @brief BuildableFlickerer class
//
// ===============================================

#include "cbase.h"
#include "ff_buildableobject.h"
#include "ff_buildableflickerer.h"
#include "baseanimating.h"
#include "tier0/vprof.h"


BEGIN_DATADESC( CFFBuildableFlickerer )
	DEFINE_THINKFUNC( OnObjectThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( ff_buildable_flickerer, CFFBuildableFlickerer );
PRECACHE_REGISTER( ff_buildable_flickerer );

//static ConVar flicker_time( "ffdev_flicker_time", "0.1", FCVAR_FF_FFDEV );
#define FLICKER_TIME 0.1f

//-----------------------------------------------------------------------------
// Purpose: Spawn a flickerer
//-----------------------------------------------------------------------------
void CFFBuildableFlickerer::Spawn( void )
{
	VPROF_BUDGET( "CFFBuildableFlickerer::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	m_flFlicker = gpGlobals->curtime;

	SetThink( &CFFBuildableFlickerer::OnObjectThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: See if it's time to un-flicker
//-----------------------------------------------------------------------------
void CFFBuildableFlickerer::OnObjectThink( void )
{
	VPROF_BUDGET( "CFFBuildableFlickerer::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// If a certain time period has gone by
	// since we last flickered we need to unflicker

	if( m_pBuildable )
	{
		// See if it's time to un-flicker
		if( ( ( m_flFlicker + FLICKER_TIME ) < gpGlobals->curtime ) /*&& ( m_pBuildable->GetRenderMode() != kRenderNormal )*/ )
		{
			//m_pBuildable->SetRenderMode( kRenderNormal );
			m_pBuildable->SetBodygroup( 1, 0 );
		}

		// Think again soon!
		SetThink( &CFFBuildableFlickerer::OnObjectThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	else
	{
		UTIL_Remove( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Flicker a buildable to indicate it's taking damage
//-----------------------------------------------------------------------------
void CFFBuildableFlickerer::Flicker( void )
{
	VPROF_BUDGET( "CFFBuildableFlickerer::Flicker", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// When flicker is called the buildable is taking damage

	if( m_pBuildable )
	{
		// Put us in a flickered "state"
		//if( m_pBuildable->GetRenderMode() == kRenderNormal )
		//{
			//m_pBuildable->SetRenderMode( kRenderTransAlpha );
			//m_pBuildable->SetRenderColorA( ( byte )110 );
			m_pBuildable->SetBodygroup( 1, 1 );
		//}

		// Note the time we flickered
		m_flFlicker = gpGlobals->curtime;
	}
	else
		UTIL_Remove( this );
}