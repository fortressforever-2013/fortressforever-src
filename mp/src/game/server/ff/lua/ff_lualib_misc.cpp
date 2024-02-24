
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
		.endClass();
};
