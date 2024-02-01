//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef FF_PLAYER_SHARED_H
#define FF_PLAYER_SHARED_H
#pragma once

#define FF_PUSHAWAY_THINK_INTERVAL		(1.0f / 20.0f)
#include "studio.h"

// avoid macro redefinition
enum
{
	FF_PLAYER_SOUNDS_CITIZEN = 0,
	FF_PLAYER_SOUNDS_COMBINESOLDIER,
	FF_PLAYER_SOUNDS_METROPOLICE,
	FF_PLAYER_SOUNDS_MAX,
};

// avoid macro redefinition
enum FFPlayerState
{
	// Happily running around in the game.
	FF_STATE_ACTIVE = 0,
	FF_STATE_OBSERVER_MODE,		// Noclipping around, watching players, etc.
	FF_NUM_PLAYER_STATES
};


#if defined( CLIENT_DLL )
#define CFFPlayer C_FFPlayer
#endif

#ifndef SDK2013CE
class CPlayerAnimState
{
public:
	enum
	{
		TURN_NONE = 0,
		TURN_LEFT,
		TURN_RIGHT
	};

	CPlayerAnimState(CFFPlayer* outer);

	Activity			BodyYawTranslateActivity(Activity activity);

	void				Update();

	const QAngle& GetRenderAngles();

	void				GetPoseParameters(CStudioHdr* pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM]);

	CFFPlayer* GetOuter();

private:
	void				GetOuterAbsVelocity(Vector& vel);

	int					ConvergeAngles(float goal, float maxrate, float dt, float& current);

	void				EstimateYaw(void);
	void				ComputePoseParam_BodyYaw(void);
	void				ComputePoseParam_BodyPitch(CStudioHdr* pStudioHdr);
	void				ComputePoseParam_BodyLookYaw(void);

	void				ComputePlaybackRate();

	CFFPlayer* m_pOuter;

	float				m_flGaitYaw;
	float				m_flStoredCycle;

	// The following variables are used for tweaking the yaw of the upper body when standing still and
	//  making sure that it smoothly blends in and out once the player starts moving
	// Direction feet were facing when we stopped moving
	float				m_flGoalFeetYaw;
	float				m_flCurrentFeetYaw;

	float				m_flCurrentTorsoYaw;

	// To check if they are rotating in place
	float				m_flLastYaw;
	// Time when we stopped moving
	float				m_flLastTurnTime;

	// One of the above enums
	int					m_nTurningInPlace;

	QAngle				m_angRender;

	float				m_flTurnCorrectionTime;
};
#endif // !SDK2013CE

#endif //FF_PLAYER_SHARED_h
