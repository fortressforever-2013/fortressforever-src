//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ff_projectile_pipebomb.h"
#include "ff_weapon_base.h"
#include "ammodef.h"
#include "ai_debug_shared.h"
#include "shot_manipulator.h"
#include "ff_shareddefs.h"
#include "ff_utils.h"
#include "ff_buildableobject.h"
#include "ff_buildable_sentrygun.h"
#include "ff_buildable_detpack.h"
#include "ff_buildable_mancannon.h"
#include "ff_buildable_dispenser.h"
#include "ff_weapon_sniperrifle.h"
#include "ff_weapon_assaultcannon.h"

#ifdef CLIENT_DLL
	#include "c_ff_player.h"
	#include <prediction.h>
	#define CRecipientFilter C_RecipientFilter	// |-- For PlayJumpSound

	extern void HudContextShow(bool visible);

#include "ff_hud_grenade1timer.h"
#include "ff_hud_grenade2timer.h"

#else

	#include "ff_player.h"
	#include "iservervehicle.h"
	#include "decals.h"
	#include "ilagcompensationmanager.h"
	#include "EntityFlame.h"

	#include "ff_info_script.h"
	#include "ff_entity_system.h"	// Entity system
	#include "ff_scriptman.h"
	#include "ff_luacontext.h"

	#include "omnibot_interface.h"
#endif

#include "gamevars_shared.h"
#include "takedamageinfo.h"
#include "effect_dispatch_data.h"
#include "engine/ivdebugoverlay.h"

// SDK 2013: just use r_shadows_gamecontrol directly...

// Wrapper CVAR for letting sv_shadows alter r_shadows_gamecontrol
//void SV_Shadows_Callback(IConVar *var, char const *pOldValue, float oldValue)
//{
//	ConVar *c = cvar->FindVar("r_shadows_gamecontrol");
//	ConVarRef sv_shadows(var->GetName());
//	if (c)
//		c->SetValue(sv_shadows.GetString());
//}
//ConVar sv_shadows("sv_shadows", "-1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Toggle shadows on and off | 0 disables | any other number enables", SV_Shadows_Callback );

ConVar sv_voice_inputfromfile("sv_voice_inputfromfile", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Toggle voice_inputfromfile");

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Shows client(red) and server(blue) bullet impact point");
ConVar sv_specchat("sv_spectatorchat", "0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Allows spectators to talk to players");

ConVar sv_motd_enable( "sv_motd_enable", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Enable the MOTD window being shown when a client connects to the server" );

//ConVar ffdev_snipertracesize("ffdev_snipertracesize", "0.25", FCVAR_FF_FFDEV_REPLICATED);
//ConVar ffdev_mancannon_commandtime( "ffdev_mancannon_commandtime", "0.3", FCVAR_FF_FFDEV_REPLICATED );
//ConVar ffdev_sniper_headshotmod( "ffdev_sniper_headshotmod", "2.0", FCVAR_FF_FFDEV_REPLICATED );
#define HEADSHOT_MOD 2.0f //ffdev_sniper_headshotmod.GetFloat()
//ConVar ffdev_sniper_legshotmod( "ffdev_sniper_legshotmod", "1.0", FCVAR_FF_FFDEV_REPLICATED );
#define LEGSHOT_MOD 1.0f //ffdev_sniper_legshotmod.GetFloat()
//ConVar ffdev_sniper_legshot_time( "ffdev_sniper_legshot_time", "5.0", FCVAR_FF_FFDEV_REPLICATED );
#define LEGSHOT_TIME 5.0f //ffdev_sniper_legshot_time.GetFloat()
//AfterShock: radiotag time is in ff_player.cpp under RADIOTAG_DURATION

//ConVar ffdev_infect_numticks("ffdev_infect_numticks","10",FCVAR_FF_FFDEV_REPLICATED,"Number of infection ticks before it wears off");
#define INFECT_NUMTICKS 10

//ConVar ffdev_overpressure_selfpush_horizontal( "ffdev_overpressure_selfpush_horizontal", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SELFPUSH_HORIZONTAL 1
//ConVar ffdev_overpressure_selfpush_vertical( "ffdev_overpressure_selfpush_vertical", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SELFPUSH_VERTICAL 1
//ConVar ffdev_overpressure_push_horizontal( "ffdev_overpressure_push_horizontal", "350", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_PUSH_HORIZONTAL 350.0f
//ConVar ffdev_overpressure_push_vertical( "ffdev_overpressure_push_vertical", "350", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_PUSH_VERTICAL 350.0f
// for release code: need to update ff_hud_overpressure.cpp #define 
//ConVar ffdev_overpressure_delay( "ffdev_overpressure_delay", "16", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_DELAY 16
//ConVar ffdev_overpressure_radius( "ffdev_overpressure_radius", "128", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_RADIUS 128
//ConVar ffdev_overpressure_groundpush_multiplier( "ffdev_overpressure_groundpush_multiplier", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_GROUNDPUSH_MULTIPLIER 1.0f
//ConVar ffdev_overpressure_speed_percent( "ffdev_overpressure_speed_percent", "1.5", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SPEED_PERCENT 1.5f
//ConVar ffdev_overpressure_speed_multiplier_horizontal( "ffdev_overpressure_speed_multiplier_horizontal", ".5", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SPEED_MULTIPLIER_HORIZONTAL 0.5f
//ConVar ffdev_overpressure_speed_multiplier_vertical( "ffdev_overpressure_speed_multiplier_vertical", ".5", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SPEED_MULTIPLIER_VERTICAL 0.5f

//ConVar ffdev_overpressure_slide( "ffdev_overpressure_slide", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE true
//ConVar ffdev_overpressure_slide_affectsself( "ffdev_overpressure_slide_affectsself", "0", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_AFFECTSSELF false
//ConVar ffdev_overpressure_slide_duration( "ffdev_overpressure_slide_duration", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_DURATION 1.0f
//ConVar ffdev_overpressure_slide_friction( "ffdev_overpressure_slide_friction", "0", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_FRICTION 0.0f
//ConVar ffdev_overpressure_slide_airaccel( "ffdev_overpressure_slide_airaccel", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_AIRACCEL 1.0f
//ConVar ffdev_overpressure_slide_accel( "ffdev_overpressure_slide_accel", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_ACCEL 1.0f
//ConVar ffdev_overpressure_slide_wearsoff( "ffdev_overpressure_slide_wearsoff", "1", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_WEARSOFF 1.0f
//ConVar ffdev_overpressure_slide_wearsoff_bias( "ffdev_overpressure_slide_wearsoff_bias", "0.2", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_SLIDE_WEARSOFF_BIAS 0.2f

//ConVar ffdev_overpressure_friendlyscale( "ffdev_overpressure_friendlyscale", "1.0", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_FRIENDLYSCALE 1.0f //ffdev_overpressure_friendlyscale.GetFloat()
//ConVar ffdev_overpressure_friendlyignore( "ffdev_overpressure_friendlyignore", "0", FCVAR_FF_FFDEV_REPLICATED );
#define OVERPRESSURE_IGNOREFRIENDLY false //ffdev_overpressure_friendlyignore.GetBool()

//ConVar ffdev_jetpack_horizontalpush_cap("ffdev_jetpack_horizontalpush_cap", "1000", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JETPACK_HORIZONTALPUSH_CAP 1000.0f // ffdev_jetpack_horizontalpush_cap.GetFloat()
//ConVar ffdev_jetpack_verticalpush_downwardslimit("ffdev_jetpack_verticalpush_downwardslimit", "1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define FFDEV_JETPACK_VERTICALPUSH_DOWNWARDSLIMIT 1.0f //ffdev_jetpack_verticalpush_downwardslimit.GetFloat()

//ConVar ffdev_jetpack_verticalpush_offground("ffdev_jetpack_verticalpush_offground", "10", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JETPACK_VERTICALPUSH_OFFGROUND 10.0f //ffdev_jetpack_verticalpush_offground.GetFloat()
//ConVar ffdev_jetpack_verticalpush_offground_downscale("ffdev_jetpack_verticalpush_offground_downscale", "0.1", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JETPACK_VERTICALPUSH_OFFGROUND_DOWNSCALE 0.1f //ffdev_jetpack_verticalpush_offground_downscale.GetFloat()
//ConVar ffdev_jetpack_horizontalpush_offground("ffdev_jetpack_horizontalpush_offground", "2", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JETPACK_HORIZONTALPUSH_OFFGROUND 2.0f //ffdev_jetpack_horizontalpush_offground.GetFloat()

//ConVar ffdev_jetpack_fuelrechargetime("ffdev_jetpack_fuelrechargetime", "0.08", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JETPACK_FUELRECHARGETIME 0.08f //ffdev_jetpack_fuelrechargetime.GetFloat()
//ConVar ffdev_jetpack_fuelhovercost("ffdev_jetpack_fuelhovercost", "0.5", FCVAR_REPLICATED | FCVAR_CHEAT);
// changed jetpack fuel to scalar, and increased to 200 to have same values
#define JETPACK_FUELHOVERCOST 1 //ffdev_jetpack_fuelhovercost.GetFloat()

#define JETPACK_MINFUEL 2
#define JETPACK_MAXFUEL 200

//ConVar ffdev_ac_bulletsize( "ffdev_ac_bulletsize", "1.0", FCVAR_FF_FFDEV_REPLICATED );
#define FF_AC_BULLETSIZE 1.0f //ffdev_ac_bulletsize.GetFloat()

//ConVar ffdev_ac_newsystem( "ffdev_ac_newsystem", "0.0", FCVAR_FF_FFDEV_REPLICATED );
#define FF_AC_NEWSYSTEM false //ffdev_ac_newsystem.GetBool()

#define OVERPRESSURE_JERKMULTI 0.0004f

//ConVar ffdev_ac_impactfreq( "ffdev_ac_impactfreq", "2.0", FCVAR_FF_FFDEV_REPLICATED );
#define FF_AC_IMPACTFREQ 2 //ffdev_ac_impactfreq.GetInt()

//ConVar ffdev_sniperrifle_legshot_minslowdownspeed( "ffdev_sniperrifle_legshot_minslowdownspeed", "0.7", FCVAR_FF_FFDEV_REPLICATED, "Player speed when hit with a minimum charge sniper rifle shot (0.7 would mean player speed at 70% after being legshot)" );
//ConVar ffdev_sniperrifle_legshot_chargedivider( "ffdev_sniperrifle_legshot_chargedivider", "3", FCVAR_FF_FFDEV_REPLICATED, "1/number = extra slowdown when hit with max charge legshot. e.g. if '3.0' then 33% extra slowdown @ max charge" );
			
// Time in seconds you have to wait until you can cloak again
//ConVar ffdev_spy_nextcloak( "ffdev_spy_nextcloak", "5", FCVAR_FF_FFDEV_REPLICATED, "Time in seconds you have to wait until you can cloak again" );
#define SPY_NEXTCLOAK 5.0f

//ConVar ffdev_spy_scloak_minstartvelocity( "ffdev_spy_scloak_minstartvelocity", "80", FCVAR_FF_FFDEV_REPLICATED, "Spy must be moving at least this slow to scloak." );
#define SPY_SCLOAK_MINSTARTVELOCITY 80.0f

//ConVar sniperrifle_pushmin( "ffdev_sniperrifle_pushmin", "2.5", FCVAR_FF_FFDEV_REPLICATED );
#define FF_SNIPER_MINPUSH 2.5f // sniperrifle_pushmin.GetFloat()

//ConVar sniperrifle_pushmax( "ffdev_sniperrifle_pushmax", "5.5", FCVAR_FF_FFDEV_REPLICATED );
#define FF_SNIPER_MAXPUSH 5.5f // sniperrifle_pushmax.GetFloat()

#define OVERPRESSURE_EFFECT "FF_OverpressureEffect"

//0001279: Need convar for pipe det delay
#define PIPE_DET_DELAY 0.55 // this is mirrored in ff_projectile_pipebomb.cpp and ff_player.cpp
extern ConVar ai_debug_shoot_positions;

// grenade information
//ConVar gren_timer("ffdev_gren_timer","3.81",0,"Timer length for all grenades.");
#define GREN_TIMER 3.81f
//ConVar gren_throw_delay("ffdev_throw_delay","0.5",0,"Delay before primed grenades can be thrown.");
#define GREN_THROW_DELAY 0.5f

#ifdef CLIENT_DLL
	extern char g_szTimerFile[MAX_PATH];

	void TimerChange_Callback(IConVar* var, const char* pOldValue, float flOldValue);
#endif

void DispatchEffect(const char *pName, const CEffectData &data);

// Used to decide whether effects are allowed
static float g_flNextEffectAllowed[MAX_PLAYERS + 1];

bool AllowEffects(int iEntityIndex, float flNewDelay)
{
	if (iEntityIndex < 1 || iEntityIndex > MAX_PLAYERS)
		return true;

	if (g_flNextEffectAllowed[iEntityIndex - 1] < gpGlobals->curtime)
	{
		g_flNextEffectAllowed[iEntityIndex - 1] = gpGlobals->curtime + flNewDelay;
		return true;
	}
	return false;
}

void ClearAllowedEffects()
{
	memset(g_flNextEffectAllowed, 0, sizeof(g_flNextEffectAllowed));
}

CFFWeaponBase * CFFPlayer::FFAnim_GetActiveWeapon()
{
	return GetActiveFFWeapon();
}

CFFPlayer * CFFPlayer::FFAnim_GetPlayer()
{
	return this;
}

bool CFFPlayer::FFAnim_CanMove()
{
	return true;
}

//ConVar sniperrifle_basedamage( "ffdev_sniperrifle_basedamage", "45", FCVAR_FF_FFDEV_REPLICATED, "Base Damage for Sniper Rifle" );
#define	SR_BASE_DAMAGE 45.0f // sniperrifle_basedamage.GetFloat()

//ConVar sniperrifle_basedamagemax( "ffdev_sniperrifle_basedamagemax", "275", FCVAR_FF_FFDEV_REPLICATED, "Base Max Damage for Sniper Rifle" );
#define	SR_BASE_DAMAGE_MAX 275.0f // sniperrifle_basedamagemax.GetFloat()

void CFFPlayer::FireBullet(
						   Vector vecSrc, 	// shooting postion
						   const QAngle &shootAngles, //shooting angle
						   float vecSpread, // spread vector
						   float flDamage, // base damage		// |-- Mirv: Floating damage
						   int iBulletType, // ammo type
						   CBaseEntity *pevAttacker, // shooter
						   bool bDoEffects, 	// create impact effect ?
						   float x, 	// spread x factor
						   float y, 	// spread y factor
						   float flSniperRifleCharge // added by Mulchman 9/20/2005
						)
{
	// NOTE NOTE NOTE: Only sniper rifle uses this anymore!

	float flCurrentDamage = SR_BASE_DAMAGE/*flDamage*/;   // damage of the bullet at it's current trajectory
	float flScale = 1.0f;			// scale the force
	//float flCurrentDistance = 0.0;  //distance that the bullet has traveled so far
	float flMaxRange = MAX_TRACE_LENGTH;

	bool bHeadshot = false;

	Vector vecDirShooting /*, vecRight, vecUp*/;
	AngleVectors( shootAngles, &vecDirShooting/*, &vecRight, &vecUp*/ );

	if (!pevAttacker)
		pevAttacker = this;  // the default attacker is ourselves

	// add the spray 
	/*
	Vector vecDir = vecDirShooting +
		x * vecSpread * vecRight +
		y * vecSpread * vecUp;
		*/

	Vector vecDir = vecDirShooting;
	VectorNormalize( vecDir );	

	Vector vecEnd = vecSrc + ( vecDir * flMaxRange ); // max bullet range is 10000 units

	trace_t tr; // main enter bullet trace
	UTIL_TraceLine( vecSrc, vecEnd, MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, this, COLLISION_GROUP_NONE, &tr );

	/*
	float flSize = ffdev_snipertracesize.GetFloat();
	Vector vecHull = Vector(1.0f, 1.0f, 1.0f) * flSize;
	QAngle tmpAngle;
	VectorAngles(vecDir, tmpAngle);

	UTIL_TraceHull(vecSrc, vecEnd, -vecHull, vecHull, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	*/

#ifdef GAME_DLL
	//NDebugOverlay::SweptBox(vecSrc, vecEnd, -vecHull, vecHull, tmpAngle, 255, 0, 0, 255, 10.0f);
#endif

	if (tr.fraction == 1.0f)
		return; // we didn't hit anything, stop tracing shoot

	if (sv_showimpacts.GetBool())
	{
#ifdef CLIENT_DLL
		// draw red client impact markers
		debugoverlay->AddBoxOverlay(tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), 255, 0, 0, 127, 4);

		if (tr.m_pEnt && tr.m_pEnt->IsPlayer())
		{
			C_BasePlayer *player = ToBasePlayer(tr.m_pEnt);
			player->DrawClientHitboxes(4, true);
		}
#else
		// draw blue server impact markers
		NDebugOverlay::Box(tr.endpos, Vector(-2, -2, -2), Vector(2, 2, 2), 0, 0, 255, 127, 4);

		if (tr.m_pEnt && tr.m_pEnt->IsPlayer())
		{
			CBasePlayer *player = ToBasePlayer(tr.m_pEnt);
			player->DrawServerHitboxes(4, true);
		}
#endif
	}

	//calculate the damage based on the distance the bullet travelled.
	//flCurrentDistance += tr.fraction * flMaxRange;

	// damage get weaker of distance
	//flCurrentDamage *= pow(0.85f, (flCurrentDistance / 500));	// |-- Mirv: Distance doesnt affect sniper rifle

	// --> Mirv: Locational damage

	// Only if this is a charged shot
	if (flSniperRifleCharge)
	{
		//float flBaseDamage = flCurrentDamage;
		//flCurrentDamage = flBaseDamage + flBaseDamage * (FF_SNIPER_MAXCHARGE - 1) * ( flSniperRifleCharge / FF_SNIPER_MAXCHARGE);

		// what was the old code?  seriously...
		float flChargePercentage = ( flSniperRifleCharge / FF_SNIPER_MAXCHARGE);
		flCurrentDamage = SR_BASE_DAMAGE + ( flChargePercentage * (SR_BASE_DAMAGE_MAX - SR_BASE_DAMAGE) );
		
		/*float fOldDamage = flBaseDamage * flSniperRifleCharge;
		if (fOldDamage < flBaseDamage)
			fOldDamage = flBaseDamage;
		DevMsg("Sniper damage: %.1f. Old damage: %.1f.",fCurrentDamage,fOldDamage);
		*/

		// Bug #0000671: Sniper rifle needs to cause more push upon hitting
		// Nothing fancy... 4.5 seemed to be about TFC's quick shot
		// and 8.5 seemed to be about TFC's full charge shot
		//fScale = clamp( flSniperRifleCharge + 3.5f, 4.5f, 8.5f );
		// NOTE: New phish scale!
		//flScale = FF_SNIPER_MINPUSH + ( ( flSniperRifleCharge * ( FF_SNIPER_MAXPUSH - FF_SNIPER_MINPUSH ) ) / FF_SNIPER_MAXCHARGE );
		// what was the old code?  seriously...
		flScale = FF_SNIPER_MINPUSH + ( flChargePercentage * (FF_SNIPER_MAXPUSH - FF_SNIPER_MINPUSH) );

		if (tr.hitgroup == HITGROUP_HEAD)
		{
			DevMsg("Headshot, damage multiplied by %f\n", HEADSHOT_MOD );
			flCurrentDamage *= HEADSHOT_MOD;

			bHeadshot = true;

#ifdef CLIENT_DLL
			FF_SendHint( SNIPER_HEADSHOT, 3, PRIORITY_NORMAL, "#FF_HINT_SNIPER_HEADSHOT" );
#endif
		}
		else if (tr.hitgroup == HITGROUP_LEFTLEG || tr.hitgroup == HITGROUP_RIGHTLEG)
		{
			DevMsg("Legshot\n");
			flCurrentDamage *= LEGSHOT_MOD;
#ifdef CLIENT_DLL
			FF_SendHint( SNIPER_LEGSHOT, 3, PRIORITY_NORMAL, "#FF_HINT_SNIPER_LEGSHOT" );
#endif

#ifdef GAME_DLL
			// Bug #0000557: Teamplay 0 + sniper legshot slows allies
			// Don't apply the speed effect if the hit player is a teammate/ally

			// Slowed down by 10% - 60% depending on charge
			// Person hit by sniper rifle
			CFFPlayer *player = ToFFPlayer(tr.m_pEnt);

			// Person shooting the sniper rifle
			CFFPlayer *pShooter = ToFFPlayer(pevAttacker);

			// Bug #0000557: Teamplay 0 + sniper legshot slows allies
			// If they're not a teammate/ally then do the leg shot speed effect
			float flDuration =  LEGSHOT_TIME;
			float flIconDuration = flDuration;
			// AfterShock: this should be like 0.7f - 7 / (7 * 2)
			// so like if divider is high, less slowdown,  divider low = more slowdown
			//float flSpeed = ffdev_sniperrifle_legshot_minslowdownspeed.GetFloat() - flSniperRifleCharge / ( FF_SNIPER_MAXCHARGE * ffdev_sniperrifle_legshot_chargedivider.GetFloat() );
			
			// max legshot slowdown is reached in 2 seconds (ish) - since you can fire off 2 quick legshots to achieve same result.
			float flSpeed = 0.7f - flSniperRifleCharge / FF_SNIPER_MAXCHARGE ;
			if( player->LuaRunEffect( LUA_EF_LEGSHOT, pShooter, &flDuration, &flIconDuration, &flSpeed ) )
			{
				if (g_pGameRules->PlayerRelationship(pShooter, player) == GR_NOTTEAMMATE)
				{
					player->AddSpeedEffect( SE_LEGSHOT, flDuration, flSpeed, SEM_ACCUMULATIVE| SEM_HEALABLE, FF_STATUSICON_LEGINJURY, flIconDuration );
				}
			}
#endif
		}			
	}
	// --> Mirv: Locational damage

	int iDamageType = DMG_BULLET | DMG_NEVERGIB;

	if (flSniperRifleCharge && FF_IsAirshot( tr.m_pEnt ))
	{
		iDamageType |= DMG_AIRSHOT;
	}

	if (bDoEffects) // Only once every 0.3 seconds
	{
		// See if the bullet ended up underwater + started out of the water
		if (enginetrace->GetPointContents(tr.endpos) & (CONTENTS_WATER|CONTENTS_SLIME))
		{	
			trace_t waterTrace;
			UTIL_TraceLine(vecSrc, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), this, COLLISION_GROUP_NONE, &waterTrace);

			if (waterTrace.allsolid != 1)
			{
				CEffectData	data;
				data.m_vOrigin = waterTrace.endpos;
				data.m_vNormal = waterTrace.plane.normal;
				data.m_flScale = random->RandomFloat(8, 12);

				if (waterTrace.contents & CONTENTS_SLIME)
				{
					data.m_fFlags |= FX_WATER_IN_SLIME;
				}

				DispatchEffect("gunshotsplash", data);
			}
		}
		else
		{
			//Do Regular hit effects

			// Don't decal nodraw surfaces
			if (! (tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP)))
			{
				CBaseEntity *pEntity = tr.m_pEnt;

				// Revised further for
				// Bug: 0000620: Trace attacks aren't hitting walls

				// Mirv: Do impact traces no matter what
				if (pEntity /*&& pEntity->IsPlayer() */) //! (!friendlyfire.GetBool() && pEntity && pEntity->IsPlayer() && pEntity->GetTeamNumber() == GetTeamNumber()))
				{
					UTIL_ImpactTrace(&tr, iDamageType);
				}
			}
		}
	} // bDoEffects

	// add damage to entity that we hit

	ClearMultiDamage();
	CTakeDamageInfo info( ToFFPlayer(pevAttacker)->GetActiveFFWeapon(), pevAttacker, flCurrentDamage, iDamageType );	// |-- Mirv: Modified this

	// for radio tagging and to make ammo type work in the DamageFunctions
	info.SetAmmoType( iBulletType );

//#ifdef GAME_DLL
//	// Hack for sniper rifle to become radio tag rifle
//	if( flSniperRifleCharge )
//		info.SetAmmoType( m_iRadioTaggedAmmoIndex );
//#endif

	CalculateBulletDamageForce(&info, iBulletType, vecDir, tr.endpos, flScale);
	info.ScaleDamageForce(flScale * flScale * flScale);

	if (tr.m_pEnt->IsPlayer())
	{
		info.ScaleDamageForce(0.01f);
	}

	if (bHeadshot)
	{
		info.SetDamageCustom(DAMAGETYPE_HEADSHOT);
	}

	tr.m_pEnt->DispatchTraceAttack(info, vecDir, &tr);

	// Bug #0000168: Blood sprites for damage on players do not display
#ifdef GAME_DLL
	TraceAttackToTriggers(info, tr.startpos, tr.endpos, vecDir);
	bool bShouldGib = false;
	if (tr.m_pEnt->IsPlayer())
	{
		CFFPlayer *pPlayer = ToFFPlayer(tr.m_pEnt);
		bShouldGib = pPlayer->ShouldGib(info);
	}
#endif

	ApplyMultiDamage();

	// Sniper rifle has some extra hit & gib sounds that we need to use.
#ifdef GAME_DLL
	if (flSniperRifleCharge)
	{
		if (tr.m_pEnt->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer(tr.m_pEnt);

			if (pPlayer)
			{
				CSingleUserRecipientFilter filter( this );

				if (pPlayer->IsAlive())
					EmitSound( filter, entindex(), "Sniper.Hit" );
				else // player just got killed
					// if headshot or gib - gibsound
					// gibbing doesnt trigger it at the moment - this is slightly bugged - AfterShock
					if (bHeadshot)
						EmitSound( filter, entindex(), "Sniper.Gib" );
					else if ( bShouldGib )
						EmitSound( filter, entindex(), "Player.Gib" );
					else // killsound
						EmitSound( filter, entindex(), "Sniper.Hit" );
				/*
				EmitSound_t ep;
				ep.m_nChannel = CHAN_BODY;
				ep.m_pSoundName = pPlayer->IsAlive() ? "Sniper.Hit" : "Sniper.Gib";
				ep.m_flVolume = 1.0f;
				ep.m_SoundLevel = SNDLVL_70dB; // params.soundlevel;
				ep.m_nFlags = 0;
				ep.m_nPitch = PITCH_NORM; // params.pitch;
				ep.m_pOrigin = &GetAbsOrigin();

				EmitSound( filter, entindex(), ep );
				*/
			}
		}
	}
#endif


}

// --> Mirv: Proper sounds
void CFFPlayer::PlayJumpSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol)
{
	// Remember last time idled
	m_flIdleTime = gpGlobals->curtime;

	if (!psurface)
		return;

	if (m_flJumpTime + 0.2f > gpGlobals->curtime)
		return;

	m_flJumpTime = gpGlobals->curtime;

	CRecipientFilter filter;
	filter.AddRecipientsByPAS(vecOrigin);

#ifdef GAME_DLL
	// Don't send to self
	if (gpGlobals->maxClients > 1)
	{
		filter.RemoveRecipient(this);
	}
#endif

	EmitSound_t ep;
	ep.m_nChannel = CHAN_BODY;
#ifdef CLIENT_DLL
	ep.m_pSoundName = "Player.ClientJump"; //params.soundname;
#else
	ep.m_pSoundName = "Player.Jump"; //params.soundname;
#endif
	ep.m_flVolume = fvol;
	ep.m_SoundLevel = SNDLVL_70dB; // params.soundlevel;
	ep.m_nFlags = 0;
	ep.m_nPitch = PITCH_NORM; // params.pitch;
	ep.m_pOrigin = &vecOrigin;

	EmitSound(filter, entindex(), ep);
}

void CFFPlayer::PlayFallSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol)
{
	if (!psurface)
		return;

	// Kill sound if we're a falling spy
	if (GetClassSlot() == 8 && GetFlags() & FL_DUCKING)
	{
		// Play a local sound
		CSingleUserRecipientFilter filter( this );
		EmitSound( filter, entindex(), "Player.SpyFall" );

		return;
	}

	if (m_flFallTime > gpGlobals->curtime)
		return;

	m_flFallTime = gpGlobals->curtime + 0.4f;

#ifdef CLIENT_DLL
	
	if ( GetClassSlot() == 8 )
		FF_SendHint( SPY_SPLAT, 3, PRIORITY_NORMAL, "#FF_HINT_SPY_SPLAT" );
#endif

	EmitSoundShared("Player.FallDamage");
}

void CFFPlayer::PlayStepSound(Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force)
{
	// Remember last time idled
	m_flIdleTime = gpGlobals->curtime;	

	switch( GetClassSlot() )
	{
		// Don't play footsteps for spy
		// Jiggles: 0001374: But do play footsteps while disguised as a non-spy class, unless cloaked
		case CLASS_SPY:
		{
			if( IsCloaked() || !IsDisguised() )
				return;

			if( IsDisguised() && (GetDisguisedClass() == CLASS_SPY) )
				return;

		}
		break;

		// Bug #0001520: Sniper has footsteps while charging sniper rifle
		case CLASS_SNIPER:
		{
			CFFWeaponBase *pWeapon = GetActiveFFWeapon();
			if( pWeapon && (pWeapon->GetWeaponID() == FF_WEAPON_SNIPERRIFLE) )
			{
				CFFWeaponSniperRifle *pSniperRifle = dynamic_cast<CFFWeaponSniperRifle *>( pWeapon );
				if( pSniperRifle && pSniperRifle->IsInFire() )
					return;
			}
		}
		break;
	}

	BaseClass::PlayStepSound( vecOrigin, psurface, fvol, force );
}
// <-- Mirv: Proper sounds

/** Allow Lua to react to and/or deny +use
*/
void CFFPlayer::PlayerUse()
{
#ifdef GAME_DLL
	if (m_afButtonPressed & IN_USE)
	{
		CFFLuaSC hContext( 0 );
		hContext.Push( this );
		if( _scriptman.RunPredicates_LUA( NULL, &hContext, "player_onuse" ) && !hContext.DidReturnNil() && !hContext.GetBool() )
		{
			return;
		}
	}
#endif

	BaseClass::PlayerUse();
}

//-----------------------------------------------------------------------------
// Purpose: Handle all class specific skills
//-----------------------------------------------------------------------------
void CFFPlayer::ClassSpecificSkill()
{
	if (m_flNextClassSpecificSkill > gpGlobals->curtime)
		return;

#ifdef CLIENT_DLL
	CFFWeaponBase *pWeapon = GetActiveFFWeapon();		
#endif

	switch (GetClassSlot())
	{
#ifdef GAME_DLL
	case CLASS_DEMOMAN:
		m_bQueueDetonation = true;
		break;

	case CLASS_MEDIC:
		// Discard a health pack
		if (IsAlive() && GetAmmoCount(AMMO_CELLS) >= 10)
		{
			CBaseEntity *pHealthDrop = CBaseEntity::Create("ff_item_healthdrop", GetAbsOrigin(), GetAbsAngles());

			if (pHealthDrop)
			{
				pHealthDrop->Spawn();
				pHealthDrop->SetOwnerEntity(this);
				QAngle angSpin = QAngle(0,450,0);
				//pHealthDrop->SetLocalAngularVelocity(RandomAngle(-400, 400));
				pHealthDrop->SetLocalAngularVelocity(angSpin);

				Vector vForward;
				AngleVectors(EyeAngles(), &vForward);

				vForward *= 420.0f;

				// Bugfix: Floating objects
				if (vForward.z < 1.0f)
					vForward.z = 1.0f;

				pHealthDrop->SetAbsVelocity(vForward + Vector(0,0,250));
				pHealthDrop->SetAbsOrigin(GetAbsOrigin());

				// Play a sound
				EmitSound("Item.Toss");

				RemoveAmmo(10, AMMO_CELLS);
			}
		}
		break;
#endif
		
		case CLASS_HWGUY:
			/*if( pWeapon && (pWeapon->GetWeaponID() == FF_WEAPON_ASSAULTCANNON) )
			{
				SwapToWeapon(FF_WEAPON_SUPERSHOTGUN);
			}
			else 
			{
				SwapToWeapon(FF_WEAPON_ASSAULTCANNON);
			}*/
			Overpressure();
			m_flNextClassSpecificSkill = gpGlobals->curtime + OVERPRESSURE_DELAY;

			break;

		case CLASS_PYRO:
			break;

#ifdef CLIENT_DLL

		case CLASS_SOLDIER:
			if( pWeapon && (pWeapon->GetWeaponID() == FF_WEAPON_RPG) )
			{
				SwapToWeapon(FF_WEAPON_SUPERSHOTGUN);
			}
			else 
			{
				SwapToWeapon(FF_WEAPON_RPG);
			}
			break;

		case CLASS_ENGINEER:
		case CLASS_SPY:
			// Bug #0001683: Can use engineer radial menu when dead.  This seems to put an end to it -> Defrag
			if( IsAlive()  )
			{
				HudContextShow(true);
			}			
			break;

#endif
		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handle all class specific skills
//-----------------------------------------------------------------------------
void CFFPlayer::ClassSpecificSkillHold()
{
	if (m_flNextClassSpecificSkill > gpGlobals->curtime)
		return;

	switch (GetClassSlot())
	{
		case CLASS_PYRO:
			JetpackHold();
			if (m_iJetpackFuel < JETPACK_MINFUEL)
			{
				m_bJetpacking = false;
				// note: bumped this up a little - 0.25f was too low w/ client side prediction,
				// holding down m2 would spert it between 0/1 or 2 fuel and not play effects
				// it can still happen on really high pings tho (150+)
				m_flNextClassSpecificSkill = gpGlobals->curtime + 0.35f;
			}
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Anything to do after they've stopped pressing
//-----------------------------------------------------------------------------
void CFFPlayer::ClassSpecificSkill_Post()
{
	switch (GetClassSlot())
	{
#ifdef CLIENT_DLL
		case CLASS_ENGINEER:
		case CLASS_SPY:
			HudContextShow(false);
			break;
#endif
		case CLASS_PYRO:
			m_bJetpacking = false;
			break;

		default:
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get our feet position
//-----------------------------------------------------------------------------
Vector CFFPlayer::GetFeetOrigin( void )
{
	// TODO: Get a position for in water (when swimming)

	if( GetFlags() & FL_DUCKING )
		return GetAbsOrigin() - Vector( 0, 0, 18 );
	else
		return GetAbsOrigin() - Vector( 0, 0, 36 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFPlayer::GetHealthPercentage( void ) const
{
	float flPerc = ((float) GetHealth() / (float) GetMaxHealth()) * 100.0f;
	return (int) flPerc;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFPlayer::GetArmorPercentage( void ) const
{
	float flPerc = ((float) GetArmor() / (float) GetMaxArmor()) * 100.0f;
	return (int) flPerc;
}

//-----------------------------------------------------------------------------
// Purpose: Player building? NOTE: This can include building SGs but still able to move around/shoot
//-----------------------------------------------------------------------------
bool CFFPlayer::IsBuilding( void ) const
{
	return m_bBuilding;
}

//-----------------------------------------------------------------------------
// Purpose: Player static building? E.g. building detpacks / jump pads (cant move, cant shoot!)
//-----------------------------------------------------------------------------
bool CFFPlayer::IsStaticBuilding( void ) const
{
	return m_bStaticBuilding;
}

//-----------------------------------------------------------------------------
// Purpose: What's the player currently buildng
//-----------------------------------------------------------------------------
int CFFPlayer::GetCurrentBuild( void ) const
{
	return m_iCurBuild;
}

//-----------------------------------------------------------------------------
// Purpose: Get detpack
//-----------------------------------------------------------------------------
CFFDetpack *CFFPlayer::GetDetpack( void ) const
{
	return static_cast< CFFDetpack * >( m_hDetpack.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: Get dispenser
//-----------------------------------------------------------------------------
CFFDispenser *CFFPlayer::GetDispenser( void ) const
{
	return static_cast< CFFDispenser * >( m_hDispenser.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: Get sentrygun
//-----------------------------------------------------------------------------
CFFSentryGun *CFFPlayer::GetSentryGun( void ) const
{
	return static_cast< CFFSentryGun * >( m_hSentryGun.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: Get man cannon
//-----------------------------------------------------------------------------
CFFManCannon *CFFPlayer::GetManCannon( void ) const
{
	return static_cast<CFFManCannon *>( m_hManCannon.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFBuildableObject *CFFPlayer::GetBuildable( int iBuildable ) const
{
	CFFBuildableObject *pEntity = NULL;

	switch( iBuildable )
	{
		case FF_BUILD_DISPENSER: pEntity = GetDispenser(); break;
		case FF_BUILD_SENTRYGUN: pEntity = GetSentryGun(); break;
		case FF_BUILD_DETPACK: pEntity = GetDetpack(); break;
		case FF_BUILD_MANCANNON: pEntity = GetManCannon(); break;
		default: return NULL; break;
	}

	return pEntity;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFPlayer::IsDisguised( void ) const
{
	return ( GetClassSlot() == CLASS_SPY ) && ( m_iSpyDisguise != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFPlayer::GetDisguisedTeam( void ) const
{
	if( IsDisguised() )	
		return ( m_iSpyDisguise & 0x0000000F );

	return TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFPlayer::GetDisguisedClass( void ) const
{
	if( IsDisguised() )
		return ( ( m_iSpyDisguise & 0x000000F0 ) >> 4 );

	return CLASS_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFPlayer::HasLastDisguise( void ) const
{
	return ( GetClassSlot() == CLASS_SPY ) && ( m_iLastSpyDisguise != 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFPlayer::GetLastDisguisedTeam( void ) const
{
	if( HasLastDisguise() )	
		return ( m_iLastSpyDisguise & 0x0000000F );

	return TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CFFPlayer::GetLastDisguisedClass( void ) const
{
	if( HasLastDisguise() )
		return ( ( m_iLastSpyDisguise & 0x000000F0 ) >> 4 );

	return CLASS_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CFFPlayer::GetMovementSpeed( void ) const
{
	Vector vecVelocity = GetAbsVelocity();
	return FastSqrt(vecVelocity[0] * vecVelocity[0] + vecVelocity[1] * vecVelocity[1]);
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/
void CFFPlayer::FireBullets(const FireBulletsInfo_t &info)
{
	static int	tracerCount;
	trace_t		tr;
	CAmmoDef *	pAmmoDef	= GetAmmoDef();
	int			nDamageType	= pAmmoDef->DamageType(info.m_iAmmoType);
	int			nAmmoFlags	= pAmmoDef->Flags(info.m_iAmmoType);

	// Split the damage up into the number of shots
	float		flDmg = (info.m_iShots ? info.m_flDamage / info.m_iShots : info.m_flDamage);

	// TODO: Should this be false in our mod too?
	bool bDoServerEffects = true;

	// This allows us to specify ourselves what damage we do to players
	int iPlayerDamage = info.m_iPlayerDamage;

	if (iPlayerDamage == 0)
	{
		if (nAmmoFlags & AMMO_INTERPRET_PLRDAMAGE_AS_DAMAGE_TO_PLAYER)
			iPlayerDamage = pAmmoDef->PlrDamage(info.m_iAmmoType);
	}

	// The default attacker is ourselves
	CBaseEntity *pAttacker = info.m_pAttacker ? info.m_pAttacker : this;

	// Make sure we don't have a dangling damage target from a recursive call
	if (g_MultiDamage.GetTarget() != NULL)
		ApplyMultiDamage();

	// Some cleanup stuff
	ClearMultiDamage();
	g_MultiDamage.SetDamageType(nDamageType | DMG_NEVERGIB);

	Vector vecDir, vecEnd;

	CTraceFilterSkipTwoEntities traceFilter(this, info.m_pAdditionalIgnoreEnt, COLLISION_GROUP_NONE);

	// Did bullet start underwater?
	bool bStartedInWater = (enginetrace->GetPointContents(info.m_vecSrc) & (CONTENTS_WATER|CONTENTS_SLIME)) != 0;

	int iSeed = 0;

	// Prediction is only usable on players
	if (IsPlayer())
		iSeed = CBaseEntity::GetPredictionRandomSeed() & 255;

	// Remember to enable this if we change bDoServerEffects
#if defined(HL2MP) && defined(GAME_DLL)
	int iEffectSeed = iSeed;
#endif

	//-----------------------------------------------------
	// Set up our shot manipulator.
	//-----------------------------------------------------
	CShotManipulator Manipulator(info.m_vecDirShooting);

	bool bDoImpacts = false;
	bool bDoTracers = false;

	bool bDoEffects = AllowEffects(entindex(), 0.3f);

#ifdef GAME_DLL
	CFFPlayer *pPlayer = ToFFPlayer(this);

	// Move other players back to history positions based on local player's lag
	lagcompensation->StartLagCompensation(pPlayer, pPlayer->GetCurrentCommand());
#endif

	int nBloodSpurts = 0;

	// Now simulate each shot
	for (int iShot = 0; iShot < info.m_iShots; iShot++)
	{
		bool bHitWater = false;
		bool bHitGlass = false;

		// Prediction is only usable on players
		// Init random system with this seed
		if (IsPlayer())
			RandomSeed(iSeed);

		// If we're firing multiple shots, and the first shot has to be bang on target, ignore spread
		// TODO: Possibly also dot his when m_iShots == 1
		if (iShot == 0 && info.m_iShots > 1 && (info.m_nFlags & FIRE_BULLETS_FIRST_SHOT_ACCURATE))
			vecDir = Manipulator.GetShotDirection();

		// Don't run the biasing code for the player at the moment.
		else
			vecDir = Manipulator.ApplySpread(info.m_vecSpread);

		vecEnd = info.m_vecSrc + vecDir * info.m_flDistance;

		if (IsPlayer() && /*info.m_iShots > 1 &&*/ (iShot % 2) == 0)
		{
			// Half of the shotgun pellets are hulls that make it easier to hit targets with the shotgun.
			//NOTE: This also applies to the AC if you're firing more than 1 bullet at a time!
			AI_TraceHull(info.m_vecSrc, vecEnd, Vector(-3, -3, -3), Vector(3, 3, 3), MASK_SHOT, &traceFilter, &tr);
		}
		else
		{
			// But half aren't
			AI_TraceLine(info.m_vecSrc, vecEnd, MASK_SHOT, &traceFilter, &tr);
		}

#ifdef GAME_DLL
		// Handy debug stuff, I guess
		if (ai_debug_shoot_positions.GetBool())
			NDebugOverlay::Line(info.m_vecSrc, vecEnd, 255, 255, 255, false, 5);
#endif

		// Has this particular bullet hit water yet
		bHitWater = bStartedInWater;

		// Now hit all triggers along the ray that respond to shots...
		// Clip the ray to the first collided solid returned from traceline
		CTakeDamageInfo triggerInfo(ToFFPlayer(pAttacker)->GetActiveFFWeapon(), pAttacker, /*info.m_iDamage */flDmg, nDamageType); // |-- Mirv: Split damage into shots
		CalculateBulletDamageForce(&triggerInfo, info.m_iAmmoType, vecDir, tr.endpos);
		triggerInfo.ScaleDamageForce(info.m_flDamageForceScale);
		triggerInfo.SetAmmoType(info.m_iAmmoType);
#ifdef GAME_DLL
		TraceAttackToTriggers(triggerInfo, tr.startpos, tr.endpos, vecDir);
#endif

		// Make sure given a valid bullet type
		if (info.m_iAmmoType == -1)
		{
			DevMsg("ERROR: Undefined ammo type!\n");
#ifdef GAME_DLL
			lagcompensation->FinishLagCompensation(pPlayer);
#endif
			return;
		}

		Vector vecTracerDest = tr.endpos;

		// Do damage, paint decals
		if (tr.fraction != 1.0)
		{
			// See if the bullet ended up underwater + started out of the water
			if (!bHitWater && (enginetrace->GetPointContents(tr.endpos) & (CONTENTS_WATER|CONTENTS_SLIME)))
			{
				// Only the first shot will do a splash effect, and only if effects
				// are enabled for this burst
				if (iShot == 0 && bDoEffects)
					bHitWater = HandleShotImpactingWater(info, vecEnd, &traceFilter, &vecTracerDest);

				// However, still do the test for bullet impacts
				else
				{
					trace_t	waterTrace;
					AI_TraceLine(info.m_vecSrc, vecEnd, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), &traceFilter, &waterTrace);

					// See if this is the point we entered
					if ((enginetrace->GetPointContents(waterTrace.endpos - Vector(0, 0, 0.1f)) & (CONTENTS_WATER|CONTENTS_SLIME)) == 0)
						bHitWater = true;
				}
			}

			// Probably can move this
			float flActualDamage = /*info.m_iDamage */ flDmg;

			// If we hit a player, and we have player damage specified, use that instead
			// Adrian: Make sure to use the currect value if we hit a vehicle the player is currently driving.
			if (iPlayerDamage)
			{
				if (tr.m_pEnt->IsPlayer())
					flActualDamage = iPlayerDamage;
#ifdef GAME_DLL
				else if (tr.m_pEnt->GetServerVehicle())
				{
					if (tr.m_pEnt->GetServerVehicle()->GetPassenger() && tr.m_pEnt->GetServerVehicle()->GetPassenger()->IsPlayer())
						flActualDamage = iPlayerDamage;
				}
#endif
			}

			// Now some more damage stuff
			int nActualDamageType = nDamageType;
			if (flActualDamage == 0.0)
			{
				flActualDamage = g_pGameRules->GetAmmoDamage(pAttacker, tr.m_pEnt, info.m_iAmmoType);
			}
			else
			{
				nActualDamageType = nDamageType | ((flActualDamage > 16) ? DMG_ALWAYSGIB : DMG_NEVERGIB);
			}

			// Now do the impacts from this shot
			if (!bHitWater || ((info.m_nFlags & FIRE_BULLETS_DONT_HIT_UNDERWATER) == 0))
			{
				// Damage specified by function parameter
				CTakeDamageInfo dmgInfo(ToFFPlayer(pAttacker)->GetActiveFFWeapon(), pAttacker, flActualDamage, nActualDamageType);
				CalculateBulletDamageForce(&dmgInfo, info.m_iAmmoType, vecDir, tr.endpos);
				dmgInfo.SetAmmoType(info.m_iAmmoType);
				dmgInfo.ScaleDamageForce(info.m_flDamageForceScale);

				// Reduce push for players
				if (tr.m_pEnt->IsPlayer())
				{
					dmgInfo.ScaleDamageForce(0.01f);
				}

				tr.m_pEnt->DispatchTraceAttack(dmgInfo, vecDir, &tr);

				if (bStartedInWater || !bHitWater || (info.m_nFlags & FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS))
				{
					// Only draw impact effects when you do a tracer, or this weapon doesnt have tracers
					// this helps cut down the effect message spam for the AC - AfterShock 
					if ((info.m_iTracerFreq == 0) || (tracerCount % FF_AC_IMPACTFREQ ) == 0)
					{
						if (bDoServerEffects)
						{
							// Is the entity valid, and the surface drawable on?
							if (tr.fraction < 1.0f && tr.m_pEnt && !(tr.surface.flags & (SURF_SKY|SURF_NODRAW)))
							{
								// Build the impact data
								CEffectData data;
								data.m_vOrigin = tr.endpos;
								data.m_vStart = tr.startpos;
								data.m_nSurfaceProp = tr.surface.surfaceProps;
								data.m_nDamageType = nDamageType;
								data.m_nHitBox = tr.hitbox;

	#ifdef GAME_DLL
								data.m_nEntIndex = tr.m_pEnt->entindex();
	#else
								data.m_hEntity = tr.m_pEnt;
	#endif

								// Always do impact effects for the first few blood spurts.
								// Otherwise we might not show them and that's bad feedback
								// Not sure if we should check bDoEffects or not really.
								if (tr.m_pEnt->IsPlayer() && nBloodSpurts < 3 && bDoEffects)
									nBloodSpurts++;
								// Otherwise the impact effects for the 4th shot onwards are optional (depends
								// on client's cl_effectdetail)
								else if (iShot > 2 || !bDoEffects)
									data.m_fFlags |= CEFFECT_EFFECTNOTNEEDED;

								// No sound for all but the first few
								if (iShot > 2)
									data.m_fFlags |= CEFFECT_SOUNDNOTNEEDED;

								// Send it off
								DispatchEffect("Impact", data);
							}
						}
						else
							bDoImpacts = true;
					}// end if tracers
				} 
				else
				{
					// We may not impact, but we DO need to affect ragdolls on the client
					CEffectData data;
					data.m_vStart = tr.startpos;
					data.m_vOrigin = tr.endpos;
					data.m_nDamageType = nDamageType;

					DispatchEffect("RagdollImpact", data);
				}

#ifdef GAME_DLL
				// Make sure if the player is holding this, he drops it
				if (nAmmoFlags & AMMO_FORCE_DROP_IF_CARRIED)
					Pickup_ForcePlayerToDropThisObject(tr.m_pEnt);		
#endif
			}
		}

		// See if we hit glass
		if (tr.m_pEnt != NULL)
		{
#ifdef GAME_DLL
			surfacedata_t *psurf = physprops->GetSurfaceData(tr.surface.surfaceProps);
			if ((psurf != NULL) && (psurf->game.material == CHAR_TEX_GLASS) && (tr.m_pEnt->ClassMatches("func_breakable")))
				bHitGlass = true;
#endif
		}

		// Do the tracers if required
		if ((info.m_iTracerFreq != 0) && (tracerCount++ % info.m_iTracerFreq) == 0 && (bHitGlass == false))
		{
			if (bDoServerEffects)
			{
				Vector vecTracerSrc = vec3_origin;
				ComputeTracerStartPosition(info.m_vecSrc, &vecTracerSrc);

				trace_t Tracer;
				Tracer = tr;
				Tracer.endpos = vecTracerDest;

				MakeTracer(vecTracerSrc, Tracer, pAmmoDef->TracerType(info.m_iAmmoType));
			}
			else
				bDoTracers = true;
		}

		// See if we should pass through glass
#ifdef GAME_DLL
		if (bHitGlass)
			HandleShotImpactingGlass(info, tr, vecDir, &traceFilter);
#endif

		iSeed++;
	}

#ifdef GAME_DLL
	lagcompensation->FinishLagCompensation(pPlayer);
#endif

	// Client side effects
#if defined(HL2MP) && defined(GAME_DLL)
	if (!bDoServerEffects && bDoEffects)
		TE_HL2MPFireBullets(entindex(), tr.startpos, info.m_vecDirShooting, info.m_iAmmoType, iEffectSeed, info.m_iShots, info.m_vecSpread.x, bDoTracers, bDoImpacts);
#endif

#ifdef GAME_DLL
	ApplyMultiDamage();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFPlayer::HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest)
{
	trace_t	waterTrace;

	// Trace again with water enabled
	AI_TraceLine(info.m_vecSrc, vecEnd, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), pTraceFilter, &waterTrace);

	// See if this is the point we entered
	if ((enginetrace->GetPointContents(waterTrace.endpos - Vector(0, 0, 0.1f)) & (CONTENTS_WATER|CONTENTS_SLIME)) == 0)
		return false;

	if (ShouldDrawWaterImpacts())
	{
		int	nMinSplashSize = GetAmmoDef()->MinSplashSize(info.m_iAmmoType);
		int	nMaxSplashSize = GetAmmoDef()->MaxSplashSize(info.m_iAmmoType);

		float flSplashModifier = 1.0f + info.m_iShots * 0.1f;	// |-- Mirv: Modify splashes by shot count

		CEffectData	data;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = random->RandomFloat(nMinSplashSize, nMaxSplashSize) * flSplashModifier;	// |-- Mirv: Modify splashes by shot count
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}
		DispatchEffect("gunshotsplash", data);
	}

	*pVecTracerDest = waterTrace.endpos;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Shared cloak code
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SpyCloak(const CCommand& args)
{
	// Jon: always allow uncloaking if already cloaked
	if( IsCloaked() )
	{
		// Can only cloak every ffdev_spy_nextcloak seconds
		m_flNextCloak = gpGlobals->curtime + SPY_NEXTCLOAK;
		Cloak();
#ifdef GAME_DLL
		SpyCloakFadeOut();
#endif
		return;
	}

	if( !IsCloakable() )
	{		
#ifdef GAME_DLL
		Omnibot::Notify_CantCloak(this);
#endif
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK" );
		return;
	}

	// 0001379: can cloak only if on the ground
	// added: or also not swimming
	if ( !(GetFlags() & FL_ONGROUND || GetWaterLevel() > WL_NotInWater) )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK_MUSTBEONGROUND" );
		return;
	}

	// Check if we can cloak yet
	if( m_flNextCloak > gpGlobals->curtime )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK_TIMELIMIT" );
		return;
	}

	// Can only cloak every ffdev_spy_nextcloak seconds
	m_flNextCloak = gpGlobals->curtime + SPY_NEXTCLOAK;

#ifdef GAME_DLL
	// Regular cloak
	m_bCloakFadeType = false;
#endif

	Cloak();

#ifdef GAME_DLL
	SpyCloakFadeIn();
#endif	
}

//-----------------------------------------------------------------------------
// Purpose: Shared smart cloak code
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SpySmartCloak(const CCommand& args)
{
	// Jon: always allow uncloaking if already cloaked
	if( IsCloaked() )
	{
		// Can only cloak every ffdev_spy_nextcloak seconds
		m_flNextCloak = gpGlobals->curtime + SPY_NEXTCLOAK;
		Cloak();
#ifdef GAME_DLL
		SpyCloakFadeOut();
#endif
		return;
	}

	if( !IsCloakable() )
	{
#ifdef GAME_DLL
		Omnibot::Notify_CantCloak(this);
#endif
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK" );
		return;
	}

	// 0001379: can cloak only if on the ground
	// added: or also not swimming
	if ( !(GetFlags() & FL_ONGROUND || GetWaterLevel() > WL_NotInWater) )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK_MUSTBEONGROUND" );
		return;
	}

	// Check if we can cloak yet
	if( m_flNextCloak > gpGlobals->curtime )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK_TIMELIMIT" );
		return;
	}

	// Can only cloak every ffdev_spy_nextcloak seconds
	m_flNextCloak = gpGlobals->curtime + SPY_NEXTCLOAK;

	// Silent cloak must be done while not moving! But if we're
	// already cloaked we'll allow it so the player can uncloak
	// Jon: adding in minimum allowed speed cvar
	if( GetLocalVelocity().Length() > SPY_SCLOAK_MINSTARTVELOCITY )
	{
#ifdef GAME_DLL	
		// normal cloak
		m_bCloakFadeType = false;
#endif
	}
	else
	{
#ifdef GAME_DLL	
		// Silent cloak
		m_bCloakFadeType = true;
#endif
	}
	Cloak();

#ifdef GAME_DLL
	SpyCloakFadeIn();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Shared silent cloak code
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SpySilentCloak(const CCommand& args)
{
	// Jon: always allow uncloaking if already cloaked
	if( IsCloaked() )
	{
		// Can only cloak every ffdev_spy_nextcloak seconds
		m_flNextCloak = gpGlobals->curtime + SPY_NEXTCLOAK;
		Cloak();
#ifdef GAME_DLL
		SpyCloakFadeOut();
#endif
		return;
	}

	if( !IsCloakable() )
	{
#ifdef GAME_DLL
		Omnibot::Notify_CantCloak(this);
#endif
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK" );
		return;
	}

	// 0001379: can cloak only if on the ground
	// added: or also not swimming
	if ( !(GetFlags() & FL_ONGROUND || GetWaterLevel() > WL_NotInWater) )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK_MUSTBEONGROUND" );
		return;
	}

	// Check if we can cloak yet
	if( m_flNextCloak > gpGlobals->curtime )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CANTCLOAK_TIMELIMIT" );
		return;
	}

	// Can only cloak every ffdev_spy_nextcloak seconds
	m_flNextCloak = gpGlobals->curtime + SPY_NEXTCLOAK;

	// Silent cloak must be done while not moving! But if we're
	// already cloaked we'll allow it so the player can uncloak
	// Jon: adding in minimum allowed speed cvar
	if( GetLocalVelocity().Length() > SPY_SCLOAK_MINSTARTVELOCITY )
	{
		// Reset next cloak time since player technically didn't cloak yet
		m_flNextCloak = gpGlobals->curtime + 0.2;

		ClientPrint( this, HUD_PRINTCENTER, "#FF_SILENTCLOAK_MUSTBESTILL" );
		return;
	}

#ifdef GAME_DLL	
	// Silent cloak
	m_bCloakFadeType = true;
#endif

	Cloak();

#ifdef GAME_DLL
	SpyCloakFadeIn();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: The actual cloak stuff
//-----------------------------------------------------------------------------
void CFFPlayer::Cloak( void )
{
#ifdef CLIENT_DLL 
#else
	// Already Cloaked so remove all effects
	if( IsCloaked() )
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_UNCLOAK" );

		// Yeah we're not Cloaked anymore bud
		m_iCloaked = 0;

		// If we're currently disguising, remove some time (50%)
		//if( m_flFinishDisguise > gpGlobals->curtime )
		//	m_flFinishDisguise -= ( m_flFinishDisguise - gpGlobals->curtime ) * 0.5f;

		//// Redeploy our weapon
		//if( GetActiveWeapon() && ( GetActiveWeapon()->IsWeaponVisible() == false ) )
		//{
		//	GetActiveWeapon()->Deploy();
		//	ShowCrosshair( true );
		//}

		// If regular cloak cleanup ragdoll
		if( !m_bCloakFadeType )
		{
			// Cleanup ragdoll
			CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll * >( m_hRagdoll.Get() );
			if( pRagdoll )
			{
				// Check if the ragdoll had flames on it
				CEntityFlame *pFlame = dynamic_cast< CEntityFlame * >( pRagdoll->GetEffectEntity() );
				if( pFlame )
					pFlame->Extinguish();

				// Remove the ragdoll instantly
				pRagdoll->SetThink( &CBaseEntity::SUB_Remove );
				pRagdoll->SetNextThink( gpGlobals->curtime );
			}
		}		

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "uncloaked" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", this->GetUserID() );
			gameeventmanager->FireEvent( pEvent, true );
		}

		// We are uncloaking so reset this
		m_bCloakFadeType = false;
	}
	// Not already cloaked
	else
	{
		// Announce being cloaked
		m_iCloaked = 1;

		// If regular cloak, scream
		if( !m_bCloakFadeType )
			EmitSoundShared( "Player.Death" );

		ClientPrint( this, HUD_PRINTCENTER, "#FF_CLOAK" );		

		m_flCloakTime = gpGlobals->curtime;

		// If we're currently disguising, add on some time (50%)
		//if( m_flFinishDisguise > gpGlobals->curtime )
		//	m_flFinishDisguise += ( m_flFinishDisguise - gpGlobals->curtime ) * 0.5f;

		//// Holster our current weapon
		//if( GetActiveWeapon() )
		//	GetActiveWeapon()->Holster( NULL );

		// Remove any decals on us
		RemoveAllDecals();		

		// If regular cloak, create ragdoll
		if( !m_bCloakFadeType )
		{
			// Create our ragdoll using this function (we could just c&p it and modify it i guess)
			CreateRagdollEntity();

			CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll * >( m_hRagdoll.Get() );
			if( pRagdoll )
			{
				pRagdoll->m_vecRagdollVelocity = 100.0f * GetLocalVelocity();
				pRagdoll->SetThink( NULL );
			}
		}		

		CFFLuaSC hOwnerCloak( 1, this );
		// Find any items that we are in control of and let them know we Cloaked
		CFFInfoScript *pEnt = ( CFFInfoScript * )gEntList.FindEntityByOwnerAndClassT( NULL, ( CBaseEntity * )this, CLASS_INFOSCRIPT );
		while( pEnt != NULL )
		{
			// Tell the ent that it Cloaked
			_scriptman.RunPredicates_LUA( pEnt, &hOwnerCloak, "onownercloak" );

			// Next!
			pEnt = ( CFFInfoScript * )gEntList.FindEntityByOwnerAndClassT( pEnt, ( CBaseEntity * )this, CLASS_INFOSCRIPT );
		}

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "cloaked" );						
		if( pEvent )
		{
			pEvent->SetInt( "userid", this->GetUserID() );
			gameeventmanager->FireEvent( pEvent, true );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: HW attack2
//-----------------------------------------------------------------------------
void CFFPlayer::Overpressure( void )
{
	if (!IsAlive())
	{
		return;
	}

#ifdef CLIENT_DLL
	if( prediction->InPrediction() && !prediction->IsFirstTimePredicted() )
		return;
#endif

	CEffectData data;
	data.m_vOrigin = GetAbsOrigin();
	
	DispatchEffect(OVERPRESSURE_EFFECT, data);

	// Play a sound
	EmitSoundShared("overpressure.explode");

#ifdef CLIENT_DLL
	FF_SendHint( HWGUY_OVERPRESS, 3, PRIORITY_NORMAL, "#FF_HINT_HWGUY_OVERPRESS" );
#endif

#ifdef GAME_DLL
	Extinguish(); // Overpressure stops you burning
#endif

	for (int i=1; i<=gpGlobals->maxClients; i++)
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );

		if (!pPlayer)
			continue;

		if( !pPlayer->IsAlive() || pPlayer->IsObserver() )
			continue;
		
		// People who are building shouldn't be pushed around by anything
		if (pPlayer->IsStaticBuilding())
			continue;
#ifdef GAME_DLL
		pPlayer->Extinguish(); // Overpressure extinguishes fire from everyone
#endif
		// Ignore people that can't take damage (teammates when friendly fire is off)
		if (OVERPRESSURE_IGNOREFRIENDLY && !g_pGameRules->FCanTakeDamage( pPlayer, this ))
			continue;

		// Some useful things to know
		Vector vecPlayerOrigin = pPlayer->GetAbsOrigin();
		Vector vecDisplacement = vecPlayerOrigin - GetAbsOrigin();
		float flDistance = vecDisplacement.Length();

		if (flDistance > OVERPRESSURE_RADIUS)
			continue;

		Vector vecDir = vecDisplacement;
		vecDir.NormalizeInPlace();

		Vector vecResult;
		if (pPlayer == this)
		{
			float flSelfLateral = OVERPRESSURE_SELFPUSH_HORIZONTAL;
			float flSelfVertical = OVERPRESSURE_SELFPUSH_VERTICAL;

			Vector vecVelocity = pPlayer->GetAbsVelocity();
			Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
			float flHorizontalSpeed = vecLatVelocity.Length();

#ifdef GAME_DLL
			if (OVERPRESSURE_SLIDE_AFFECTSSELF)
				pPlayer->StartSliding( OVERPRESSURE_SLIDE_DURATION, OVERPRESSURE_SLIDE_DURATION );
#endif

			// apply push force
			if (pPlayer->GetFlags() & FL_ONGROUND)
			{
				vecResult = Vector(vecVelocity.x * flSelfLateral, vecVelocity.y  * flSelfLateral, (vecVelocity.z + 90)* flSelfVertical);
				DevMsg("[HW attack2] on ground (%f)\n", flHorizontalSpeed);
			}
			else
			{
				vecResult = Vector(vecVelocity.x * flSelfLateral, vecVelocity.y * flSelfLateral, vecVelocity.z * flSelfVertical);
				DevMsg("[HW attack2] in air (%f)\n", flHorizontalSpeed);
			}
		}
		else
		{
			float flFriendlyScale = 1.0f;

			// Check if is a teammate and scale accordingly
			if (g_pGameRules->PlayerRelationship(pPlayer, this) == GR_TEAMMATE)
				flFriendlyScale = OVERPRESSURE_FRIENDLYSCALE;

			QAngle angDirection;
			VectorAngles(vecDir, angDirection);

			pPlayer->ViewPunch(angDirection * OVERPRESSURE_JERKMULTI * flDistance);

#ifdef GAME_DLL
			if (OVERPRESSURE_SLIDE)
				pPlayer->StartSliding( OVERPRESSURE_SLIDE_DURATION, OVERPRESSURE_SLIDE_DURATION );
#endif


			Vector vecVelocity = pPlayer->GetAbsVelocity();
			Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
			float flHorizontalSpeed = vecLatVelocity.Length();

			float flSpeedPercent = OVERPRESSURE_SPEED_PERCENT;

			float flLateral = OVERPRESSURE_PUSH_HORIZONTAL * flFriendlyScale;
			float flVertical = OVERPRESSURE_PUSH_VERTICAL * flFriendlyScale;

			if (flHorizontalSpeed > pPlayer->MaxSpeed() * flSpeedPercent)
			{
				float flSpeedMultiplier = flHorizontalSpeed / pPlayer->MaxSpeed() - flSpeedPercent + 1;

				float flSpeedMultiplierHorizontal = OVERPRESSURE_SPEED_MULTIPLIER_HORIZONTAL * flSpeedMultiplier;
				float flSpeedMultiplierVertical = OVERPRESSURE_SPEED_MULTIPLIER_VERTICAL * flSpeedMultiplier;

				vecResult = Vector(vecDir.x * flLateral * flSpeedMultiplierHorizontal, vecDir.y * flLateral * flSpeedMultiplierHorizontal, vecDir.z * flVertical * flSpeedMultiplierVertical);
				DevMsg("[HW attack2] enemy going supersonic (speed: %f direction: %f,%f,%f)\n", flHorizontalSpeed, vecDir.x, vecDir.y, vecDir.z);
			}
			else
			{
				// apply push force
				if (pPlayer->GetFlags() & FL_ONGROUND)
				{
					float flGroundPush = OVERPRESSURE_GROUNDPUSH_MULTIPLIER;

					vecResult = Vector(vecDir.x * flLateral * flGroundPush, vecDir.y  * flLateral * flGroundPush, vecDir.z * flVertical);
					DevMsg("[HW attack2] enemy on ground, under speed (speed: %f direction: %f,%f,%f)\n", flHorizontalSpeed, vecDir.x, vecDir.y, vecDir.z);
				}
				else
				{
					vecResult = Vector(vecDir.x * flLateral, vecDir.y * flLateral, vecDir.z * flVertical);
					DevMsg("[HW attack2] enemy in air, under speed (speed: %f direction: %f,%f,%f)\n", flHorizontalSpeed, vecDir.x, vecDir.y, vecDir.z);
				}
			}
		}

#ifdef GAME_DLL
		// cap mancannon + overpressure speed
		if ( pPlayer->m_flMancannonTime && gpGlobals->curtime < pPlayer->m_flMancannonTime + 5.2f )
		{
			if ( vecResult.Length() > 1700.0f )
			{
				vecResult.NormalizeInPlace();
				vecResult *= 1700.0f;
			}
		}
#endif
		pPlayer->SetAbsVelocity(vecResult);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shared pre think code
//-----------------------------------------------------------------------------
void CFFPlayer::SharedPreThink( void )
{
	// Do we need to do a class specific skill?
	if( m_afButtonPressed & IN_ATTACK2 )
		ClassSpecificSkill();
	else if (m_afButtonReleased & IN_ATTACK2)
		ClassSpecificSkill_Post();

	// Do we need to do a class specific skill?
	if (m_nButtons & IN_ATTACK2)
		ClassSpecificSkillHold();

	JetpackRechargeThink();
#ifdef GAME_DLL
	if (m_bQueueDetonation)
	{
		if ((GetPipebombShotTime() + PIPE_DET_DELAY) < gpGlobals->curtime)
		{
			m_bQueueDetonation = false;
			CFFProjectilePipebomb::DestroyAllPipes(this);
		}
	}
#endif

	// grenades
	// 2-tap binds
	// HACK: this is not what m_bLastPrimed is for
	if ( m_bLastPrimed && m_afButtonPressed & ( IN_TOGGLEGRENADE1 | IN_TOGGLEGRENADE2 ) && IsGrenadePrimed() ) // throw either grenade on releasing key (2-tap)
	{
		m_bLastPrimed = false;
		ThrowPrimedGrenade();
		return;
	}
	else if ( m_afButtonPressed & IN_TOGGLEGRENADE1 )
	{
		if ( !IsGrenade1Primed() )
			PrimeGrenade1();

#ifdef CLIENT_DLL
		// Hint Code: Check for 2 consecutive unthrown grenades (player got blowed up!)
		if (m_bLastPrimed == true)
		{
			m_iUnthrownGrenCount++;
			if (m_iUnthrownGrenCount > 1)
			{
				FF_SendHint(GLOBAL_NOPRIME2, 2, PRIORITY_NORMAL, "#FF_HINT_GLOBAL_NOPRIME2");
				m_iUnthrownGrenCount = 0;
				m_bLastPrimed = false;
			}
		}
		else
#endif
			m_bLastPrimed = true;
	}
	else if ( m_afButtonPressed & IN_TOGGLEGRENADE2 )
	{
		if ( !IsGrenade2Primed() )
			PrimeGrenade2();

#ifdef CLIENT_DLL
		// Hint Code: Check for 2 consecutive unthrown grenades (player got blowed up!)
		if (m_bLastPrimed == true)
		{
			m_iUnthrownGrenCount++;
			if (m_iUnthrownGrenCount > 1)
			{
				FF_SendHint(GLOBAL_NOPRIME2, 2, PRIORITY_NORMAL, "#FF_HINT_GLOBAL_NOPRIME2");
				m_iUnthrownGrenCount = 0;
				m_bLastPrimed = false;
			}
		}
		else
#endif
			m_bLastPrimed = true;
	}

	// hold binds
	if ( m_afButtonPressed & IN_GRENADE1 && !IsGrenade1Primed() ) // prime grenade 1 (hold)
	{
		// throw the other primed grenade before priming another one
		if ( IsGrenade2Primed() )
		{
			ThrowPrimedGrenade();
			return; // allowing player to prime another right away breaks shit
		}

		PrimeGrenade1();
	}
	else if ( m_afButtonPressed & IN_GRENADE2 && !IsGrenade2Primed() ) // prime grenade 2 (hold)
	{
		// throw the other primed grenade before priming another one
		if ( IsGrenade1Primed() )
		{
			ThrowPrimedGrenade();
			return; // allowing player to prime another right away breaks shit
		}

		PrimeGrenade2();
	}
	else if ( m_afButtonReleased & IN_GRENADE1 && IsGrenade1Primed() && !m_bWantToThrowGrenade ) // throw grenade 1 on releasing key (hold)
	{
		ThrowPrimedGrenade();
	}
	else if ( m_afButtonReleased & IN_GRENADE2 && IsGrenade2Primed() && !m_bWantToThrowGrenade ) // throw grenade 1 on releasing key (hold)
	{
		ThrowPrimedGrenade();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayer::JetpackRechargeThink( void )
{
	if (GetClassSlot() != CLASS_PYRO)
	{
		return;
	}

	if (m_flJetpackNextFuelRechargeTime < gpGlobals->curtime)
	{
		if (m_iJetpackFuel < JETPACK_MAXFUEL)
		{
			m_flJetpackNextFuelRechargeTime = gpGlobals->curtime + JETPACK_FUELRECHARGETIME;
			// dont forget: we doubled total fuel and hover cost when converrting to int,
			// double fuel recharge rate to match old rates here
			m_iJetpackFuel += 2;
		}
	}
}

bool CFFPlayer::CanJetpack()
{
	if (!IsAlive())
	{
		return false;
	}

	if (GetFlags() & FL_ONGROUND) // no jetpacking on ground
	{
		return false;
	}

	if (m_iJetpackFuel < JETPACK_MINFUEL)
	{
		return false;
	}

	if ( !m_bCanUseJetpack )
	{
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Jetpack jump
//-----------------------------------------------------------------------------
void CFFPlayer::JetpackHold( void )
{
	if (!CanJetpack())
	{
		m_bJetpacking = false;
		return;
	}

	m_bJetpacking = true;
	m_iJetpackFuel -= JETPACK_FUELHOVERCOST;

	Vector vecForward, vecRight, vecUp;
	EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );
	VectorNormalizeFast( vecRight );

	// get only the direction the player is looking (ignore any z)
	Vector horizPush = CrossProduct(Vector( 0.0f, 0.0f, 1.0f ), vecRight);

	float flPercent = 1.0f;

	Vector vecLatVelocity = GetAbsVelocity() * Vector(1.0f, 1.0f, 0.0f);
	float curVertical = GetAbsVelocity().z;

	if (vecLatVelocity.IsLengthGreaterThan(JETPACK_HORIZONTALPUSH_CAP))
	{
		horizPush *= 0;
	}
	else
	{
		horizPush *= JETPACK_HORIZONTALPUSH_OFFGROUND;
	}

	float verticalPush = JETPACK_VERTICALPUSH_OFFGROUND;
	if (curVertical < 0)
	{
		float verticalScale = (FFDEV_JETPACK_VERTICALPUSH_DOWNWARDSLIMIT + curVertical) / FFDEV_JETPACK_VERTICALPUSH_DOWNWARDSLIMIT;
		if (verticalScale < JETPACK_VERTICALPUSH_OFFGROUND_DOWNSCALE)
			verticalScale = JETPACK_VERTICALPUSH_OFFGROUND_DOWNSCALE;

		verticalPush *= verticalScale;
	}

	ApplyAbsVelocityImpulse(Vector(horizPush.x, horizPush.y, verticalPush) * flPercent);
}

//-----------------------------------------------------------------------------
// Purpose: If cloaked we use no damage decal
//-----------------------------------------------------------------------------
char const *CFFPlayer::DamageDecal( int bitsDamageType, int gameMaterial )
{
	if( IsCloaked() )
		return "";

	return BaseClass::DamageDecal( bitsDamageType, gameMaterial );
}

//-----------------------------------------------------------------------------
// Purpose: Shared ammome
//-----------------------------------------------------------------------------
void CFFPlayer::Command_AmmoMe(const CCommand& args)
{
	if( m_flSaveMeTime < gpGlobals->curtime )
	{
#ifdef GAME_DLL
		m_bAmmoMe = true; // AfterShock: this is only used for seeing other peoples icons, so no need for client to predict his own state
#endif
		// Set the time we can do another saveme/engyme/ammome at
		m_flSaveMeTime = gpGlobals->curtime + 5.0f;

		// Call for ammo
		EmitSoundShared("ammo.saveme");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shared saveme
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SaveMe(const CCommand& args)
{
	if( m_flSaveMeTime < gpGlobals->curtime )
	{
#ifdef GAME_DLL
		m_bSaveMe = true; // AfterShock: this is only used for seeing other peoples icons, so no need for client to predict his own state
#endif
		// Set the time we can do another saveme at
		m_flSaveMeTime = gpGlobals->curtime + 5.0f;

		if (IsInfected())
			EmitSoundShared( "infected.saveme" );
		else
			EmitSoundShared( "medical.saveme" );

#ifdef GAME_DLL
		// Hint Code -- Event: Allied player within 1000 units calls for medic
		CBaseEntity *ent = NULL;
		for( CEntitySphereQuery sphere( GetAbsOrigin(), 1000 ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( ent->IsPlayer() )
			{
				CFFPlayer *player = ToFFPlayer( ent );
				// Only alive friendly medics within 1000 units are sent this hint
				if( player && ( player != this ) && player->IsAlive() && ( g_pGameRules->PlayerRelationship( this, player ) == GR_TEAMMATE ) && ( player->GetClassSlot() == CLASS_MEDIC ) )
					FF_SendHint( player, MEDIC_GOHEAL, 5, PRIORITY_NORMAL, "#FF_HINT_MEDIC_GOHEAL" );  // Go heal that dude!
			}
		}
		// End Hint Code
#endif
	}
}

//-----------------------------------------------------------------------------
// Purpose: Shared engyme
//-----------------------------------------------------------------------------
void CFFPlayer::Command_EngyMe(const CCommand& args)
{
	if( m_flSaveMeTime < gpGlobals->curtime )
	{
#ifdef GAME_DLL
		m_bEngyMe = true; // AfterShock: this is only used for seeing other peoples icons, so no need for client to predict his own state
#endif
		// Set the time we can do another engyme at
		m_flSaveMeTime = gpGlobals->curtime + 5.0f;

		EmitSoundShared("maintenance.saveme");

		// Hint Code -- Event: Allied player within 1000 units calls for engy
#ifdef GAME_DLL
		CBaseEntity *ent = NULL;
		for( CEntitySphereQuery sphere( GetAbsOrigin(), 1000 ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( ent->IsPlayer() )
			{
				CFFPlayer *player = ToFFPlayer( ent );
				// Only alive friendly engies within 1000 units are sent this hint
				if( player && ( player != this ) && player->IsAlive() && ( g_pGameRules->PlayerRelationship( this, player ) == GR_TEAMMATE ) && ( player->GetClassSlot() == CLASS_ENGINEER ) )
					FF_SendHint( player, ENGY_GOSMACK, 5, PRIORITY_NORMAL, "#FF_HINT_ENGY_GOSMACK" );  // Go wrench that dude!
			}
		}
		// End Hint Code
#endif
	}
}

bool CFFPlayer::IsGrenade1Primed( void )
{
	return ( ( m_iGrenadeState == FF_GREN_PRIMEONE ) );
}
bool CFFPlayer::IsGrenade2Primed( void )
{
	return ( ( m_iGrenadeState == FF_GREN_PRIMETWO ) );
}
bool CFFPlayer::IsGrenadePrimed( void )
{
	return IsGrenade1Primed() || IsGrenade2Primed();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFPlayer::PrimeGrenade1( void )
{
#ifdef CLIENT_DLL
	if ( !engine->IsConnected() || !engine->IsInGame() )
		return;
#endif

	if (IsGrenadePrimed())
		return;

	// Don't want timers going when frozen
	if ( GetFlags() & FL_FROZEN )
		return;

	if ( IsStaticBuilding() )
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when Cloaked
	if ( IsCloaked() )
		return;

	// Bug #0000176: Sniper gren2 shouldn't trigger timer.wav
	// Bug #0000064: Civilian has primary & secondary grenade.
	if ( m_iPrimary <= 0 )
	{
		//DevMsg("[Grenades] You are out of primary grenades!\n");
		return;
	}

	// Bug #0000169: Grenade timer is played when player is dead and primes a grenade
	if ( !IsAlive() || GetTeamNumber() < TEAM_BLUE )
		return;

	// Make sure we can't insta-prime on the client either
	// This should match the grenade throw delay with the server
	// otherwise it results in ghost nades
	if (gpGlobals->curtime < m_flPrimeTime + GREN_THROW_DELAY)
		return;

	m_flPrimeTime = gpGlobals->curtime;
	m_iGrenadeState = FF_GREN_PRIMEONE;

	EmitSoundShared("Grenade.Prime");

#ifndef _DEBUG
	m_iPrimary--;
#endif

#ifdef CLIENT_DLL
	// dexter: if g_szTimerFile hasnt been set yet, force update to default. this happens first run of a new install etc
	ConVarRef cl_timerwav("cl_grenadetimer");

	if (Q_strlen(g_szTimerFile) < 1 && cl_timerwav.IsValid())
		TimerChange_Callback(cl_timerwav.GetLinkedConVar(), NULL, NULL);

	CPASAttenuationFilter filter(this, g_szTimerFile);

	EmitSound_t params;
	params.m_pSoundName = g_szTimerFile;
	params.m_flSoundTime = 0.0f;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = false;

	EmitSound(filter, entindex(), params);

	Assert(m_pGrenade1Timer);
	m_pGrenade1Timer->SetTimer(3.81f);
#else
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// we have a primary grenade type
	if (strcmp( pPlayerClassInfo.m_szPrimaryClassName, "None" ))
	{
		// Jiggles: Added lua callback for squeek's training map
		CFFLuaSC hContext( 1, this );
		_scriptman.RunPredicates_LUA( NULL, &hContext, "player_onprimegren1" );
		// End callback
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFPlayer::PrimeGrenade2( void )
{
#ifdef CLIENT_DLL
	if ( !engine->IsConnected() || !engine->IsInGame() )
		return;
#endif

	if ( IsGrenadePrimed() )
		return;

	// Don't want timers going when frozen
	if ( GetFlags() & FL_FROZEN )
		return;

	if ( IsStaticBuilding() )
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when Cloaked
	if ( IsCloaked() )
		return;

	// Bug #0000176: Sniper gren2 shouldn't trigger timer.wav
	// Bug #0000064: Civilian has primary & secondary grenade.
	if ( m_iSecondary <= 0 )
	{
		//DevMsg("[Grenades] You are out of primary grenades!\n");
		return;
	}

	// Bug #0000169: Grenade timer is played when player is dead and primes a grenade
	if ( !IsAlive() || GetTeamNumber() < TEAM_BLUE )
		return;

	// Make sure we can't insta-prime on the client either
	// This should match the grenade throw delay with the server
	// otherwise it results in ghost nades
	if (gpGlobals->curtime < m_flPrimeTime + GREN_THROW_DELAY)
		return;

	m_flPrimeTime = gpGlobals->curtime;
	m_iGrenadeState = FF_GREN_PRIMETWO;

	EmitSoundShared("Grenade.Prime");

#ifndef _DEBUG
	m_iSecondary--;
#endif

#ifdef CLIENT_DLL
	// dexter: if g_szTimerFile hasnt been set yet, force update to default. this happens first run of a new install etc
	ConVarRef cl_timerwav("cl_grenadetimer");

	if (Q_strlen(g_szTimerFile) < 1 && cl_timerwav.IsValid())
		TimerChange_Callback(cl_timerwav.GetLinkedConVar(), NULL, NULL);

	CPASAttenuationFilter filter(this, g_szTimerFile);

	EmitSound_t params;
	params.m_pSoundName = g_szTimerFile;
	params.m_flSoundTime = 0.0f;
	params.m_pflSoundDuration = NULL;
	params.m_bWarnOnDirectWaveReference = false;

	EmitSound(filter, entindex(), params);

	Assert(m_pGrenade2Timer);
	m_pGrenade2Timer->SetTimer(3.81f);
#else
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// we have a primary grenade type
	if (strcmp( pPlayerClassInfo.m_szSecondaryClassName, "None" ))
	{
		// Jiggles: Added lua callback for squeek's training map
		CFFLuaSC hContext( 1, this );
		_scriptman.RunPredicates_LUA( NULL, &hContext, "player_onprimegren2" );
		// End callback

		// Hint Code
		//Msg("\nSecondary Class Name: %s\n", pPlayerClassInfo.m_szSecondaryClassName );
		//if ( strcmp( pPlayerClassInfo.m_szSecondaryClassName, "ff_grenade_nail" ) == 0 )
		//	FF_SendHint( this, SOLDIER_NAILGREN, 4, PRIORITY_NORMAL, "#FF_HINT_SOLDIER_NAILGREN" );
		//else if ( strcmp( pPlayerClassInfo.m_szSecondaryClassName, "ff_grenade_concussion" ) == 0 )
		//	FF_SendHint( this, SCOUT_CONC1, 1, PRIORITY_NORMAL, "#FF_HINT_SCOUT_CONC1" );

		// Jiggles: Let's try it this way to avoid the above string compares
		switch( GetClassSlot() )
		{
			case CLASS_SOLDIER: 
				FF_SendHint( this, SOLDIER_LASERGREN, 1, PRIORITY_NORMAL, "#FF_HINT_SOLDIER_LASERGREN" );
				break;
			case CLASS_HWGUY:
				FF_SendHint( this, HWGUY_SLOWFIELD, 1, PRIORITY_NORMAL, "#FF_HINT_HWGUY_SLOWFIELD" );
				break;
			case CLASS_MEDIC:
			case CLASS_SCOUT:
				FF_SendHint( this, SCOUT_CONC1, 1, PRIORITY_NORMAL, "#FF_HINT_SCOUT_CONC1" );
				break;
		}
		// End hint code
	}
#endif
}

void CFFPlayer::ResetGrenadeState(void)
{
	m_bWantToThrowGrenade = false;
	m_iGrenadeState = FF_GREN_NONE;
	m_flPrimeTime = 0.0f;
#ifdef CLIENT_DLL
	m_iUnthrownGrenCount = 0;
	//m_bLastPrimed = false; // see CFFPlayer::SharedPreThink()
#else
	m_bEngyGrenWarned = false;
#endif
}

void CFFPlayer::ThrowPrimedGrenade( void )
{
#ifdef CLIENT_DLL
	if (!engine->IsConnected() || !engine->IsInGame())
		return;
#endif

	if ( !IsGrenadePrimed() )
		return;

	// Jiggles: Hint Code
	// Let's see if the player is throwing an "unprimed" grenade
	if ( ( gpGlobals->curtime - m_flPrimeTime ) <= GREN_THROW_DELAY)
	{
#ifdef CLIENT_DLL
		m_iUnprimedGrenCount++;
		// Event: 2 consecutive unprimed grenades thrown
		if (m_iUnprimedGrenCount > 1)
		{
			FF_SendHint(GLOBAL_NOPRIME1, 4, PRIORITY_NORMAL, "#FF_HINT_GLOBAL_NOPRIME1");
			m_iUnprimedGrenCount = 0;
		}
#endif
		m_bWantToThrowGrenade = true;
		return;
	}
	else  // Not an "unprimed" grenade -- the time between priming and throwing was > 0.5 seconds
	{
#ifdef CLIENT_DLL
		m_iUnprimedGrenCount = 0;
#endif
	}

#ifdef GAME_DLL
	bool bThrowGrenade = true;
	float fPrimeTimer = GREN_TIMER - (gpGlobals->curtime - m_flPrimeTime);

	// Give lua the chance to override grenade throwing.
	// It should return false to avoid throwing the grenade
	CFFLuaSC hContext(1, this);
	hContext.Push(1.0f - (fPrimeTimer / GREN_TIMER));

	const char* pLuaFn = 0;

	if (m_iGrenadeState == FF_GREN_PRIMEONE)
		pLuaFn = "player_onthrowgren1";
	else if (m_iGrenadeState == FF_GREN_PRIMETWO)
		pLuaFn = "player_onthrowgren2";

	if (pLuaFn && _scriptman.RunPredicates_LUA(NULL, &hContext, pLuaFn))
		bThrowGrenade = hContext.GetBool();

	if (bThrowGrenade)
		ThrowGrenade(fPrimeTimer);
#endif

	ResetGrenadeState();
}