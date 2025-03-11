
// ff_lualib_buildables.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_buildableobject.h"
#include "ff_buildable_sentrygun.h"
#include "ff_buildable_dispenser.h"
#include "ff_buildable_detpack.h"

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

namespace FFLib
{
	// helper functions to avoid manadatory bEmitSound parameter
	// Luabind doesn't handle default parameter values well
	void SetSGLevel(CFFSentryGun *pSentryGun, int iLevel)
	{
		pSentryGun->SetLevel(iLevel, false);
	}
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitBuildables(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)
		.deriveClass<CFFBuildableObject, CBaseEntity>("BaseBuildable")
			.addFunction("GetTeamId",			&CFFBuildableObject::GetTeamNumber)
			.addFunction("GetOwner",			&CFFBuildableObject::GetOwnerPlayer)
			.addFunction("GetTeam",				&CFFBuildableObject::GetOwnerTeam)
			//.addFunction("GetTeamId",			&CFFBuildableObject::GetOwnerTeamId)
		.endClass()

		// Dispenser
		.deriveClass<CFFDispenser, CFFBuildableObject>("Dispenser")
		.endClass()
		
		// Sentrygun
		.deriveClass<CFFSentryGun, CFFBuildableObject>("Sentrygun")
			.addFunction("GetLevel",			&CFFSentryGun::GetLevel)
			.addFunction("SetLevel",			&FFLib::SetSGLevel)
			.addFunction("Upgrade",				&CFFSentryGun::Upgrade)
			.addFunction("Repair",				&CFFSentryGun::Repair)
			.addFunction("AddAmmo",				&CFFSentryGun::AddAmmo)
			.addFunction("RocketPosition",		&CFFSentryGun::RocketPosition)
			.addFunction("MuzzlePosition",		&CFFSentryGun::MuzzlePosition)
			.addFunction("GetRockets",			&CFFSentryGun::GetRockets)
			.addFunction("GetShells",			&CFFSentryGun::GetShells)
			.addFunction("GetHealth",			&CFFSentryGun::GetHealth)
			.addFunction("SetRockets",			&CFFSentryGun::SetRockets)
			.addFunction("SetShells",			&CFFSentryGun::SetShells)
			.addFunction("SetHealth",			&CFFSentryGun::SetHealth)
			.addFunction("GetMaxRockets",		&CFFSentryGun::GetMaxRockets)
			.addFunction("GetMaxShells",		&CFFSentryGun::GetMaxShells)
			.addFunction("GetMaxHealth",		&CFFSentryGun::GetMaxHealth)
			.addFunction("SetFocusPoint",		&CFFSentryGun::SetFocusPoint)
			.addFunction("GetEnemy",			&CFFSentryGun::GetEnemy)
			.addFunction("SetEnemy",			&CFFSentryGun::SetEnemy)
			.addFunction("GetVecAiming",		&CFFSentryGun::GetVecAiming)
			.addFunction("GetVecGoal",			&CFFSentryGun::GetVecGoal)

			.addFunction("Shoot",
				overload<>(&CFFSentryGun::Shoot)
			)

			.addFunction("ShootRocket",
				overload<>(&CFFSentryGun::ShootRocket)
			)

		.endClass()
		
		// Detpack
		.deriveClass<CFFDetpack, CFFBuildableObject>("Detpack")
			.addFunction("GetFuseTime",			&CFFDetpack::GetFuseTime)
			.addFunction("GetDetonateTime",		&CFFDetpack::GetDetonateTime)
			.addFunction("LastFiveSeconds",		&CFFDetpack::LastFiveSeconds)
		.endClass()

		.beginNamespace("BuildableTypes")
			.addProperty("kNone",		[]() -> int { return FF_BUILD_NONE;		 })
			.addProperty("kDispenser",	[]() -> int { return FF_BUILD_DISPENSER; })
			.addProperty("kSentryGun",	[]() -> int { return FF_BUILD_SENTRYGUN; })
			.addProperty("kDetpack",	[]() -> int { return FF_BUILD_DETPACK;	 })
			.addProperty("kJumpPad",	[]() -> int { return FF_BUILD_MANCANNON; })
			// For consistency with other names, return even though jump pad should be preferred everywhere
			.addProperty("kManCannon",	[]() -> int { return FF_BUILD_MANCANNON; })
		.endNamespace();
};
