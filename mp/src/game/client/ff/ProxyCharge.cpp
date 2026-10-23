/********************************************************************
	created:	2006/08/07
	created:	7:8:2006   22:02
	filename: 	f:\ff-svn\code\trunk_current\cl_dll\ff\ProxyCharge.cpp
	file path:	f:\ff-svn\code\trunk_current\cl_dll\ff
	file base:	ProxyCharge
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "functionproxy.h"
#include "c_ff_player.h"
#include "ff_weapon_base.h"
#include "ff_weapon_assaultcannon.h"
#include "materialsystem/imaterialvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Returns charge of current weapon
//-----------------------------------------------------------------------------
class CProxyCharge : public CResultProxy
{
public:
	virtual void OnBind(void *pC_BaseEntity);

private:
	int		m_iModifier;
};

void CProxyCharge::OnBind(void *pC_BaseEntity)
{
	Assert(m_pResult);
	if (!m_pResult)
		return;

	// Materials can bind during class changes before the player or active
	// weapon is available. Clear any charge left over from a previous bind.
	SetFloatResult(0);

	if (!pC_BaseEntity)
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	if (!pPlayer)
		return;

	C_FFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

	if (!pWeapon || pWeapon->GetWeaponID() != FF_WEAPON_ASSAULTCANNON)
		return;

	CFFWeaponAssaultCannon *pAC = (CFFWeaponAssaultCannon *) pWeapon;

	float flCharge = pAC->m_flChargeTime / FF_AC_MAXCHARGETIME;
	flCharge = 100 * clamp(flCharge, 0.01f, 1.0f);
	
	SetFloatResult(flCharge);
}

EXPOSE_INTERFACE(CProxyCharge, IMaterialProxy, "CurrentCharge" IMATERIAL_PROXY_INTERFACE_VERSION);
