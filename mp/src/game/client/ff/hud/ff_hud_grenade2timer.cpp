#include "cbase.h"

#include "ff_hud_grenade2timer.h"
#include "c_ff_player.h"

DECLARE_HUDELEMENT(CHudGrenade2Timer);

CHudGrenade2Timer::CHudGrenade2Timer(const char* pElementName)
	: CHudGrenadeTimer(
		pElementName,
		"HudGrenade2Timer",
		"#HudPanel_Grenade2Timer",
		FF_GREN_PRIMETWO)
{
}

void CHudGrenade2Timer::OnTick()
{
	BaseClass::OnTick();
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (pPlayer)
		pPlayer->m_pGrenade2Timer = this;
}

KeyValues* CHudGrenade2Timer::GetDefaultStyleData()
{
	KeyValues* kvPreset = BaseClass::GetDefaultStyleData();
	kvPreset->SetInt("x", 640 / 2);
	kvPreset->SetInt("y", 348);
	kvPreset->SetInt("alignH", ALIGN_CENTER);
	kvPreset->SetInt("alignV", ALIGN_TOP);
	return kvPreset;
}
