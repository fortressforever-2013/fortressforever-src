
// ff_lualib_misc.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"

#include "beam_shared.h"

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

/// tostring implemenation for Color
std::string ColorToString(const Color& color)
{
	std::stringstream stream;
	stream << "(" << color.r() << "," << color.g() << "," << color.b() << "," << color.a() << ")";
	return stream.str();
}
//---------------------------------------------------------------------------
// Stuff for LUA deathnotice control
//---------------------------------------------------------------------------

// helper function to check for keys
bool CheckKeyName(const char* keyName)
{
	// we limit on how many keys lua can access/write
	// so lua cannot access the internal keys
	if (Q_strcmp(keyName, "userid")			&& // (int) player_death, sentrygun_killed, dispenser_killed, mancannon_killed, objective_event
		Q_strcmp(keyName, "attacker")		&& // (int) player_death, sentrygun_killed, dispenser_killed, mancannon_killed
		Q_strcmp(keyName, "weapon")			&& // (string classname) player_death, sentrygun_killed, dispenser_killed, mancannon_killed
		Q_strcmp(keyName, "damagetype")		&& // (int) player_death
		Q_strcmp(keyName, "customkill")		&& // (int) player_death
		Q_strcmp(keyName, "killassister")	&& // (int) player_death
		Q_strcmp(keyName, "killersglevel")	&& // (int) player_death, sentrygun_killed, dispenser_killed, mancannon_killed
		Q_strcmp(keyName, "killedsglevel")	&& // (int) player_death, sentrygun_killed
		Q_strcmp(keyName, "attackerpos")	&& // (string vector) sentrygun_killed
		Q_strcmp(keyName, "eventtext")		   // (string) objective_event
		)
	{
		return false;
	}

	return true;
}

// GETTERS

bool GetEventBool(IGameEvent* pEvent, const char* keyName)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("GetEventBool() Unknown key passed!\n");
		return NULL;
	}

	return pEvent->GetBool(keyName);
}

int GetEventInt(IGameEvent* pEvent, const char* keyName)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("GetEventInt() Unknown key passed!\n");
		return NULL;
	}

	return pEvent->GetInt(keyName);
}

float GetEventFloat(IGameEvent* pEvent, const char* keyName)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("GetEventFloat() Unknown key passed!\n");
		return NULL;
	}

	return pEvent->GetFloat(keyName);
}

const char* GetEventString(IGameEvent* pEvent, const char* keyName)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("GetEventString() Unknown key passed!\n");
		return "";
	}

	return pEvent->GetString(keyName);
}

// SETTERS

void SetEventBool(IGameEvent* pEvent, const char* keyName, bool value)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("SetEventBool() Unknown key passed!\n");
		return;
	}
	pEvent->SetBool(keyName, value);
}

void SetEventInt(IGameEvent* pEvent, const char* keyName, int value)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("SetEventInt() Unknown key passed!\n");
		return;
	}
	pEvent->SetInt(keyName, value);
}

void SetEventFloat(IGameEvent* pEvent, const char* keyName, float value)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("SetEventFloat() Unknown key passed!\n");
		return;
	}
	pEvent->SetFloat(keyName, value);
}

void SetEventString(IGameEvent* pEvent, const char* keyName, const char* value)
{
	if ( !CheckKeyName( keyName ) )
	{
		Warning("SetEventString() Unknown key passed!\n");
		return;
	}
	pEvent->SetString(keyName, value);
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitMisc(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)

		// CBeam
		.deriveClass<CBeam, CBaseEntity>("Beam")
			.addFunction("SetColor", &CBeam::SetColor)
		.endClass()
			
		.beginClass<Color>("CustomColor")
			.addConstructor<void (),
							void (int, int, int),
							void (int, int, int, int)
							>()
			.addFunction("__tostring", &ColorToString)
			.addProperty("r",			&Color::r, &Color::setR)
			.addProperty("g",			&Color::g, &Color::setG)
			.addProperty("b",			&Color::b, &Color::setB)
			.addProperty("a",			&Color::a, &Color::setA)
			.addFunction("SetColor",	&Color::SetColor)
		.endClass()

		// IGameEvent class, but we'll only use it for death notices
		.beginClass<IGameEvent>("DeathNotice")
			.addFunction("GetName",		&IGameEvent::GetName)
			.addFunction("IsEmpty",		&IGameEvent::IsEmpty)

			.addFunction("GetBool",		&GetEventBool)
			.addFunction("GetInt",		&GetEventInt)
			.addFunction("GetFloat",	&GetEventFloat)
			.addFunction("GetString",	&GetEventString)

			.addFunction("SetBool",		&SetEventBool)
			.addFunction("SetInt",		&SetEventInt)
			.addFunction("SetFloat",	&SetEventFloat)
			.addFunction("SetString",	&SetEventString)
		.endClass();
};
