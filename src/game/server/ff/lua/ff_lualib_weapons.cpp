
// ff_lualib_weapons.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_grenade_base.h"

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

//---------------------------------------------------------------------------
void CFFLuaLib::InitWeapons(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)

		// CFFGrenadeBase
		.deriveClass<CFFGrenadeBase, CBaseEntity>("FF_Grenade")
			.addFunction("Type",		&CFFGrenadeBase::GetGrenId)
		.endClass()

		// Grenade types enum
		.beginNamespace("Grenade")
			.addProperty("kNormal",		[]() -> int { return CLASS_GREN; })
			.addProperty("kNail",		[]() -> int { return CLASS_GREN_NAIL; })
			.addProperty("kMirv",		[]() -> int { return CLASS_GREN_MIRV; })
			.addProperty("kMirvlet",	[]() -> int { return CLASS_GREN_MIRVLET; })
			.addProperty("kConc",		[]() -> int { return CLASS_GREN_CONC; })
			.addProperty("kNapalm",		[]() -> int { return CLASS_GREN_NAPALM; })
			.addProperty("kGas",		[]() -> int { return CLASS_GREN_GAS; })
			.addProperty("kEmp",		[]() -> int { return CLASS_GREN_EMP; })
			.addProperty("kSlowfield",	[]() -> int { return CLASS_GREN_SLOWFIELD; })
			.addProperty("kLaser",		[]() -> int { return CLASS_GREN_LASER; })
		.endNamespace()

		// CTakeDamageInfo
		.beginClass<CTakeDamageInfo>("DamageInfo")
			.addFunction("GetAttacker",			&CTakeDamageInfo::GetAttacker)
			.addFunction("GetInflictor",		&CTakeDamageInfo::GetInflictor)
			.addFunction("GetDamage",			&CTakeDamageInfo::GetDamage)
			.addFunction("GetDamageForce",		&CTakeDamageInfo::GetDamageForce)
			.addFunction("GetDamagePosition",	&CTakeDamageInfo::GetDamagePosition)
			.addFunction("GetDamageType",		&CTakeDamageInfo::GetDamageType)
			.addFunction("GetAmmoType",			&CTakeDamageInfo::GetAmmoTypeLua)
			.addFunction("SetDamage",			&CTakeDamageInfo::SetDamage)
			.addFunction("SetDamageForce",		&CTakeDamageInfo::SetDamageForce)
			.addFunction("ScaleDamage",			&CTakeDamageInfo::ScaleDamage)
		.endClass()

		// Damage Types Enum
		.beginNamespace("Damage")
			.addProperty("kCrush",				[]() -> int { return DMG_CRUSH; })
			.addProperty("kBullet",				[]() -> int { return DMG_BULLET; })
			.addProperty("kSlash",				[]() -> int { return DMG_SLASH; })
			.addProperty("kBurn",				[]() -> int { return DMG_BURN; })
			.addProperty("kVehicle",			[]() -> int { return DMG_VEHICLE; })
			.addProperty("kFall",				[]() -> int { return DMG_FALL; })
			.addProperty("kBlast",				[]() -> int { return DMG_BLAST; })
			.addProperty("kClub",				[]() -> int { return DMG_CLUB; })
			.addProperty("kShock",				[]() -> int { return DMG_SHOCK; })
			.addProperty("kSonic",				[]() -> int { return DMG_SONIC; })
			.addProperty("kEnergyBeam",			[]() -> int { return DMG_ENERGYBEAM; })
			.addProperty("kPreventPhysForce",	[]() -> int { return DMG_PREVENT_PHYSICS_FORCE; })
			.addProperty("kNeverGib",			[]() -> int { return DMG_NEVERGIB; })
			.addProperty("kAlwaysGib",			[]() -> int { return DMG_ALWAYSGIB; })
			.addProperty("kDrown",				[]() -> int { return DMG_DROWN; })
			.addProperty("kTimeBased",			[]() -> int { return DMG_TIMEBASED; })
			.addProperty("kParalyze",			[]() -> int { return DMG_PARALYZE; })
			.addProperty("kNerveGas",			[]() -> int { return DMG_NERVEGAS; })
			.addProperty("kPoison",				[]() -> int { return DMG_POISON; })
			.addProperty("kRadiation",			[]() -> int { return DMG_RADIATION; })
			.addProperty("kDrownRecover",		[]() -> int { return DMG_DROWNRECOVER; })
			.addProperty("kAcid",				[]() -> int { return DMG_ACID; })
			.addProperty("kSlowBurn",			[]() -> int { return DMG_SLOWBURN; })
			.addProperty("kRemoveNoRagdoll",	[]() -> int { return DMG_REMOVENORAGDOLL; })
			.addProperty("kPhysgun",			[]() -> int { return DMG_PHYSGUN; })
			.addProperty("kPlasma",				[]() -> int { return DMG_PLASMA; })
			.addProperty("kAirboat",			[]() -> int { return DMG_AIRBOAT; })
			.addProperty("kDissolve",			[]() -> int { return DMG_DISSOLVE; })
			.addProperty("kBlastSurface",		[]() -> int { return DMG_BLAST_SURFACE; })
			.addProperty("kDirect",				[]() -> int { return DMG_DIRECT; })
			.addProperty("kBuckshot",			[]() -> int { return DMG_BUCKSHOT; })
			.addProperty("kGibCorpse",			[]() -> int { return DMG_GIB_CORPSE; })
			.addProperty("kShownHud",			[]() -> int { return DMG_SHOWNHUD; })
			.addProperty("kNoPhysForce",		[]() -> int { return DMG_NO_PHYSICS_FORCE; })
		.endNamespace();
};
