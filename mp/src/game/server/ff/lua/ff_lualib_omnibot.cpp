
// ff_lualib_omnibot.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"

#include "omnibot_interface.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "LuaBridge/LuaBridge.h"

#include "omnibot_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------

using namespace luabridge;

//---------------------------------------------------------------------------
void CFFLuaLib::InitOmnibot(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)
		.addFunction("SendBotTrigger", &Omnibot::BotSendTriggerEx)
		.addFunction("SendBotSignal", &Omnibot::SendBotSignal)

		.beginNamespace("Bot")
			.addProperty("kNone",				[]() -> int { return Omnibot::kNone; })
			.addProperty("kBackPack_Ammo",		[]() -> int { return Omnibot::kBackPack_Ammo; })
			.addProperty("kBackPack_Armor",		[]() -> int { return Omnibot::kBackPack_Armor; })
			.addProperty("kBackPack_Health",	[]() -> int { return Omnibot::kBackPack_Health; })
			.addProperty("kBackPack_Grenades",	[]() -> int { return Omnibot::kBackPack_Grenades; })
			.addProperty("kFlag",				[]() -> int { return Omnibot::kFlag; })
			.addProperty("kFlagCap",			[]() -> int { return Omnibot::kFlagCap; })
			.addProperty("kTrainerSpawn",		[]() -> int { return Omnibot::kTrainerSpawn; })
			.addProperty("kHuntedEscape",		[]() -> int { return Omnibot::kHuntedEscape; })
		.endNamespace();
};
