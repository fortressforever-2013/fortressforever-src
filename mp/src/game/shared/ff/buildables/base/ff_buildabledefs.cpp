// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildabledefs.cpp
// @author Patrick O'Leary (Mulchman)
// @date ?/?/2005
// @brief This file contains the crucial variables
//			used by the buildables.
// ===============================================

#include "cbase.h"
#include "ff_buildabledefs.h"

// Array of char *'s to dispenser models
const char *g_pszFFDispenserModels[ ] =
{
	FF_DISPENSER_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFDispenserGibModels[ ] =
{
	FF_DISPENSER_GIB01_MODEL,
	FF_DISPENSER_GIB02_MODEL,
	FF_DISPENSER_GIB03_MODEL,
	FF_DISPENSER_GIB04_MODEL,
	FF_DISPENSER_GIB05_MODEL,
	FF_DISPENSER_GIB06_MODEL,
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFDispenserSounds[ ] =
{
	FF_DISPENSER_BUILD_SOUND,
	FF_DISPENSER_EXPLODE_SOUND,
	FF_DISPENSER_UNBUILD_SOUND,
	FF_DISPENSER_OMNOMNOM_SOUND,
	NULL
};

// Array of char *'s to sentrygun models
const char *g_pszFFSentryGunModels[] =
{
	FF_SENTRYGUN_MODEL, 
	FF_SENTRYGUN_MODEL_LVL2, 
	FF_SENTRYGUN_MODEL_LVL3, 
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFSentryGunGibModelsL1[] =
{
	FF_SENTRYGUN_GIB1A_MODEL,
	FF_SENTRYGUN_GIB1B_MODEL,
	FF_SENTRYGUN_GIB1C_MODEL,
	FF_SENTRYGUN_GIB1D_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

const char *g_pszFFSentryGunGibModelsL2[] =
{
	FF_SENTRYGUN_GIB2A_MODEL,
	FF_SENTRYGUN_GIB2B_MODEL,
	FF_SENTRYGUN_GIB2C_MODEL,
	FF_SENTRYGUN_GIB2D_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

const char *g_pszFFSentryGunGibModelsL3[] =
{
	FF_SENTRYGUN_GIB3A_MODEL,
	FF_SENTRYGUN_GIB3B_MODEL,
	FF_SENTRYGUN_GIB3C_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3E_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

// Array of char *'s to ALL sg gib models to precache
const char *g_pszFFSentryGunGibModels[] =
{
	FF_SENTRYGUN_GIB1A_MODEL,
	FF_SENTRYGUN_GIB1B_MODEL,
	FF_SENTRYGUN_GIB1C_MODEL,
	FF_SENTRYGUN_GIB1D_MODEL,
	FF_SENTRYGUN_GIB2A_MODEL,
	FF_SENTRYGUN_GIB2B_MODEL,
	FF_SENTRYGUN_GIB2C_MODEL,
	FF_SENTRYGUN_GIB2D_MODEL,
	FF_SENTRYGUN_GIB3A_MODEL,
	FF_SENTRYGUN_GIB3B_MODEL,
	FF_SENTRYGUN_GIB3C_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3D_MODEL,
	FF_SENTRYGUN_GIB3E_MODEL,
	FF_SENTRYGUN_GIBTRIPOD_MODEL,
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFSentryGunSounds[] =
{
	FF_SENTRYGUN_BUILD_SOUND, 
	FF_SENTRYGUN_EXPLODE_SOUND, 
	"Sentry.Fire", 
	"Sentry.Spot", 
	"Sentry.Scan", 
	"Sentry.Two", 
	"Sentry.Three", 
	"Sentry.Aim",
	FF_SENTRYGUN_UNBUILD_SOUND,
	"Spanner.HitSG",
	"Sentry.RocketFire",
	"Sentry.SabotageActivate",
	//"Sentry.SabotageFinish",
	"Sentry.CloakDetection",
	"Sentry.CloakSonar",
	"DoSpark",
	NULL
};

// Array of char *'s to dispenser models
const char *g_pszFFDetpackModels[ ] =
{
	FF_DETPACK_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFDetpackGibModels[ ] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFDetpackSounds[ ] =
{
	FF_DETPACK_BUILD_SOUND,
	FF_DETPACK_EXPLODE_SOUND,
	"Detpack.FiveSeconds",
	"Detpack.Defuse",
	NULL
};

// Array of char *'s to dispenser models
const char *g_pszFFManCannonModels[] =
{
	FF_MANCANNON_MODEL,
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFManCannonGibModels[] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFManCannonSounds[] =
{
	FF_MANCANNON_BUILD_SOUND,
	FF_MANCANNON_EXPLODE_SOUND,
	//"JumpPad.WarmUp",
	//"JumpPad.PowerDown",
	"JumpPad.Fire",
	"JumpPad.Heal",//For the healing sound on jumppads -GreenMushy
	NULL
};