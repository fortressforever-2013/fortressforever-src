#ifndef FF_HUD_GRENADE1TIMER_H
#define FF_HUD_GRENADE1TIMER_H

#include "ff_hud_timer_grenade.h"

class CHudGrenade1Timer : public CHudGrenadeTimer
{
public:
	DECLARE_CLASS_SIMPLE(CHudGrenade1Timer, CHudGrenadeTimer);
	CHudGrenade1Timer(const char* pElementName);
	void OnTick() override;
	KeyValues* GetDefaultStyleData() override;
};

#endif
