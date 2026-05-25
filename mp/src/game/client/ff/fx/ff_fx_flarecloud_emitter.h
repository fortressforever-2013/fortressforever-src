// Fortress Forever
// Flare Grenade Cloud Emitter
// Snipers Secondary Grenade
// The code of Flare Grenade was based on the code of Gas Grenade
// The author of the original code of Gas Grenade is Shawn Smith (L0ki)
// The author of this edited version of the code is Hlieb
// May 23, 2026

#ifndef FF_FX_FLARECLOUD_EMITTER_H
#define FF_FX_FLARECLOUD_EMITTER_H

class FlareParticle : public Particle
{
public:
	FlareParticle() {}

	Vector			m_vOrigin;
	Vector			m_vFinalPos;
	Vector			m_vVelocity;
	float			m_flDieTime;
	float			m_flLifetime;
	float			m_flAlpha;
	float			m_flSize;
	unsigned char	m_uchColor[3];
};

class CFlareCloud : public CParticleEffect
{
public:
	DECLARE_CLASS( CFlareCloud, CParticleEffect );

	static CSmartPtr<CFlareCloud> Create( const char *pDebugName );

	virtual void SimulateParticles	( CParticleSimulateIterator *pIterator );
	virtual void RenderParticles	( CParticleRenderIterator *pIterator );

	void AddAttractor(Vector *F, Vector apos, Vector ppos, float scale);
	void ApplyDrag(Vector *F, Vector vel, float scale, float targetvel);

	virtual void	Update( float flTimeDelta );

	FlareParticle*	AddFlareParticle( const Vector &vOrigin);

	void UpdateEmitter(const Vector &vecOrigin, const Vector &vecVelocity)
	{
		m_vecOrigin = vecOrigin;
		m_vecVelocity = vecVelocity;
	}

	void SetDieTime(float flDieTime)
	{
		m_flDieTime = flDieTime;
	}

protected:
	CFlareCloud( const char *pDebugName );
	virtual			~CFlareCloud();

private:
	CFlareCloud( const CFlareCloud & );

	float m_flNearClipMin;
	float m_flNearClipMax;

	Vector	m_vecOrigin;
	Vector	m_vecVelocity;

	float	m_flDieTime;
	float	m_flNextParticle;

	static PMaterialHandle m_hMaterial;
};

#endif