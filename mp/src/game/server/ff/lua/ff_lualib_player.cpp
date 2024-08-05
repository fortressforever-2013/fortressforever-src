
// ff_lualib_player.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_player.h"

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

/// tostring implemenation for CFFPlayer
std::string FFPlayerToString(const CFFPlayer& player)
{
	std::stringstream stream;
	stream << const_cast<CFFPlayer&>(player).GetClassname() << ":" << const_cast<CFFPlayer&>(player).GetPlayerName() << ":" << player.entindex();
	return stream.str();
}

namespace FFLib
{
	// helper function because CFFPlayer::FlashlightIsOn returns an int rather than a bool
	bool IsFlashlightOn(CFFPlayer *pPlayer)
	{
		return pPlayer->FlashlightIsOn() != 0;
	}

	// helper function for MarkRadioTag, so it knows when it's called from LUA
	// this basically allows for radio tag to still work if pWhoTaggedMe is null.
	void SetRadioTagged(CFFPlayer* pPlayer, CFFPlayer* pWhoTaggedMe, float flStartTime, float flDuration)
	{
		pPlayer->SetRadioTagged(pWhoTaggedMe, flStartTime, flDuration, true);
	}
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitPlayer(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)

		.deriveClass<CBasePlayer, CBaseEntity>("BasePlayer")
		.endClass()

		.deriveClass<CFFPlayer, CBasePlayer>("FF_Player")
			.addFunction("__tostring", &FFPlayerToString)

			.addFunction("MaxSpeed", &CFFPlayer::MaxSpeed)
			.addFunction("GetSpeed", &CFFPlayer::LuaGetMovementSpeed)
			.addFunction("AddAmmo", &CFFPlayer::LuaAddAmmo)
			.addFunction("AddArmor", &CFFPlayer::AddArmor)
			.addFunction("AddFrags", &CFFPlayer::IncrementFragCount)
			.addFunction("AddFortPoints", &CFFPlayer::AddFortPoints)

			/*.addFunction("AddAction",
				overload<CFFPlayer*, const char*, const char*>(&CFFPlayer::AddAction),
				overload<CFFPlayer*, const char*, const char*, Vector&, const char*>(&CFFPlayer::AddAction)
			)*/
			
			//.addFunction("AddStat",				&CFFPlayer::AddStat)

			.addFunction("AddHealth",
				overload<int>(&CFFPlayer::LuaAddHealth),
				overload<int, bool>(&CFFPlayer::LuaAddHealth)
			)

			.addFunction("GetClass", &CFFPlayer::GetClassSlot)
			.addFunction("GetName", &CFFPlayer::GetPlayerName)
			.addFunction("GetArmor", &CFFPlayer::GetArmor)
			.addFunction("GetMaxArmor", &CFFPlayer::GetMaxArmor)
			.addFunction("GetArmorAbsorption", &CFFPlayer::GetArmorAbsorption)
			.addFunction("GetHealth", &CFFPlayer::GetHealth)
			.addFunction("GetMaxHealth", &CFFPlayer::GetMaxHealth)
			.addFunction("GetFortPoints", &CFFPlayer::FortPointsCount)
			.addFunction("GetFrags", &CFFPlayer::FragCount)
			.addFunction("GetDeaths", &CFFPlayer::DeathCount)
			.addFunction("HasItem", &CFFPlayer::HasItem)
			.addFunction("IsFeetDeepInWater", &CFFPlayer::IsFeetDeepInWater)
			.addFunction("IsInNoBuild", &CFFPlayer::IsInNoBuild)
			.addFunction("IsUnderWater", &CFFPlayer::IsUnderWater)
			.addFunction("IsWaistDeepInWater", &CFFPlayer::IsWaistDeepInWater)
			.addFunction("IsInAttack1", &CFFPlayer::IsInAttack1)
			.addFunction("IsInAttack2", &CFFPlayer::IsInAttack2)
			.addFunction("IsInUse", &CFFPlayer::IsInUse)
			.addFunction("IsInJump", &CFFPlayer::IsInJump)
			.addFunction("IsInForward", &CFFPlayer::IsInForward)
			.addFunction("IsInBack", &CFFPlayer::IsInBack)
			.addFunction("IsInMoveLeft", &CFFPlayer::IsInMoveLeft)
			.addFunction("IsInMoveRight", &CFFPlayer::IsInMoveRight)
			.addFunction("IsInLeft", &CFFPlayer::IsInLeft)
			.addFunction("IsInRight", &CFFPlayer::IsInRight)
			.addFunction("IsInRun", &CFFPlayer::IsInRun)
			.addFunction("IsInReload", &CFFPlayer::IsInReload)
			.addFunction("IsInSpeed", &CFFPlayer::IsInSpeed)
			.addFunction("IsInWalk", &CFFPlayer::IsInWalk)
			.addFunction("IsInZoom", &CFFPlayer::IsInZoom)
			.addFunction("IsOnGround", &CFFPlayer::IsOnGround)

			.addFunction("IsInAir",
				overload<>(&CFFPlayer::IsInAir),
				overload<float>(&CFFPlayer::IsInAir)
			)

			.addFunction("IsDucking", &CFFPlayer::IsDucking)
			.addFunction("IsBot", &CFFPlayer::IsBot)
			.addFunction("IsFlashlightOn", &FFLib::IsFlashlightOn)
			.addFunction("MarkRadioTag", &FFLib::SetRadioTagged)
			//.addFunction("RemoveAmmo",			(void(CFFPlayer::*)(int, const char*))&CFFPlayer::RemoveAmmo)
			.addFunction("OwnsWeaponType", &CFFPlayer::LuaOwnsWeaponType)
			.addFunction("RemoveAllAmmo", &CFFPlayer::LuaRemoveAllAmmo)
			.addFunction("RemoveAmmo", &CFFPlayer::LuaRemoveAmmo)
			.addFunction("RemoveArmor", &CFFPlayer::RemoveArmor)
			.addFunction("RemoveLocation", &CFFPlayer::RemoveLocation)
			.addFunction("Respawn", &CFFPlayer::KillAndRemoveItems)
			.addFunction("RemoveBuildables", &CFFPlayer::RemoveBuildables)
			.addFunction("RemoveProjectiles", &CFFPlayer::RemoveProjectiles)
			.addFunction("RemoveItems", &CFFPlayer::RemoveItems)
			.addFunction("SetDisguisable", &CFFPlayer::SetDisguisable)
			.addFunction("IsDisguisable", &CFFPlayer::GetDisguisable)
			.addFunction("SetCloakable", &CFFPlayer::SetCloakable)
			.addFunction("IsCloakable", &CFFPlayer::IsCloakable)
			.addFunction("SetRespawnable", &CFFPlayer::SetRespawnable)
			.addFunction("IsRespawnable", &CFFPlayer::IsRespawnable)
			.addFunction("SetLocation", &CFFPlayer::SetLocation)
			.addFunction("GetLocation", &CFFPlayer::GetLocation)
			.addFunction("GetLocationTeam", &CFFPlayer::GetLocationTeam)
			.addFunction("SetRespawnDelay", &CFFPlayer::LUA_SetPlayerRespawnDelay)
			//.addFunction("InstaSwitch",			&CFFPlayer::InstaSwitch) -- doing this as part of ApplyToPlayer()
			.addFunction("GetActiveWeaponName", &CFFPlayer::GetActiveWeaponName)
			//.addFunction("GiveWeapon",			(void(CFFPlayer::*)(const char*))&CFFPlayer::LuaGiveWeapon)
			.addFunction("GiveWeapon", &CFFPlayer::LuaGiveWeapon)
			.addFunction("RemoveWeapon", &CFFPlayer::TakeNamedItem)
			.addFunction("RemoveAllWeapons", &CFFPlayer::LuaRemoveAllWeapons)
			.addFunction("IsFeigned", &CFFPlayer::IsCloaked)	// need to remove this one eventually!
			.addFunction("IsCloaked", &CFFPlayer::IsCloaked)
			.addFunction("IsDisguised", &CFFPlayer::IsDisguised)
			.addFunction("GetDisguisedClass", &CFFPlayer::GetDisguisedClass)
			.addFunction("GetDisguisedTeam", &CFFPlayer::GetDisguisedTeam)
			.addFunction("SetDisguise", &CFFPlayer::SetDisguise)
			.addFunction("ResetDisguise", &CFFPlayer::ResetDisguise)
			.addFunction("AddEffect", &CFFPlayer::LuaAddEffect)
			.addFunction("IsEffectActive", &CFFPlayer::LuaIsEffectActive)
			.addFunction("RemoveEffect", &CFFPlayer::LuaRemoveEffect)
			.addFunction("GetSteamID", &CFFPlayer::GetSteamID)
			.addFunction("GetPing", &CFFPlayer::GetPing)
			.addFunction("GetPacketloss", &CFFPlayer::GetPacketloss)
			.addFunction("IsAlive", &CFFPlayer::IsAlive)
			.addFunction("Spectate", &CFFPlayer::StartObserverMode)
			.addFunction("Freeze", &CFFPlayer::LuaFreezePlayer)
			.addFunction("LockInPlace", &CFFPlayer::LuaLockPlayerInPlace)
			.addFunction("IsFrozen", &CFFPlayer::LuaIsPlayerFrozen)
			.addFunction("GetSpeedMod", &CFFPlayer::GetLaggedMovementValue)
			.addFunction("SpeedMod", &CFFPlayer::SetLaggedMovementValue)
			.addFunction("ReloadClips", &CFFPlayer::ReloadClips)
			.addFunction("IsGrenade1Primed", &CFFPlayer::IsGrenade1Primed)
			.addFunction("IsGrenade2Primed", &CFFPlayer::IsGrenade2Primed)
			.addFunction("IsGrenadePrimed", &CFFPlayer::IsGrenadePrimed)

			.addFunction("GetAmmoInClip", 
				overload<>(&CFFPlayer::GetAmmoInClip),
				overload<const char*>(&CFFPlayer::GetAmmoInClip)
			)

			.addFunction("SetAmmoInClip",
				overload<int>(&CFFPlayer::SetAmmoInClip),
				overload<const char*, int>(&CFFPlayer::SetAmmoInClip)
			)

			.addFunction("GetAmmoCount", &CFFPlayer::LuaGetAmmoCount)

			.addFunction("SendBotMessage",
				overload<const char*>(&CFFPlayer::SendBotMessage),
				overload<const char*, const char*>(&CFFPlayer::SendBotMessage),
				overload<const char*, const char*, const char*>(&CFFPlayer::SendBotMessage),
				overload<const char*, const char*, const char*, const char*>(&CFFPlayer::SendBotMessage)
			)

			.addFunction("GetSentryGun", &CFFPlayer::GetSentryGun)
			.addFunction("GetDispenser", &CFFPlayer::GetDispenser)
			.addFunction("GetDetpack", &CFFPlayer::GetDetpack)
			.addFunction("GetJumpPad", &CFFPlayer::GetManCannon)
			.addFunction("GetEyeAngles", &CFFPlayer::EyeAngles)

			.addFunction("GetJetpackFuelPercent", &CFFPlayer::GetJetpackFuelPercent)
			.addFunction("SetJetpackFuelPercent", &CFFPlayer::SetJetpackFuelPercent)

			.addFunction("GetJetpackState",		&CFFPlayer::GetJetpackState)
			.addFunction("SetJetpackState",		&CFFPlayer::SetJetpackState)
		.endClass()

		.beginNamespace("Player")
			.addProperty("kRandom",			[]() -> int { return 0; })
			.addProperty("kScout",			[]() -> int { return CLASS_SCOUT; })
			.addProperty("kSniper",			[]() -> int { return CLASS_SNIPER; })
			.addProperty("kSoldier",		[]() -> int { return CLASS_SOLDIER; })
			.addProperty("kDemoman",		[]() -> int { return CLASS_DEMOMAN; })
			.addProperty("kMedic",			[]() -> int { return CLASS_MEDIC; })
			.addProperty("kHwguy",			[]() -> int { return CLASS_HWGUY; })
			.addProperty("kPyro",			[]() -> int { return CLASS_PYRO; })
			.addProperty("kSpy",			[]() -> int { return CLASS_SPY; })
			.addProperty("kEngineer",		[]() -> int { return CLASS_ENGINEER; })
			.addProperty("kCivilian",		[]() -> int { return CLASS_CIVILIAN; })
		.endNamespace();
}
