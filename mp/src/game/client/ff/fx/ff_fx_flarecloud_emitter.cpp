// Fortress Forever
// Flare Grenade Cloud Emitter
// Snipers Secondary Grenade
// The code of Flare Grenade was based on the code of Gas Grenade
// The author of the original code of Gas Grenade is Shawn Smith (L0ki)
// The author of this edited version of the code is Hlieb
// May 23, 2026

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_flarecloud_emitter.h"
#include "ff_grenade_base.h"

ConVar flare_dietime("ffdev_flare_dietime","5.0", FCVAR_CHEAT,"How long flare cloud particles live.");
ConVar flare_scale("ffdev_flare_scale","48.0", FCVAR_CHEAT,"How big flare particles are.");
ConVar flare_alpha("ffdev_flare_alpha","0.2", FCVAR_CHEAT,"Alpha of flare particles.");
ConVar flare_moveforce("ffdev_flare_moveforce","0.025", FCVAR_CHEAT,"Strength of flare movement attractor.");

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CFlareCloud::m_hMaterial			= INVALID_MATERIAL_HANDLE;

//========================================================================
// material strings
//========================================================================
#define FLARE_PARTICLE_MATERIAL		"particle/particle_smokegrenade"

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheFlareCloud )
CLIENTEFFECT_MATERIAL( FLARE_PARTICLE_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// CFlareCloud constructor
//========================================================================
CFlareCloud::CFlareCloud( const char *pDebugName ) : CParticleEffect( pDebugName )
{
	m_pDebugName = pDebugName;

	m_flNearClipMin	= 16.0f;
	m_flNearClipMax	= 64.0f;

	m_flNextParticle = 0;
}

//========================================================================
// CFlareCloud destructor
//========================================================================
CFlareCloud::~CFlareCloud()
{
}

//========================================================================
// CFlareCloud::Create
// ----------------------
// Purpose: Creates a new instance of a CFlareCloud object
//========================================================================
CSmartPtr<CFlareCloud> CFlareCloud::Create( const char *pDebugName )
{
	CFlareCloud *pRet = new CFlareCloud( pDebugName );
	pRet->SetDynamicallyAllocated( true );
	if(m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(FLARE_PARTICLE_MATERIAL);
	return pRet;
}

//========================================================================
// AddNapalmParticle
// -----------------
// Purpose: Adds a new NapalmParticle to the system
//========================================================================
FlareParticle* CFlareCloud::AddFlareParticle( const Vector &vOrigin )
{
	FlareParticle *pRet = (FlareParticle*)AddParticle( sizeof( FlareParticle ), m_hMaterial, vOrigin );
	if ( pRet )
	{
		pRet->m_vOrigin = vOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = 10.0f;
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = 255;
		pRet->m_uchColor[1] = 20;
		pRet->m_uchColor[2] = 0;
		pRet->m_flAlpha = 1.0f;
		pRet->m_flSize = 0.3f;
	}

	return pRet;
}

//========================================================================
// SimulateParticles
// ----------
// Purpose: Handles adjusting particle properties as time progresses as
//			well as removing dead particles
//========================================================================
void CFlareCloud::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();


	FlareParticle *pParticle = (FlareParticle*)pIterator->GetFirst();
	while ( pParticle )
	{
		pParticle->m_flLifetime += timeDelta;

		float end = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float start = 1.0f - end;

		// Keep moving if we're not past out end time
		/*
		if (pParticle->m_flLifetime < pParticle->m_flEndPosTime)
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta;
		*/
		// ted - Now implements the same smoke-disturbance code as the conc particles
		//if (pParticle->m_flLifetime < pParticle->m_flEndPosTime)
		{
			Vector F(0.0f, 0.0f, 0.0f);

			/*C_BaseEntityIterator iterator;
			CBaseEntity *point = iterator.Next();
			while(point != NULL)
			{
				if((point->GetAbsOrigin() - pParticle->m_vOrigin).IsLengthLessThan(256.0f))
				{
					if(point->GetAbsVelocity().IsLengthGreaterThan(600.0f))
						AddAttractor(&F, point->GetAbsOrigin(), pParticle->m_Pos, flare_moveforce.GetFloat() * point->GetAbsVelocity().LengthSqr());
				}
				point = iterator.Next();
			}*/

			ApplyDrag(&F, pParticle->m_vVelocity, 4.0f, 20.0f);

			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
			pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		}

		pParticle->m_flAlpha = 0.8f * start + 0.0f * end;
		pParticle->m_flSize = 1.0f * start + 96.0f * end;

		if ( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );

		pParticle = (FlareParticle*)pIterator->GetNext();
	}
}

void CFlareCloud::AddAttractor(Vector *F, Vector apos, Vector ppos, float scale)
{
	Vector dir = (apos - ppos);
	dir.NormalizeInPlace();
	float dist = (apos - ppos).Length();
	if(dist > 0.00001f)
		*F += (scale / (dist/* * dist*/)) * dir;
}

void CFlareCloud::ApplyDrag(Vector *F, Vector vel, float scale, float targetvel)
{
	if(vel.IsLengthLessThan(targetvel))
		return;
	Vector dir = -vel;
	vel.NormalizeInPlace();
	float mag = vel.Length() * scale;
	*F += (dir * mag);
}


//========================================================================
// RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CFlareCloud::RenderParticles( CParticleRenderIterator *pIterator )
{
	const FlareParticle *pParticle = (const FlareParticle *)pIterator->GetFirst();
	while ( pParticle )
	{
		//Render
		Vector	tPos;

		TransformParticle( ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos );
		float sortKey = (int) tPos.z;

		Vector vColor = Vector(pParticle->m_uchColor[0] / 255.0f,
			pParticle->m_uchColor[1] / 255.0f,
			pParticle->m_uchColor[2] / 255.0f);

		RenderParticle_ColorSize(
			pIterator->GetParticleDraw(),
			tPos,
			vColor,
			pParticle->m_flAlpha,
			pParticle->m_flSize
			);

		pParticle = (const FlareParticle *)pIterator->GetNext( sortKey );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add a new bunch of particles
//-----------------------------------------------------------------------------
void CFlareCloud::Update(float flTimeDelta)
{
	// Don't create any more after this has died. Once the remaining ones have
	// been simulated this entity will automatically remove itself
	if (gpGlobals->curtime > m_flDieTime)
		return;

	if (gpGlobals->curtime < m_flNextParticle)
		return;

	m_flNextParticle = gpGlobals->curtime + 0.1f;

	//float scale = flare_scale.GetFloat();
	FlareParticle *pParticle = NULL;
	QAngle angle;
	Vector forward, right, up, velocity;

	pParticle = AddFlareParticle(m_vecOrigin);

	if(!pParticle)
		return;

	// Pick a random direction
	Vector vecDirection(RandomFloat(-1.0, 1.0f), RandomFloat(-1.0, 1.0f), RandomFloat(0, 2.0f));
	vecDirection.NormalizeInPlace();

	// And a random distance
	Vector vecFinalPos = m_vecOrigin + vecDirection * RandomFloat(50.0f, 200.0f);

	// Go as far as possible
	trace_t tr;
	UTIL_TraceLine(m_vecOrigin, vecFinalPos, MASK_SOLID, NULL, COLLISION_GROUP_DEBRIS, &tr);

	// Takes 5 seconds for a cloud to disperse
	pParticle->m_vVelocity = m_vecVelocity + (tr.endpos - m_vecOrigin) * 0.2f;

	// This is the position we're going to, even though we may not reach it
	pParticle->m_vFinalPos = tr.endpos;
}