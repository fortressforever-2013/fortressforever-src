#ifndef FF_SCRIPTMAN_H
#define FF_SCRIPTMAN_H

// forward declarations
struct lua_State;

namespace luabridge {
	class LuaRef;
};

class CFFLuaSC;

class CFFScriptManager
{
public:
	CFFScriptManager();
	~CFFScriptManager();

private:
	// initializes the script VM
	bool Init();
	void Shutdown();

	void SetupEnvironmentForFF();

public:
	bool LoadFileIntoFunction( const char *filename );
	bool LoadFile( const char *filename );

	void LevelInit(const char* szMapName);

	// Adds a hud element to the list
	int GetOrAddHudElementIndex(const char* szElementName);

	// cleans up the scripts for the most recent level
	void LevelShutdown();

	void LuaMsg( const tchar* pMsg, ... );
	void LuaWarning( const tchar* pMsg, ... );

	// sets a global variable in the script environment
	static void SetVar( lua_State *L, const char *name, const char *value );
	static void SetVar( lua_State *L, const char *name, int value );
	static void SetVar( lua_State *L, const char *name, float value );
	void SetVar(const char* name, const char* value);
	void SetVar(const char* name, int value);
	void SetVar(const char* name, float value);

	// gets a global variable from the script environment
	const char* GetString(const char* name);
	int GetInt(const char* name);
	float GetFloat(const char* name);

	bool GetObject(CBaseEntity* pEntity, luabridge::LuaRef& outObject);
	bool GetObject(const char* szTableName, luabridge::LuaRef& outObject);

	bool GetFunction(CBaseEntity* pEntity, const char* szFunctionName, luabridge::LuaRef& outObject);
	bool GetFunction(luabridge::LuaRef& tableObject, const char* szFunctionName, luabridge::LuaRef& outObject);

	bool RunPredicates_LUA( CBaseEntity *pObject, CFFLuaSC *pContext, const char *szFunctionName );

public:
	// returns the lua interpreter
	lua_State* GetLuaState() const { return L; }

private:
	lua_State*	L;				///< Lua VM
};

// global externs
extern CFFScriptManager _scriptman;

#endif
