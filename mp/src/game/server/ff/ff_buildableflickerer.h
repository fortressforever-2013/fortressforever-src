// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableflickerer.h
// @author Patrick O'Leary (Mulchman)
// @date unknown
// @brief BuildableFlickerer class
//
// ===============================================

#ifndef FF_BUILDABLEFLICKERER_H
#define FF_BUILDABLEFLICKERER_H

class CFFBuildableObject;

//=============================================================================
//
//	class CFFBuildableFlickerer
//
//=============================================================================
class CFFBuildableFlickerer : public CBaseAnimating
{
	DECLARE_CLASS( CFFBuildableFlickerer, CBaseAnimating );
	DECLARE_DATADESC();

public:
	virtual void	Spawn( void );

	void			OnObjectThink( void );
	void			SetBuildable( CFFBuildableObject *pBuildable ) { m_pBuildable = pBuildable; }
	void			Flicker( void );

protected:
	CFFBuildableObject *m_pBuildable;
	float				m_flFlicker;
};

#endif // FF_BUILDABLEFLICKERER_H