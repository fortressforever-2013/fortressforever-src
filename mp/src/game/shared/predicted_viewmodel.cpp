//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "predicted_viewmodel.h"

#ifdef CLIENT_DLL
#include "prediction.h"
#include "iinput.h"
#include "in_buttons.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( predicted_viewmodel, CPredictedViewModel );

IMPLEMENT_NETWORKCLASS_ALIASED( PredictedViewModel, DT_PredictedViewModel )

BEGIN_NETWORK_TABLE( CPredictedViewModel, DT_PredictedViewModel )
//#ifdef GAME_DLL
//	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
//#else
//	RecvPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
//	RecvPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
//#endif
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
CPredictedViewModel::CPredictedViewModel() : m_LagAnglesHistory("CPredictedViewModel::m_LagAnglesHistory")
{
	m_vLagAngles.Init();
	m_LagAnglesHistory.Setup( &m_vLagAngles, 0 );
	m_flWalkBobScale = 0.0f;
	m_flWalkBobPhase = 0.0f;
	m_flAirTime = 0.0f;
	m_flAirBobScale = 0.0f;
	m_flAirBobPhase = 0.0f;
	m_flAirRiseScale = 0.0f;
	m_bWasAirborneRise = false;
	m_bWasOnGround = true;
	m_flLandBobStartTime = -1.0f;
	m_flMedkitSteerAngle = 0.0f;
	m_bMedkitNeedForSpeed = false;
}
#else
CPredictedViewModel::CPredictedViewModel()
{
}
#endif


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPredictedViewModel::~CPredictedViewModel()
{
}

#if defined( HL2_DLL ) || defined( HL2_CLIENT_DLL )
ConVar sv_wpn_sway_pred_legacy( "sv_wpn_sway_pred_legacy", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
#else
ConVar sv_wpn_sway_pred_legacy( "sv_wpn_sway_pred_legacy", "1", FCVAR_REPLICATED | FCVAR_CHEAT );
#endif

#ifdef CLIENT_DLL
ConVar cl_wpn_sway_interp( "cl_wpn_sway_interp", "0.1", FCVAR_CLIENTDLL );
	#ifndef FF_CLIENT_DLL
	ConVar cl_wpn_sway_scale( "cl_wpn_sway_scale", "1.0", FCVAR_CLIENTDLL|FCVAR_CHEAT );
	#else
	ConVar cl_wpn_sway_scale("cl_wpn_sway_scale", "1.0", FCVAR_CLIENTDLL );
	#endif
	
	ConVar cl_bob("cl_bob", "1", FCVAR_CLIENTDLL);
	ConVar cl_wpn_sway_follow("cl_wpn_sway_follow", "1", FCVAR_CLIENTDLL, "Viewmodel sways toward the direction the camera is turning");
	#endif
	
void CPredictedViewModel::CalcViewModelLag( Vector& origin, QAngle& angles, QAngle& original_angles )
{
	#ifdef CLIENT_DLL
		if (prediction->InPrediction())
			return;
		// Calculate our drift
		Vector	forward, right, up;
		AngleVectors( angles, &forward, &right, &up );
	
		// Add an entry to the history.
		m_vLagAngles = angles;
		m_LagAnglesHistory.NoteChanged( gpGlobals->curtime, cl_wpn_sway_interp.GetFloat(), false );
	
		// Interpolate back 100ms.
		m_LagAnglesHistory.Interpolate( gpGlobals->curtime, cl_wpn_sway_interp.GetFloat() );
	
		// Now take the 100ms angle difference and figure out how far the forward vector moved in local space.
		Vector vLaggedForward;
		QAngle angleDiff = m_vLagAngles - angles;
		AngleVectors( -angleDiff, &vLaggedForward, 0, 0 );
		Vector vForwardDiff = Vector(1,0,0) - vLaggedForward;

		// Now offset the origin using that.
		vForwardDiff *= cl_wpn_sway_scale.GetFloat();
		if (cl_wpn_sway_follow.GetBool())
		origin -= forward*vForwardDiff.x + right*-vForwardDiff.y + up*vForwardDiff.z;
		else
		origin += forward*vForwardDiff.x + right*-vForwardDiff.y + up*vForwardDiff.z;

	// Viewmodel bobs
	WalkBob(origin, angles);
	AirBob(origin, angles);
	LandBob(origin, angles);
	MedkitSteerBob(angles);
#endif
}

#ifdef CLIENT_DLL
void CPredictedViewModel::WalkBob(Vector& origin, const QAngle& angles)
{
	if (!cl_bob.GetBool())
	return;

	C_BasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	return;

	const float REFSPEED_FREQ = 10.0f;
	const float REFSPEED_AMP = 7.0f;
	const float REFSPEED = 400.0f; // reference speed, classes that are slower will have the bob that depends on their velocity
	const float BOB_FADE = 4.0f;

	float flSpeed = pPlayer->GetAbsVelocity().Length2D();
	bool bMoving = (flSpeed > 5.0f) && (pPlayer->GetFlags() & FL_ONGROUND);
	float flTargetScale = bMoving ? 1.0f : 0.0f;
	m_flWalkBobScale = Approach(flTargetScale, m_flWalkBobScale, BOB_FADE * gpGlobals->frametime);

	if (m_flWalkBobScale <= 0.001f)
	return;

	float flSpeedScale = clamp(flSpeed / REFSPEED, 0.0f, 1.375f);
	float flFreq = REFSPEED_FREQ * flSpeedScale;
	float flAmp = 0.06f * REFSPEED_AMP * flSpeedScale * m_flWalkBobScale;

	// Accumulate freq phase and only after that include it
	m_flWalkBobPhase += flFreq * gpGlobals->frametime;
	if (m_flWalkBobPhase > 2.0f * M_PI)
		m_flWalkBobPhase -= 2.0f * M_PI;

	float x = cos(m_flWalkBobPhase) *flAmp;
	float y = fabs(sin(m_flWalkBobPhase)) *flAmp;

	Vector vecRight, vecUp;
	AngleVectors(angles, NULL, &vecRight, &vecUp);
	origin += vecRight * x;
	origin += vecUp * y;
}

void CPredictedViewModel::AirBob(Vector& origin, const QAngle& angles)
{
	if (!cl_bob.GetBool())
	return;

	C_BasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	return;

	// Extreme time in air bob to imitate air resistance ...or panic?
	const float START_DELAY = 1.2f;
	const float REFTIME = 2.2f;
	const float REFTIME_FREQ = 70.0f;
	const float REFTIME_AMP = 2.0f;
	const float BOB_FADE = 4.0f;

	// Normal in air bob
	float RISE_DIST = 1.0f;
	const float RISE_UP_TIME = 0.6f;

	// Individual values for some weapons for them to feel good
	CBaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon && Q_stristr(pWeapon->GetClassname(), "ff_weapon_rpg"))
		RISE_DIST = 2.0f;
	if (pWeapon && Q_stristr(pWeapon->GetClassname(), "ff_weapon_grenadelauncher"))
		RISE_DIST = 0.5f;
	if (pWeapon && Q_stristr(pWeapon->GetClassname(), "ff_weapon_pipelauncher"))
		RISE_DIST = 0.5f;
	if (pWeapon && Q_stristr(pWeapon->GetClassname(), "ff_weapon_assaultcannon"))
		RISE_DIST = 0.5f;
	if (pWeapon && Q_stristr(pWeapon->GetClassname(), "ff_weapon_ic"))
		RISE_DIST = 0.5f;

	bool bInWater = (pPlayer->GetWaterLevel() > WL_NotInWater);
	bool bOnLadder = (pPlayer->GetMoveType() == MOVETYPE_LADDER);
	bool bAirborne = !(pPlayer->GetFlags() & FL_ONGROUND) && !bInWater && !bOnLadder;

	Vector vecRight, vecUp;
	AngleVectors(angles, NULL, &vecRight, &vecUp);

	if (!bAirborne && m_bWasAirborneRise)
	m_flAirRiseScale = 0.0f;
	m_bWasAirborneRise = bAirborne;
	float flRiseTarget = bAirborne ? 1.0f : 0.0f;
	m_flAirRiseScale = Approach(flRiseTarget, m_flAirRiseScale, (1.0f / RISE_UP_TIME) * gpGlobals->frametime);
	origin += vecUp * (RISE_DIST * m_flAirRiseScale);

	if (bAirborne)
	m_flAirTime += gpGlobals->frametime;
	else
	m_flAirTime = 0.0f;

	float flTargetScale = bAirborne ? 1.0f : 0.0f;
	m_flAirBobScale = Approach(flTargetScale, m_flAirBobScale, BOB_FADE * gpGlobals->frametime);
	if (m_flAirBobScale <= 0.001f)
	return;

	float flTimeScale = clamp((m_flAirTime - START_DELAY) / (REFTIME - START_DELAY), 0.0f, 1.0f);
	if (flTimeScale <= 0.0f)
	return;

	float flFreq = REFTIME_FREQ * flTimeScale;
	float flAmp = 0.06f * REFTIME_AMP * flTimeScale * m_flAirBobScale;

	m_flAirBobPhase += flFreq * gpGlobals->frametime;
	if (m_flAirBobPhase > 2.0f * M_PI)
		m_flAirBobPhase -= 2.0f * M_PI;

	float x = cos(m_flAirBobPhase) * flAmp;
	float y = sin(m_flAirBobPhase) * cos(m_flAirBobPhase) * flAmp;
	
	origin += vecRight * x;
	origin += vecUp * y;
}

void CPredictedViewModel::LandBob(Vector& origin, const QAngle& angles)
{
	if (!cl_bob.GetBool())
		return;

	C_BasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	const float DOWN_DIST = 1.0f;
	const float DOWN_TIME = 0.05f;
	const float UP_TIME = 0.2f;

	bool bOnGroundNow = (pPlayer->GetFlags() & FL_ONGROUND) != 0;

	if (bOnGroundNow && !m_bWasOnGround)
	m_flLandBobStartTime = gpGlobals->curtime;
	m_bWasOnGround = bOnGroundNow;
	if (m_flLandBobStartTime < 0.0f)
	return;

	float flElapsed = gpGlobals->curtime - m_flLandBobStartTime;
	float flOffset = 0.0f;
	if (flElapsed <= DOWN_TIME)
	{
		flOffset = -DOWN_DIST * (flElapsed / DOWN_TIME);
	}
	else if (flElapsed <= DOWN_TIME + UP_TIME)
	{
		float t = (flElapsed - DOWN_TIME) / UP_TIME;
		flOffset = -DOWN_DIST * (1.0f - t);
	}
	else
	{
		m_flLandBobStartTime = -1.0f;
		return;
	}

	Vector vecUp;
	AngleVectors(angles, NULL, NULL, &vecUp);
	origin += vecUp * flOffset;
}

void CPredictedViewModel::MedkitSteerBob(QAngle& angles) // hlieb: bc its funny
{
	if (!cl_bob.GetBool())
	return;

	C_BasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	return;

	CBaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
	bool bIsMedkit = pWeapon && Q_stristr(pWeapon->GetClassname(), "ff_weapon_medkit");
	if (!bIsMedkit)
	{
		m_flMedkitSteerAngle = 0.0f;
		m_bMedkitNeedForSpeed = false;
		return;
	}

	const float MAX_ANGLE = 11.25f;
	const float TURN_TIME = 0.2f;
	const float RETURN_TIME = 0.2f;
	const float MIN_SPEED_ON = 440.0f;
	const float MIN_SPEED_OFF = 420.0f;

	float flSpeed = pPlayer->GetAbsVelocity().Length2D();
	if (!m_bMedkitNeedForSpeed && flSpeed >= MIN_SPEED_ON)
	m_bMedkitNeedForSpeed = true;
	else if (m_bMedkitNeedForSpeed && flSpeed < MIN_SPEED_OFF)
	m_bMedkitNeedForSpeed = false;

	int nButtons = input->GetButtonBits(0);
	bool bLeft = m_bMedkitNeedForSpeed && (nButtons & IN_MOVELEFT) != 0;
	bool bRight = m_bMedkitNeedForSpeed && (nButtons & IN_MOVERIGHT) != 0;

	float flTargetAngle = 0.0f;
	if (bLeft && !bRight)
	flTargetAngle = -MAX_ANGLE;
	else if (bRight && !bLeft)
	flTargetAngle = MAX_ANGLE;
	float flRampTime = (flTargetAngle != 0.0f) ? TURN_TIME : RETURN_TIME;
	m_flMedkitSteerAngle = Approach(flTargetAngle, m_flMedkitSteerAngle, (MAX_ANGLE / flRampTime) * gpGlobals->frametime);
	angles[ROLL] += m_flMedkitSteerAngle;
}
#endif
