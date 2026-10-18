#ifndef FF_HUD_GRENADE2TIMER_H
#define FF_HUD_GRENADE2TIMER_H

#include "ff_hud_timer_grenade.h"

class CHudGrenade2Timer : public CHudGrenadeTimer
{
public:
	DECLARE_CLASS_SIMPLE(CHudGrenade2Timer, CHudGrenadeTimer);
	CHudGrenade2Timer(const char* pElementName);
	void OnTick() override;
	KeyValues* GetDefaultStyleData() override;
};

#endif
