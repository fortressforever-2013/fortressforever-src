
// ff_lualib_constants.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib_constants.h"
#include "ff_lualib.h"
#include "ff_entity_system.h"
#include "ff_gamerules.h"
#include "ff_shareddefs.h"
#include "ff_info_script.h"
#include "ff_triggerclip.h"

#include "ff_weapon_base.h"

#include "ammodef.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "LuaBridge/LuaBridge.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabridge;

/////////////////////////////////////////////////////////////////////////////-
// Purpose: Convert lua ammo type (int) to game ammo type (string)
/////////////////////////////////////////////////////////////////////////////-
const char *LookupLuaAmmo( int iLuaAmmoType )
{
	switch( iLuaAmmoType )
	{
		case LUA_AMMO_SHELLS: return AMMO_SHELLS; break;
		case LUA_AMMO_CELLS: return AMMO_CELLS; break;
		case LUA_AMMO_NAILS: return AMMO_NAILS; break;
		case LUA_AMMO_ROCKETS: return AMMO_ROCKETS; break;
		case LUA_AMMO_DETPACK: return AMMO_DETPACK; break;
		case LUA_AMMO_MANCANNON: return AMMO_MANCANNON; break;
		case LUA_AMMO_GREN1: return AMMO_GREN1; break;
		case LUA_AMMO_GREN2: return AMMO_GREN2; break;
	}

	AssertMsg( false, "LookupLuaAmmo - invalid ammo type!" );

	return "";
}

/////////////////////////////////////////////////////////////////////////////-
// Purpose: Convert ammo to lua ammo
/////////////////////////////////////////////////////////////////////////////-
int LookupAmmoLua( int iAmmoType )
{
	// NOTE: this is kind of lame as in i don't think we even setup the ammo
	// type in our CTakeDamageInfo classes ... except for radio tag rifle.

	if( GetAmmoDef() )
	{
		char *pszName = GetAmmoDef()->GetAmmoOfIndex( iAmmoType )->pName;

		if( pszName && Q_strlen( pszName ) )
		{
			if( !Q_strcmp( pszName, AMMO_SHELLS ) )
				return LUA_AMMO_SHELLS;
			else if( !Q_strcmp( pszName, AMMO_CELLS ) )
				return LUA_AMMO_CELLS;
			else if( !Q_strcmp( pszName, AMMO_NAILS ) )
				return LUA_AMMO_NAILS;
			else if( !Q_strcmp( pszName, AMMO_ROCKETS ) )
				return LUA_AMMO_ROCKETS;
			else if( !Q_strcmp( pszName, AMMO_DETPACK ) )
				return LUA_AMMO_DETPACK;
			else if( !Q_strcmp( pszName, AMMO_MANCANNON ) )
				return LUA_AMMO_MANCANNON;
			// TODO: Maybe figure these in somehow?
			/*
			else if( !Q_strcmp( pszName, AMMO_GREN1 ) )
				return LUA_AMMO_GREN1;
			else if( !Q_strcmp( pszName, AMMO_GREN2 ) )
				return LUA_AMMO_GREN2;
				*/
		}
	}

	return LUA_AMMO_INVALID;
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitConstants(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)

		.beginNamespace("Ammo")
			.addProperty("kShells",		[]() -> int { return LUA_AMMO_SHELLS; })
			.addProperty("kCells",		[]() -> int { return LUA_AMMO_CELLS; })
			.addProperty("kNails",		[]() -> int { return LUA_AMMO_NAILS; })
			.addProperty("kRockets",	[]() -> int { return LUA_AMMO_ROCKETS; })
			.addProperty("kDetpack",	[]() -> int { return LUA_AMMO_DETPACK; })
			.addProperty("kManCannon",	[]() -> int { return LUA_AMMO_MANCANNON; })
			.addProperty("kGren1",		[]() -> int { return LUA_AMMO_GREN1; })
			.addProperty("kGren2",		[]() -> int { return LUA_AMMO_GREN2; })
			.addProperty("kInvalid",	[]() -> int { return LUA_AMMO_INVALID; })
		.endNamespace()

		.beginNamespace("EF")
			.addProperty("kOnfire",		[]() -> int { return LUA_EF_ONFIRE; })
			.addProperty("kConc",		[]() -> int { return LUA_EF_CONC; })
			.addProperty("kGas",		[]() -> int { return LUA_EF_GAS; })
			.addProperty("kInfect",		[]() -> int { return LUA_EF_INFECT; })
			.addProperty("kRadiotag",	[]() -> int { return LUA_EF_RADIOTAG; })
			.addProperty("kHeadshot",	[]() -> int { return LUA_EF_HEADSHOT; })
			.addProperty("kLegshot",	[]() -> int { return LUA_EF_LEGSHOT; })
			.addProperty("kTranq",		[]() -> int { return LUA_EF_TRANQ; })
			.addProperty("kACSpinup",	[]() -> int { return LUA_EF_ACSPINUP; })
			.addProperty("kSniperrifle",[]() -> int { return LUA_EF_SNIPERRIFLE; })
			.addProperty("kSpeedlua1",	[]() -> int { return LUA_EF_SPEED_LUA1; })
			.addProperty("kSpeedlua2",	[]() -> int { return LUA_EF_SPEED_LUA2; })
			.addProperty("kSpeedlua3",	[]() -> int { return LUA_EF_SPEED_LUA3; })
			.addProperty("kSpeedlua4",	[]() -> int { return LUA_EF_SPEED_LUA4; })
			.addProperty("kSpeedlua5",	[]() -> int { return LUA_EF_SPEED_LUA5; })
			.addProperty("kSpeedlua6",	[]() -> int { return LUA_EF_SPEED_LUA6; })
			.addProperty("kSpeedlua7",	[]() -> int { return LUA_EF_SPEED_LUA7; })
			.addProperty("kSpeedlua8",	[]() -> int { return LUA_EF_SPEED_LUA8; })
			.addProperty("kSpeedlua9",	[]() -> int { return LUA_EF_SPEED_LUA9; })
			.addProperty("kSpeedlua10",	[]() -> int { return LUA_EF_SPEED_LUA10; })
		.endNamespace()

		.beginNamespace("AT")
			.addProperty("kKillPlayers",			[]() -> int { return AT_KILL_PLAYERS; })
			.addProperty("kRespawnPlayers",			[]() -> int { return AT_RESPAWN_PLAYERS; })
			.addProperty("kDropItems",				[]() -> int { return AT_DROP_ITEMS; })
			.addProperty("kForceDropItems",			[]() -> int { return AT_FORCE_DROP_ITEMS; })
			.addProperty("kThrowItems",				[]() -> int { return AT_THROW_ITEMS; })
			.addProperty("kForceThrowItems",		[]() -> int { return AT_FORCE_THROW_ITEMS; })
			.addProperty("kReturnCarriedItems",		[]() -> int { return AT_RETURN_CARRIED_ITEMS; })
			.addProperty("kReturnDroppedItems",		[]() -> int { return AT_RETURN_DROPPED_ITEMS; })
			.addProperty("kRemoveRagdolls",			[]() -> int { return AT_REMOVE_RAGDOLLS; })
			.addProperty("kRemovePacks",			[]() -> int { return AT_REMOVE_PACKS; })
			.addProperty("kRemoveProjectiles",		[]() -> int { return AT_REMOVE_PROJECTILES; })
			.addProperty("kRemoveBuildables",		[]() -> int { return AT_REMOVE_BUILDABLES; })
			.addProperty("kRemoveDecals",			[]() -> int { return AT_REMOVE_DECALS; })
			.addProperty("kEndMap",					[]() -> int { return AT_END_MAP; })
			.addProperty("kReloadClips",			[]() -> int { return AT_RELOAD_CLIPS; })
			.addProperty("kAllowRespawn",			[]() -> int { return AT_ALLOW_RESPAWN; })
			.addProperty("kDisallowRespawn",		[]() -> int { return AT_DISALLOW_RESPAWN; })

			.addProperty("kChangeClassScout",		[]() -> int { return AT_CHANGECLASS_SCOUT; })
			.addProperty("kChangeClassSniper",		[]() -> int { return AT_CHANGECLASS_SNIPER; })
			.addProperty("kChangeClassSoldier",		[]() -> int { return AT_CHANGECLASS_SOLDIER; })
			.addProperty("kChangeClassDemoman",		[]() -> int { return AT_CHANGECLASS_DEMOMAN; })
			.addProperty("kChangeClassMedic",		[]() -> int { return AT_CHANGECLASS_MEDIC; })
			.addProperty("kChangeClassHWGuy",		[]() -> int { return AT_CHANGECLASS_HWGUY; })
			.addProperty("kChangeClassPyro",		[]() -> int { return AT_CHANGECLASS_PYRO; })
			.addProperty("kChangeClassSpy",			[]() -> int { return AT_CHANGECLASS_SPY; })
			.addProperty("kChangeClassEngineer",	[]() -> int { return AT_CHANGECLASS_ENGINEER; })
			.addProperty("kChangeClassCivilian",	[]() -> int { return AT_CHANGECLASS_CIVILIAN; })
			.addProperty("kChangeClassRandom",		[]() -> int { return AT_CHANGECLASS_RANDOM; })

			.addProperty("kChangeTeamBlue",			[]() -> int { return AT_CHANGETEAM_BLUE; })
			.addProperty("kChangeTeamRed",			[]() -> int { return AT_CHANGETEAM_RED; })
			.addProperty("kChangeTeamYellow",		[]() -> int { return AT_CHANGETEAM_YELLOW; })
			.addProperty("kChangeTeamGreen",		[]() -> int { return AT_CHANGETEAM_GREEN; })
			.addProperty("kChangeTeamSpectator",	[]() -> int { return AT_CHANGETEAM_SPEC; })

			.addProperty("kStopPrimedGrens",		[]() -> int { return AT_STOP_PRIMED_GRENS; })
		.endNamespace()

		.beginNamespace("Hud")
			.addProperty("kIcon",	[]() -> int { return LUA_HUD_ITEM_ICON; })
			.addProperty("kText",	[]() -> int { return LUA_HUD_ITEM_TEXT; })
			.addProperty("kTimer",	[]() -> int { return LUA_HUD_ITEM_TIMER; })
			.addProperty("kRemove",	[]() -> int { return LUA_HUD_ITEM_REMOVE; })
		.endNamespace()

		.beginNamespace("Color")
			.addProperty("kDefault",	[]() -> int { return LUA_COLOR_DEFAULT; })
			.addProperty("kBlue",		[]() -> int { return LUA_COLOR_BLUE; })
			.addProperty("kRed",		[]() -> int { return LUA_COLOR_RED; })
			.addProperty("kYellow",		[]() -> int { return LUA_COLOR_YELLOW; })
			.addProperty("kGreen",		[]() -> int { return LUA_COLOR_GREEN; })
			.addProperty("kWhite",		[]() -> int { return LUA_COLOR_WHITE; })
			.addProperty("kBlack",		[]() -> int { return LUA_COLOR_BLACK; })
			.addProperty("kOrange",		[]() -> int { return LUA_COLOR_ORANGE; })
			.addProperty("kPink",		[]() -> int { return LUA_COLOR_PINK; })
			.addProperty("kPurple",		[]() -> int { return LUA_COLOR_PURPLE; })
			.addProperty("kGrey",		[]() -> int { return LUA_COLOR_GREY; })
			.addProperty("kInvalid",	[]() -> int { return LUA_COLOR_INVALID; })
		.endNamespace()

		.beginNamespace("AllowFlags")
			.addProperty("kOnlyPlayers",	[]() -> int { return kAllowOnlyPlayers; })
			.addProperty("kBlue",			[]() -> int { return kAllowBlueTeam; })
			.addProperty("kRed",			[]() -> int { return kAllowRedTeam; })
			.addProperty("kYellow",			[]() -> int { return kAllowYellowTeam; })
			.addProperty("kGreen",			[]() -> int { return kAllowGreenTeam; })
			.addProperty("kScout",			[]() -> int { return kAllowScout; })
			.addProperty("kSniper",			[]() -> int { return kAllowSniper; })
			.addProperty("kSoldier",		[]() -> int { return kAllowSoldier; })
			.addProperty("kDemoman",		[]() -> int { return kAllowDemoman; })
			.addProperty("kMedic",			[]() -> int { return kAllowMedic; })
			.addProperty("kHwguy",			[]() -> int { return kAllowHwguy; })
			.addProperty("kPyro",			[]() -> int { return kAllowPyro; })
			.addProperty("kSpy",			[]() -> int { return kAllowSpy; })
			.addProperty("kEngineer",		[]() -> int { return kAllowEngineer; })
			.addProperty("kCivilian",		[]() -> int { return kAllowCivilian; })
		.endNamespace()

		.beginNamespace("ClipFlags")
			.addProperty("kClipTeamBlue",				[]() -> int { return LUA_CLIP_FLAG_TEAMBLUE; })
			.addProperty("kClipTeamRed",				[]() -> int { return LUA_CLIP_FLAG_TEAMRED; })
			.addProperty("kClipTeamYellow",				[]() -> int { return LUA_CLIP_FLAG_TEAMYELLOW; })
			.addProperty("kClipTeamGreen",				[]() -> int { return LUA_CLIP_FLAG_TEAMGREEN; })
			.addProperty("kClipAllPlayers",				[]() -> int { return LUA_CLIP_FLAG_PLAYERS; })
			.addProperty("kClipAllGrenades",			[]() -> int { return LUA_CLIP_FLAG_GRENADES; })
			.addProperty("kClipAllProjectiles",			[]() -> int { return LUA_CLIP_FLAG_PROJECTILES; })
			.addProperty("kClipAllBullets",				[]() -> int { return LUA_CLIP_FLAG_BULLETS; })
			.addProperty("kClipAllBuildables",			[]() -> int { return LUA_CLIP_FLAG_BUILDABLES; })
			.addProperty("kClipAllBuildableWeapons",	[]() -> int { return LUA_CLIP_FLAG_BUILDABLEWEAPONS; })
			.addProperty("kClipAllBackpacks",			[]() -> int { return LUA_CLIP_FLAG_BACKPACKS; })
			.addProperty("kClipAllInfoScripts",			[]() -> int { return LUA_CLIP_FLAG_INFOSCRIPTS; })
			.addProperty("kClipAllSpawnTurrets",		[]() -> int { return LUA_CLIP_FLAG_SPAWNTURRETS; })
			.addProperty("kClipAllNonPlayers",			[]() -> int { return LUA_CLIP_FLAG_NONPLAYERS; })
			.addProperty("kClipPlayers",				[]() -> int { return LUA_CLIP_FLAG_PLAYERSBYTEAM; })
			.addProperty("kClipGrenades",				[]() -> int { return LUA_CLIP_FLAG_GRENADESBYTEAM; })
			.addProperty("kClipProjectiles",			[]() -> int { return LUA_CLIP_FLAG_PROJECTILESBYTEAM; })
			.addProperty("kClipBullets",				[]() -> int { return LUA_CLIP_FLAG_BULLETSBYTEAM; })
			.addProperty("kClipBuildables",				[]() -> int { return LUA_CLIP_FLAG_BUILDABLESBYTEAM; })
			.addProperty("kClipBuildableWeapons",		[]() -> int { return LUA_CLIP_FLAG_BUILDABLEWEAPONSBYTEAM; })
			.addProperty("kClipBackpacks",				[]() -> int { return LUA_CLIP_FLAG_BACKPACKSBYTEAM; })
			.addProperty("kClipInfoScripts",			[]() -> int { return LUA_CLIP_FLAG_INFOSCRIPTS; })	// TODO: Make this ByTeam once it works
			.addProperty("kClipSpawnTurrets",			[]() -> int { return LUA_CLIP_FLAG_SPAWNTURRETSBYTEAM; })
			.addProperty("kClipNonPlayers",				[]() -> int { return LUA_CLIP_FLAG_NONPLAYERSBYTEAM; })
			.addProperty("kClipPlayersByTeam",			[]() -> int { return LUA_CLIP_FLAG_PLAYERSBYTEAM; })
			.addProperty("kClipGrenadesByTeam",			[]() -> int { return LUA_CLIP_FLAG_GRENADESBYTEAM; })
			.addProperty("kClipProjectilesByTeam",		[]() -> int { return LUA_CLIP_FLAG_PROJECTILESBYTEAM; })
			.addProperty("kClipBulletsByTeam",			[]() -> int { return LUA_CLIP_FLAG_BULLETSBYTEAM; })
			.addProperty("kClipBuildablesByTeam",		[]() -> int { return LUA_CLIP_FLAG_BUILDABLESBYTEAM; })
			.addProperty("kClipBuildableWeaponsByTeam",	[]() -> int { return LUA_CLIP_FLAG_BUILDABLEWEAPONSBYTEAM; })
			.addProperty("kClipBackpacksByTeam",		[]() -> int { return LUA_CLIP_FLAG_BACKPACKSBYTEAM; })
			//.addProperty("kClipInfoScriptsByTeam",	[]() -> int { return LUA_CLIP_FLAG_INFOSCRIPTSBYTEAM; })
			.addProperty("kClipSpawnTurretsByTeam",		[]() -> int { return LUA_CLIP_FLAG_SPAWNTURRETSBYTEAM; })
			.addProperty("kClipNonPlayersByTeam",		[]() -> int { return LUA_CLIP_FLAG_NONPLAYERSBYTEAM; })
		.endNamespace()

		.beginNamespace("SpecMode")
			.addProperty("kNone",		[]() -> int { return 	(int) OBS_MODE_NONE; })
			.addProperty("kDeathCam",	[]() -> int { return	(int) OBS_MODE_DEATHCAM; })
			.addProperty("kFixed",		[]() -> int { return 	(int) OBS_MODE_FIXED; })				
			.addProperty("kInEye",		[]() -> int { return 	(int) OBS_MODE_IN_EYE; })
			.addProperty("kChase",		[]() -> int { return 	(int) OBS_MODE_CHASE; })
			.addProperty("kRoaming",	[]() -> int { return	(int) OBS_MODE_ROAMING; })
		.endNamespace()

		.beginNamespace("RenderFx")
			.addProperty("kPulseSlow",		[]() -> int { return (int) kRenderFxPulseSlow; })
			.addProperty("kPulseFast",		[]() -> int { return (int) kRenderFxPulseFast; })
			.addProperty("kPulseSlowWide",	[]() -> int { return (int) kRenderFxPulseSlowWide; })				
			.addProperty("kPulseFastWide",	[]() -> int { return (int) kRenderFxPulseFastWide; })
			.addProperty("kFadeSlow",		[]() -> int { return (int) kRenderFxFadeSlow; })
			.addProperty("kFadeFast",		[]() -> int { return (int) kRenderFxFadeFast; })
			.addProperty("kSolidSlow",		[]() -> int { return (int) kRenderFxSolidSlow; })
			.addProperty("kSolidFast",		[]() -> int { return (int) kRenderFxSolidFast; })
			.addProperty("kStrobeSlow",		[]() -> int { return (int) kRenderFxStrobeSlow; })				
			.addProperty("kStrobeFast",		[]() -> int { return (int) kRenderFxStrobeFast; })
			.addProperty("kStrobeFaster",	[]() -> int { return (int) kRenderFxStrobeFaster; })
			.addProperty("kFlickerSlow",	[]() -> int { return (int) kRenderFxFlickerSlow; })
			.addProperty("kFlickerFast",	[]() -> int { return (int) kRenderFxFlickerFast; })
			.addProperty("kNoDissipation",	[]() -> int { return (int) kRenderFxNoDissipation; })
			.addProperty("kDistort",		[]() -> int { return (int) kRenderFxDistort; })				
			.addProperty("kHologram",		[]() -> int { return (int) kRenderFxHologram; })
			.addProperty("kExplode",		[]() -> int { return (int) kRenderFxExplode; })
			.addProperty("kGlowShell",		[]() -> int { return (int) kRenderFxGlowShell; })
		.endNamespace();
};
