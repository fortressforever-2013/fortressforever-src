// Fortress Forever
// Flare Grenade
// Snipers Secondary Grenade
// The code of Flare Grenade was based on the code of Gas Grenade
// The author of the original code of Gas Grenade is Shawn Smith (L0ki)
// The author of this edited version of the code is Hlieb
// May 23, 2026

#include "cbase.h"
#include "ff_grenade_base.h"
#include "ff_utils.h"

#define FLAREGRENADE_MODEL	"models/grenades/flares/flares_holster.mdl"
#define FLARE_SOUND			"GasGrenade.Explode"
#define FLARE_EFFECT			"GasCloud"

#ifdef CLIENT_DLL
#define CFFGrenadeFlare C_FFGrenadeFlare
#include "c_te_effect_dispatch.h"
#include "ff_fx_flarecloud_emitter.h"
#include <igameevents.h>
#include "iefx.h"
#else
#include "te_effect_dispatch.h"
#include "ff_entity_system.h"
#include "ai_basenpc.h"
#include "ff_player.h"
#endif

#define FFDEV_FLAREGRENRADIUS 125.0f

class CFFGrenadeFlare : public CFFGrenadeBase
{
public:
	DECLARE_CLASS(CFFGrenadeFlare, CFFGrenadeBase)
	DECLARE_NETWORKCLASS();

	CNetworkVar(unsigned int, m_bIsEmitting);

	virtual void Precache();
	virtual float GetShakeAmplitude(void) { return 0.0f; }
	virtual float GetGrenadeDamage() { return 0.0f; }
	virtual float GetGrenadeRadius() { return FFDEV_FLAREGRENRADIUS; }
	virtual const char* GetBounceSound() { return "GasGrenade.Bounce"; }
	virtual Class_T Classify(void) { return CLASS_GREN_FLARE; }

	virtual color32 GetColour() { color32 col = { 255, 20, 0, GREN_ALPHA_DEFAULT }; return col; }

#ifdef CLIENT_DLL
	CFFGrenadeFlare() {}
	CFFGrenadeFlare(const CFFGrenadeFlare&) {}

	virtual void ClientThink();
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual void UpdateOnRemove(void);

	CSmartPtr<CFlareCloud>	m_pFlareEmitter;

#else
	virtual void Spawn();
	virtual void Explode(trace_t* pTrace, int bitsDamageType);
	virtual void GrenadeThink();

protected:
	float m_flNextTag;
	float m_flOpenTime;

	Vector	m_vecLastPosition;
#endif
};

IMPLEMENT_NETWORKCLASS_ALIASED(FFGrenadeFlare, DT_FFGrenadeFlare)

BEGIN_NETWORK_TABLE(CFFGrenadeFlare, DT_FFGrenadeFlare)
#ifdef GAME_DLL
SendPropInt(SENDINFO(m_bIsEmitting), 1, SPROP_UNSIGNED),
#else
RecvPropInt(RECVINFO(m_bIsEmitting)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_grenade_flare, CFFGrenadeFlare);
PRECACHE_WEAPON_REGISTER(ff_grenade_flare);

#ifdef GAME_DLL

void CFFGrenadeFlare::Spawn(void)
{
	SetModel(FLAREGRENADE_MODEL);
	BaseClass::Spawn();

	m_flNextTag = 0;
	m_flOpenTime = 0.0f;

	m_bIsEmitting = 0;

	m_vecLastPosition = vec3_origin;
}

void CFFGrenadeFlare::Explode(trace_t* pTrace, int bitsDamageType)
{
	UTIL_Remove(this);
}

void CFFGrenadeFlare::GrenadeThink(void)
{
	QAngle ang = GetAbsAngles();
	ang.x = 0.0f;
	ang.z = 0.0f;
	SetAbsAngles(ang);

	if (gpGlobals->curtime > m_flDetonateTime + 10.0f)
	{
		SetThink(&CBaseGrenade::SUB_FadeOut);
	}

	if (gpGlobals->curtime > m_flDetonateTime && m_flNextTag < gpGlobals->curtime)
	{
		if (m_vecLastPosition != GetAbsOrigin())
		{
			if (!FFScriptRunPredicates(this, "onexplode", true) && (gpGlobals->curtime > m_flDetonateTime))
			{
				UTIL_Remove(this);
				return;
			}

			m_vecLastPosition = GetAbsOrigin();
		}

		m_flNextTag = gpGlobals->curtime + 0.5f;

		if (m_flOpenTime == 0.0f)
		{
			m_flOpenTime = gpGlobals->curtime;
			m_bIsEmitting = 1;
			EmitSound(FLARE_SOUND);
		}

		CBaseEntity* pEntity = NULL;
		for (CEntitySphereQuery sphere(GetAbsOrigin(), GetGrenadeRadius()); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
		{
			if (!pEntity)
				continue;

			if (!pEntity->IsPlayer())
				continue;

			CFFPlayer* pPlayer = ToFFPlayer(pEntity);
			CFFPlayer* pTagger = ToFFPlayer(GetOwnerEntity());

			if (!pPlayer || pPlayer->IsObserver() || !pTagger)
				continue;

			if (!g_pGameRules->FCanTakeDamage(pPlayer, GetOwnerEntity()))
				continue;

			pPlayer->SetRadioTagged(pTagger, gpGlobals->curtime, 15.0f, false);
		}
	}

	SetNextThink(gpGlobals->curtime);
}
#else

void CFFGrenadeFlare::UpdateOnRemove(void)
{
	if (m_bIsEmitting)
	{
		if (!!m_pFlareEmitter)
		{
			m_pFlareEmitter->SetDieTime(0.0f);
			m_pFlareEmitter = NULL;
		}
	}

	BaseClass::UpdateOnRemove();
}

void CFFGrenadeFlare::ClientThink()

{
	color32 col = GetColour();
	dlight_t* dl = effects->CL_AllocDlight(entindex());
	if (dl)
	{
		dl->origin = GetAbsOrigin() + Vector(0, 0, 1.0f);
		dl->color.r = col.r;
		dl->color.g = col.g;
		dl->color.b = col.b;
		dl->color.exponent = 4;
		dl->radius = 128.0f;
		dl->die = gpGlobals->curtime + 0.4f;
		dl->decay = dl->radius / 0.4f;
		dl->style = 0;
	}

	{
		if (m_bIsEmitting)
		{
			if (!m_pFlareEmitter)
			{
				m_pFlareEmitter = CFlareCloud::Create("FlareCloud");
				m_pFlareEmitter->SetDieTime(gpGlobals->curtime + 10.0f);
			}

			if (!!m_pFlareEmitter)
			{
				m_pFlareEmitter->UpdateEmitter(GetAbsOrigin(), GetAbsVelocity());
			}
		}
	}
}

void CFFGrenadeFlare::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
		m_pFlareEmitter = NULL;
	}
}

#endif

void CFFGrenadeFlare::Precache()
{
	//DevMsg("[Grenade Debug] CFFGrenadeFlare::Precache\n");
	PrecacheModel(FLAREGRENADE_MODEL);
	PrecacheScriptSound(FLARE_SOUND);
	PrecacheScriptSound("GasGrenade.Open");
	BaseClass::Precache();
}
