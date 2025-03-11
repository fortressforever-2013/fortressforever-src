//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#if !defined(_STATIC_LINKED) || defined(_SHARED_LIB)

#include "stdlib.h"
#include "vstdlib/random.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern "C" 
{
	// hide visibility on GCC or shit hits the fan ! -azzy

	DLL_LOCAL
		void __cdecl srand(unsigned int)
	{
	}

	DLL_LOCAL
		int __cdecl rand()
	{
		return RandomInt(0, VALVE_RAND_MAX);
	}

} // extern "C"

#endif // !_STATIC_LINKED || _SHARED_LIB
