
// ff_lualib_math.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"


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

/// tostring implemenation for Vector
std::string VectorToString(const Vector& vec)
{
	std::stringstream stream;
	stream << "Vector(" << vec.x << "," << vec.y << "," << vec.z << ")";
	return stream.str();
}
/// tostring implemenation for QAngle
std::string QAngleToString(const QAngle& angle)
{
	std::stringstream stream;
	stream << "QAngle(" << angle.x << "," << angle.y << "," << angle.z << ")";
	return stream.str();
}


namespace FFLib
{
	void AngleVectors(::lua_State* L, const QAngle &angle)
	{
		Vector forward, right, up;
		::AngleVectors(angle, &forward, &right, &up);
		LuaRef(L, forward).push(L);
		LuaRef(L, right).push(L);
		LuaRef(L, up).push(L);
	}

	void VectorAngles(::lua_State* L, const Vector &forward)
	{
		QAngle angles;
		::VectorAngles(forward, angles);
		LuaRef(L, angles).push(L);
	}
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitMath(lua_State* L)
{
	ASSERT(L);

	getGlobalNamespace(L)
		.addFunction("AngleVectors", &FFLib::AngleVectors/*, raw(_1)*/)
		.addFunction("VectorAngles", &FFLib::VectorAngles/*, raw(_1)*/)

		.beginClass<Vector>("Vector")
			.addConstructor<void(), void(float, float, float)>()
			.addFunction("__tostring", &VectorToString)							// def(tostring(const_self))

			.addFunction("__mul",
				overload<float>(&Vector::operator*),							// def(self * float())
				overload<const Vector&>(&Vector::operator*)						// def(self * const_self)
			)

			.addFunction("__div",
				overload<float>(&Vector::operator/),							// def(self / float())
				overload<const Vector&>(&Vector::operator/)						// def(self / const_self)
			)

			.addFunction("__add", overload<const Vector&>(&Vector::operator+))  // def(self + const_self)
			.addFunction("__sub", overload<const Vector&>(&Vector::operator-))  // def(self - const_self)
			.addFunction("__eq", overload<const Vector&>(&Vector::operator==))  // def(const_self == const_self)

			.addProperty("x",					&Vector::x)
			.addProperty("y",					&Vector::y)
			.addProperty("z",					&Vector::z)
			.addFunction("IsValid",				&Vector::IsValid)
			.addFunction("IsZero",				&Vector::IsZero)
			.addFunction("DistTo",				&Vector::DistTo)
			.addFunction("DistToSq",			&Vector::DistToSqr)
			.addFunction("Dot",					&Vector::Dot)
			.addFunction("Cross",				&Vector::Cross)
			.addFunction("Length",				&Vector::Length)
			.addFunction("LengthSqr",			&Vector::LengthSqr)
			.addFunction("Normalize",			&Vector::NormalizeInPlace)
			.addFunction("ClampToAABB",			&Vector::ClampToAABB)
			.addFunction("Random",				&Vector::Random)
			.addFunction("Min",					&Vector::Min)
			.addFunction("Max",					&Vector::Max)
			.addFunction("Negate",				&Vector::Negate)
		.endClass()

		.beginClass<QAngle>("QAngle")
			.addConstructor<void(), void(float, float, float)>()
			.addFunction("__tostring", &QAngleToString)							// def(tostring(const_self))
			.addFunction("__mul", overload<float>(&QAngle::operator*))			// def(self * float())
			.addFunction("__div", overload<float>(&QAngle::operator/))			// def(self / float())
			.addFunction("__add", overload<const QAngle&>(&QAngle::operator+))  // def(self + const_self)
			.addFunction("__sub", overload<const QAngle&>(&QAngle::operator-))  // def(self - const_self)
			.addFunction("__eq", overload<const QAngle&>(&QAngle::operator==))  // def(const_self == const_self)
			.addProperty("x",					&QAngle::x)
			.addProperty("y",					&QAngle::y)
			.addProperty("z",					&QAngle::z)
			.addFunction("IsValid",				&QAngle::IsValid)
			.addFunction("Random",				&QAngle::Random)
			.addFunction("Length",				&QAngle::Length)
			.addFunction("LengthSqr",			&QAngle::LengthSqr)
		.endClass();
};
