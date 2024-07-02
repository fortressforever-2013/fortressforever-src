// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildabledefs.h
// @author Patrick O'Leary (Mulchman)
// @date ?/?/2005
// @brief This file contains the crucial variables
//			used by the buildables.
// ===============================================

#ifndef FF_BUILDABLEDEFS_H
#define FF_BUILDABLEDEFS_H

// Defines
#define FF_BUILD_ERROR_NOROOM					"sprites/ff_build_noroom"
#define FF_BUILD_ERROR_TOOSTEEP					"sprites/ff_build_toosteep"
#define FF_BUILD_ERROR_TOOFAR					"sprites/ff_build_toofar"
#define FF_BUILD_ERROR_OFFGROUND				"sprites/ff_build_offground"
#define FF_BUILD_ERROR_MOVEABLE					"sprites/ff_build_moveable"
#define FF_BUILD_ERROR_NEEDAMMO					"sprites/ff_build_needammo"
#define FF_BUILD_ERROR_ALREADYBUILTSG			"sprites/ff_build_alreadybuiltsg"
#define FF_BUILD_ERROR_ALREADYBUILTDISP			"sprites/ff_build_alreadybuiltdisp"
#define FF_BUILD_ERROR_ALREADYBUILTMANCANNON	"sprites/ff_build_alreadybuiltmancannon"

enum BuildInfoResult_t
{
	BUILD_ALLOWED = 0,

	BUILD_NOROOM,			// No room, geometry/player/something in the way
	BUILD_NOPLAYER,			// Player pointer went invalid (?)
	BUILD_TOOSTEEP,			// Ground is too steep
	BUILD_TOOFAR,			// Ground is too far away
	BUILD_PLAYEROFFGROUND,	// player is not on the ground!
	BUILD_MOVEABLE,			// can't built on movable stuff
	BUILD_NEEDAMMO,			// Not Enough ammo to build
	BUILD_ALREADYBUILT,		// Already built

	BUILD_ERROR
};

//ConVar ffdev_mancannon_combatcooldown( "ffdev_mancannon_combatcooldown", "3", FCVAR_FF_FFDEV_REPLICATED );
#define MANCANNON_COMBATCOOLDOWN 3.0f

//Enum for the jumppad states - GreenMushy
enum JumpPadState_t
{
	JUMPPAD_INCOMBAT = 0,
	JUMPPAD_IDLE
};

// For ghost buildables.
//ConVar ffdev_pulsebuildable("ffdev_pulsebuildable", "0", FCVAR_FF_FFDEV_REPLICATED, "Buildable ghost slow pulse");
#define FFDEV_PULSEBUILDABLE false
//ConVar ffdev_buildabledrawonerror("ffdev_buildabledrawonerror", "1", FCVAR_FF_FFDEV_REPLICATED, "Draw the buildable when it can't be built");
#define FFDEV_BUILDABLEDRAWONERROR true
const RenderFx_t g_BuildableRenderFx = kRenderFxPulseSlowWide;

#define FF_DISPENSER_MODEL					"models/buildable/dispenser/dispenser.mdl"
#define FF_DISPENSER_GIB01_MODEL			"models/buildable/dispenser/gib1.mdl"
#define FF_DISPENSER_GIB02_MODEL			"models/buildable/dispenser/gib2.mdl"
#define FF_DISPENSER_GIB03_MODEL			"models/buildable/dispenser/gib3.mdl"
#define FF_DISPENSER_GIB04_MODEL			"models/buildable/dispenser/gib4.mdl"
#define FF_DISPENSER_GIB05_MODEL			"models/buildable/dispenser/gib5.mdl"
#define FF_DISPENSER_GIB06_MODEL			"models/buildable/dispenser/gib6.mdl"
#define FF_DISPENSER_BUILD_SOUND			"Dispenser.Build"
#define FF_DISPENSER_UNBUILD_SOUND			"Dispenser.unbuild"
#define FF_DISPENSER_EXPLODE_SOUND			"Dispenser.Explode"
#define FF_DISPENSER_OMNOMNOM_SOUND			"Dispenser.omnomnom"

#define FF_DETPACK_MODEL					"models/buildable/detpack/detpack.mdl"
#define FF_DETPACK_BUILD_SOUND				"Detpack.Build"
#define FF_DETPACK_EXPLODE_SOUND			"Detpack.Explode"

#define FF_SENTRYGUN_MODEL					"models/buildable/sg/sg_lvl1.mdl"
#define FF_SENTRYGUN_MODEL_LVL2				"models/buildable/sg/sg_lvl2.mdl"
#define FF_SENTRYGUN_MODEL_LVL3				"models/buildable/sg/sg_lvl3.mdl"
#define FF_SENTRYGUN_GIBTRIPOD_MODEL		"models/buildable/sg/gibs/tripod.mdl"
#define FF_SENTRYGUN_GIB1A_MODEL			"models/buildable/sg/gibs/1a.mdl"
#define FF_SENTRYGUN_GIB1B_MODEL			"models/buildable/sg/gibs/1b.mdl"
#define FF_SENTRYGUN_GIB1C_MODEL			"models/buildable/sg/gibs/1c.mdl"
#define FF_SENTRYGUN_GIB1D_MODEL			"models/buildable/sg/gibs/1d.mdl"
#define FF_SENTRYGUN_GIB2A_MODEL			"models/buildable/sg/gibs/2a.mdl"
#define FF_SENTRYGUN_GIB2B_MODEL			"models/buildable/sg/gibs/2b.mdl"
#define FF_SENTRYGUN_GIB2C_MODEL			"models/buildable/sg/gibs/2c.mdl"
#define FF_SENTRYGUN_GIB2D_MODEL			"models/buildable/sg/gibs/2d.mdl"
#define FF_SENTRYGUN_GIB3A_MODEL			"models/buildable/sg/gibs/3a.mdl"
#define FF_SENTRYGUN_GIB3B_MODEL			"models/buildable/sg/gibs/3b.mdl"
#define FF_SENTRYGUN_GIB3C_MODEL			"models/buildable/sg/gibs/3c.mdl"
#define FF_SENTRYGUN_GIB3D_MODEL			"models/buildable/sg/gibs/3d.mdl"
#define FF_SENTRYGUN_GIB3E_MODEL			"models/buildable/sg/gibs/3e.mdl"
#define FF_SENTRYGUN_BUILD_SOUND			"Sentry.One"
#define FF_SENTRYGUN_UNBUILD_SOUND			"Sentry.unbuild"
#define FF_SENTRYGUN_EXPLODE_SOUND			"Sentry.Explode"

#define FF_MANCANNON_MODEL					"models/items/jumppad/jumppad.mdl"
#define FF_MANCANNON_BUILD_SOUND			"JumpPad.Build"
#define FF_MANCANNON_EXPLODE_SOUND			"JumpPad.Explode"

//#define FF_SENTRYGUN_AIMSPHERE_MODEL		"models/buildable/sg/sentrygun_aimsphere.mdl"

#define FF_BUILDABLE_GENERIC_GIB_MODEL_01	"models/gibs/random/randGib1.mdl"
#define FF_BUILDABLE_GENERIC_GIB_MODEL_02	"models/gibs/random/randGib2.mdl"
#define FF_BUILDABLE_GENERIC_GIB_MODEL_03	"models/gibs/random/randGib3.mdl"
#define FF_BUILDABLE_GENERIC_GIB_MODEL_04	"models/gibs/random/randGib4.mdl"
#define FF_BUILDABLE_GENERIC_GIB_MODEL_05	"models/gibs/random/randGib5.mdl"

#define FF_BUILDABLE_TIMER_BUILD_STRING		"FF_Building"
#define FF_BUILDABLE_TIMER_DETPACK_STRING	"FF_Detpack_Primed"

#define FF_BUILD_NONE		0
#define FF_BUILD_DISPENSER	1
#define FF_BUILD_SENTRYGUN	2
#define FF_BUILD_DETPACK	3
#define FF_BUILD_MANCANNON	4

// The *_BUILD_DIST means how far in front of the player
// the object is built
#define FF_BUILD_DISP_BUILD_DIST	36.0f
#define FF_BUILD_SG_BUILD_DIST		64.0f //54.0f
#define FF_BUILD_DET_BUILD_DIST		42.0f
//#define FF_BUILD_DET_RAISE_VAL		48.0f
//#define FF_BUILD_DET_DUCKED_RAISE_VAL	24.0f

#define FF_BUILD_MC_BUILD_DIST		80.0f
//#define FF_BUILD_MC_RAISE_VAL		48.0f
//#define FF_BUILD_MC_DUCKED_RAISE_VAL	24.0f

#define FF_BUILD_DISP_STRING_LEN	256

// Using this value based off of the mins/maxs
#define FF_BUILD_DISP_HALF_WIDTH	12.0f

#define FF_DISPENSER_MINS	Vector( -12, -12, 0 )
#define FF_DISPENSER_MAXS	Vector( 12, 12, 48 )

#define FF_SENTRYGUN_MINS	Vector( -32, -32, 0 )
#define FF_SENTRYGUN_MAXS	Vector( 32, 32, 65 )

#define FF_DETPACK_MINS		Vector( -14, -14, 0 )
#define FF_DETPACK_MAXS		Vector( 14, 14, 11 )

//#define FF_MANCANNON_MINS	Vector( -14, -14, 0 )
//#define FF_MANCANNON_MAXS	Vector( 14, 14, 11 )
#define FF_MANCANNON_MINS	Vector( -54, -54, 0 )
#define FF_MANCANNON_MAXS	Vector( 54, 54, 48 )

#define FF_SOUND_BUILD		0	// Don't change these two values
#define FF_SOUND_EXPLODE	1

#define FF_BUILD_SABOTAGE_TIMEOUT 90.0f

#define FF_BUILDCOST_SENTRYGUN 130
#define FF_BUILDCOST_DISPENSER 30
#define FF_BUILDCOST_UPGRADE_SENTRYGUN 130
#define FF_REPAIRAMOUNTPERCELL_SENTRYGUN 3.5f
#define FF_REPAIRAMOUNTPERCELL_DISPENSER 5.0f

#define FF_BUILD_DEBUG_VISUALIZATIONS

// Array of char *'s to dispenser models
extern const char *g_pszFFDispenserModels[ ];

// Array of char *'s to gib models
extern const char *g_pszFFDispenserGibModels[ ];

// Array of char *'s to sounds
extern const char *g_pszFFDispenserSounds[ ];

// Array of char *'s to sentrygun models
extern const char *g_pszFFSentryGunModels[ ];

// Array of char *'s to gib models
extern const char *g_pszFFSentryGunGibModelsL1[ ];
extern const char *g_pszFFSentryGunGibModelsL2[ ];
extern const char *g_pszFFSentryGunGibModelsL3[ ];

// Array of char *'s to ALL sg gib models to precache
extern const char *g_pszFFSentryGunGibModels[ ];

// Array of char *'s to sounds
extern const char *g_pszFFSentryGunSounds[ ];

// Array of char *'s to dispenser models
extern const char *g_pszFFDetpackModels[ ];

// Array of char *'s to gib models
extern const char *g_pszFFDetpackGibModels[ ];

// Array of char *'s to sounds
extern const char *g_pszFFDetpackSounds[ ];

// Array of char *'s to dispenser models
extern const char *g_pszFFManCannonModels[ ];

// Array of char *'s to gib models
extern const char *g_pszFFManCannonGibModels[ ];

// Array of char *'s to sounds
extern const char *g_pszFFManCannonSounds[ ];

#endif