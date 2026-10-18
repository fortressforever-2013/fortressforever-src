#include "cbase.h"

#include "ff_hud_grenade1timer.h"
#include "c_ff_player.h"

DECLARE_HUDELEMENT(CHudGrenade1Timer);

CHudGrenade1Timer::CHudGrenade1Timer(const char* pElementName)
	: CHudGrenadeTimer(
		pElementName,
		"HudGrenade1Timer",
		"#HudPanel_Grenade1Timer",
		FF_GREN_PRIMEONE)
{
}

void CHudGrenade1Timer::OnTick()
{
	BaseClass::OnTick();
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (pPlayer)
		pPlayer->m_pGrenade1Timer = this;
}

KeyValues* CHudGrenade1Timer::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	kvPreset->SetInt("x", 640 / 2);
	kvPreset->SetInt("y", 345);
	kvPreset->SetInt("alignH", ALIGN_CENTER);
	kvPreset->SetInt("alignV", ALIGN_BOTTOM);
	return kvPreset;
}
