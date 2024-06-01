
// ff_scheduleman.h

/////////////////////////////////////////////////////////////////////////////
// includes
#ifndef UTLMAP_H
#include "utlmap.h"
#endif
#ifndef CHECKSUM_CRC_H
#include "checksum_crc.h"
#endif

extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "LuaBridge/LuaBridge.h"
#include "ff_scriptman.h"

/////////////////////////////////////////////////////////////////////////////
class CFFScheduleCallback
{
public:
	// 'structors
	CFFScheduleCallback(const luabridge::LuaRef& fn,
		float timer);

	CFFScheduleCallback(const luabridge::LuaRef& fn,
		float timer,
		int nRepeat);

	CFFScheduleCallback(const luabridge::LuaRef& fn,
		float timer,
		int nRepeat,
		const luabridge::LuaRef& param);

	CFFScheduleCallback(const luabridge::LuaRef& fn,
		float timer,
		int nRepeat,
		const luabridge::LuaRef& param1,
		const luabridge::LuaRef& param2);

	CFFScheduleCallback(const luabridge::LuaRef& fn,
		float timer,
		int nRepeat,
		const luabridge::LuaRef& param1,
		const luabridge::LuaRef& param2,
		const luabridge::LuaRef& param3);

	CFFScheduleCallback(const luabridge::LuaRef& fn,
		float timer,
		int nRepeat,
		const luabridge::LuaRef& param1,
		const luabridge::LuaRef& param2,
		const luabridge::LuaRef& param3,
		const luabridge::LuaRef& param4);

	CFFScheduleCallback(const CFFScheduleCallback& rhs);

	~CFFScheduleCallback() {}

public:
	// updates. call only once per frame. returns true if the schedule is
	// complete and should be deleted; otherwise returns false
	bool Update();

	// returns true if this schedule is completely done and should be deleted
	bool IsComplete();

private:
	// private data
	luabridge::LuaRef m_function;	// handle to the lua function to call
	float	m_timeLeft;					// time until the lua function should be called
	float	m_timeTotal;				// total time for a complete cycle
	int		m_nRepeat;					// number of times to cycle (-1 is infinite)
	int		m_nParams;					// number of params to pass to the function
	luabridge::LuaRef m_params[4] = {	luabridge::LuaRef(_scriptman.GetLuaState(), luabridge::LuaNil()),
										luabridge::LuaRef(_scriptman.GetLuaState(), luabridge::LuaNil()),
										luabridge::LuaRef(_scriptman.GetLuaState(), luabridge::LuaNil()),
										luabridge::LuaRef(_scriptman.GetLuaState(), luabridge::LuaNil())
	};	// params to pass to function
};

/////////////////////////////////////////////////////////////////////////////
class CFFScheduleManager
{
public:
	// 'structors
	CFFScheduleManager();
	~CFFScheduleManager();

public:
	void Init();
	void Shutdown();
	void Update();

public:
	// adds a schedule
	void AddSchedule(const char* szScheduleName,
		float timer,
		const luabridge::LuaRef& fn);

	void AddSchedule(const char* szScheduleName,
		float timer,
		const luabridge::LuaRef& fn,
		int nRepeat);

	void AddSchedule(const char* szScheduleName,
		float timer,
		const luabridge::LuaRef& fn,
		int nRepeat,
		const luabridge::LuaRef& param);

	void AddSchedule(const char* szScheduleName,
		float timer,
		const luabridge::LuaRef& fn,
		int nRepeat,
		const luabridge::LuaRef& param1,
		const luabridge::LuaRef& param2);

	void AddSchedule(const char* szScheduleName,
		float timer,
		const luabridge::LuaRef& fn,
		int nRepeat,
		const luabridge::LuaRef& param1,
		const luabridge::LuaRef& param2,
		const luabridge::LuaRef& param3);

	void AddSchedule(const char* szScheduleName,
		float timer,
		const luabridge::LuaRef& fn,
		int nRepeat,
		const luabridge::LuaRef& param1,
		const luabridge::LuaRef& param2,
		const luabridge::LuaRef& param3,
		const luabridge::LuaRef& param4);

	// removes a schedule
	void RemoveSchedule(const char* szScheduleName);

private:
	// list of schedules. key is the checksum of an identifying name; it
	// isnt necessarily the name of the lua function to call
	CUtlMap<CRC32_t, CFFScheduleCallback*>	m_schedules;
};

/////////////////////////////////////////////////////////////////////////////
extern CFFScheduleManager _scheduleman;

/////////////////////////////////////////////////////////////////////////////