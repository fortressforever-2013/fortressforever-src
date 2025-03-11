/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_railgun.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF railgun code & class declaration
///
/// REVISIONS
/// ---------
/// Jan 19 2004 Mirv: First implementation
//
//	11/11/2006 Mulchman: Reverting back to bouncy rail

#include "cbase.h"
#include "ff_weapon_base.h"
#include "ff_fx_shared.h"
#include "ff_projectile_rail.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponRailgun C_FFWeaponRailgun

	#include "soundenvelope.h"
	#include "c_ff_player.h"
	//#include "c_te_effect_dispatch.h"
	
	#include "beamdraw.h"

	extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );
	//extern void DrawHalo(IMaterial* pMaterial, const Vector &source, float scale, float const *color, float flHDRColorScale);
#else
	#include "omnibot_interface.h"
	#include "ff_player.h"
	#include "te_effect_dispatch.h"
#endif

extern unsigned char g_uchRailColors[3][3];

//ConVar ffdev_railgun_revsound_volume_high("ffdev_railgun_revsound_volume_high", "1.0", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound High Volume");
#define RAILGUN_REVSOUND_VOLUME_HIGH 1.0f // ffdev_railgun_revsound_volume_high.GetFloat()
//ConVar ffdev_railgun_revsound_volume_low("ffdev_railgun_revsound_volume_low", "0.0", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound Low Volume");
#define RAILGUN_REVSOUND_VOLUME_LOW 0.0f // ffdev_railgun_revsound_volume_low.GetFloat()
//ConVar ffdev_railgun_revsound_pitch_high("ffdev_railgun_revsound_pitch_high", "208", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound High Pitch");
#define RAILGUN_REVSOUND_PITCH_HIGH 208 // ffdev_railgun_revsound_pitch_high.GetInt()
//ConVar ffdev_railgun_revsound_pitch_low("ffdev_railgun_revsound_pitch_low", "64", FCVAR_FF_FFDEV_REPLICATED, "Railgun Rev Sound Low Pitch");
#define RAILGUN_REVSOUND_PITCH_LOW 64 // ffdev_railgun_revsound_pitch_low.GetInt()


//ConVar ffdev_railgun_revsound_updateinterval("ffdev_railgun_revsound_updateinterval", "0.01", FCVAR_FF_FFDEV_REPLICATED, "How much time to wait before updating");
#define RAILGUN_REVSOUND_UPDATEINTERVAL 0.01f // ffdev_railgun_revsound_updateinterval.GetFloat()

//ConVar ffdev_railgun_ammoamount_halfcharge( "ffdev_railgun_ammoamount_halfcharge", "2", FCVAR_FF_FFDEV_REPLICATED, "How much total ammo is lost for a half charge." );
#define RAILGUN_AMMOAMOUNT_HALFCHARGE 2 // ffdev_railgun_ammoamount_halfcharge.GetInt()
//ConVar ffdev_railgun_ammoamount_fullcharge( "ffdev_railgun_ammoamount_fullcharge", "3", FCVAR_FF_FFDEV_REPLICATED, "How much total ammo is lost for a full charge." );
#define RAILGUN_AMMOAMOUNT_FULLCHARGE 3 // ffdev_railgun_ammoamount_fullcharge.GetInt()

//ConVar ffdev_railgun_overchargetime( "ffdev_railgun_overchargetime", "6.0", FCVAR_FF_FFDEV_REPLICATED, "Railgun overcharges at this time, stops charging, and damages player." );
#define RAILGUN_OVERCHARGETIME 6.0f // ffdev_railgun_overchargetime.GetFloat()
//ConVar ffdev_railgun_overchargedamage( "ffdev_railgun_overchargedamage", "0.0", FCVAR_FF_FFDEV_REPLICATED, "Amount of damage an overcharge gives to the player (doubled on full charge)." );
#define RAILGUN_OVERCHARGEDAMAGE 0.0f // ffdev_railgun_overchargedamage.GetFloat()

//ConVar ffdev_railgun_cooldowntime_zerocharge( "ffdev_railgun_cooldowntime_zerocharge", "0.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after firing a non-charged shot." );
#define RAILGUN_COOLDOWNTIME_ZEROCHARGE 0.333f // ffdev_railgun_cooldowntime_zerocharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_halfcharge( "ffdev_railgun_cooldowntime_halfcharge", "0.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after firing a half-charged shot." );
#define RAILGUN_COOLDOWNTIME_HALFCHARGE 0.333f // ffdev_railgun_cooldowntime_halfcharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_fullcharge( "ffdev_railgun_cooldowntime_fullcharge", "0.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after firing a full-charged shot." );
#define RAILGUN_COOLDOWNTIME_FULLCHARGE 0.333f // ffdev_railgun_cooldowntime_fullcharge.GetFloat()
//ConVar ffdev_railgun_cooldowntime_overcharge( "ffdev_railgun_cooldowntime_overcharge", "2.333", FCVAR_FF_FFDEV_REPLICATED, "Cooldown time after overcharging." );
#define RAILGUN_COOLDOWNTIME_OVERCHARGE 2.333f // ffdev_railgun_cooldowntime_overcharge.GetFloat()

//ConVar ffdev_rail_speed_min( "ffdev_rail_speed_min", "1800.0", FCVAR_FF_FFDEV_REPLICATED, "Minimum speed of rail" );
#define RAIL_SPEED_MIN 1800.0f // ffdev_rail_speed_min.GetFloat()
//ConVar ffdev_rail_speed_max( "ffdev_rail_speed_max", "3000.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum speed of rail" );
#define RAIL_SPEED_MAX 3000.0f // ffdev_rail_speed_max.GetFloat()
//ConVar ffdev_rail_damage_min( "ffdev_rail_damage_min", "35.0", FCVAR_FF_FFDEV_REPLICATED, "Minimum damage dealt by rail" );
#define RAIL_DAMAGE_MIN 35.0f // ffdev_rail_damage_min.GetFloat()
//ConVar ffdev_rail_damage_max( "ffdev_rail_damage_max", "60.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum damage dealt by rail" );
#define RAIL_DAMAGE_MAX 60.0f // ffdev_rail_damage_max.GetFloat()

//ConVar ffdev_railgun_pushforce_min("ffdev_railgun_pushforce_min", "32.0", FCVAR_FF_FFDEV_REPLICATED, "Minimum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
#define RAILGUN_PUSHFORCE_MIN 32.0f // ffdev_railgun_pushforce_min.GetFloat()
//ConVar ffdev_railgun_pushforce_max("ffdev_railgun_pushforce_max", "64.0", FCVAR_FF_FFDEV_REPLICATED, "Maximum force of backwards push (Like the HL Gauss Gun, WOOH YEAH!)");
#define RAILGUN_PUSHFORCE_MAX 64.0f // ffdev_railgun_pushforce_max.GetFloat()

//ConVar ffdev_railgun_recoil_min("ffdev_railgun_recoil_min", "2", FCVAR_FF_FFDEV_REPLICATED, "Minimum recoil");
#define RAILGUN_RECOIL_MIN 2 // ffdev_railgun_recoil_min.GetInt()
//ConVar ffdev_railgun_recoil_max("ffdev_railgun_recoil_max", "10", FCVAR_FF_FFDEV_REPLICATED, "Minimum recoil");
#define RAILGUN_RECOIL_MAX 10 // ffdev_railgun_recoil_max.GetInt()

//ConVar ffdev_railgun_resupply_interval("ffdev_railgun_resupply_interval", "4.0", FCVAR_FF_FFDEV_REPLICATED, "Resupply every X seconds.");
#define RAILGUN_RESUPPLY_INTERVAL 4.0f // ffdev_railgun_resupply_interval.GetFloat()
//ConVar ffdev_railgun_resupply_rails("ffdev_railgun_resupply_rails", "1", FCVAR_FF_FFDEV_REPLICATED, "Resupply X rails");
#define RAILGUN_RESUPPLY_RAILS 1 // ffdev_railgun_resupply_rails.GetInt()
//ConVar ffdev_railgun_resupply_cells("ffdev_railgun_resupply_cells", "40", FCVAR_FF_FFDEV_REPLICATED, "Resupply X cells on overcharge");
#define RAILGUN_RESUPPLY_CELLS 40 // ffdev_railgun_resupply_cells.GetInt()

#ifdef GAME_DLL
#else

#define RAILGUN_CHARGETIMEBUFFERED_UPDATEINTERVAL 0.02f

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectStunstick )
CLIENTEFFECT_MATERIAL( "effects/stunstick" )
CLIENTEFFECT_REGISTER_END()

#endif

//=============================================================================
// CFFWeaponRailgun
//=============================================================================

class CFFWeaponRailgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponRailgun, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponRailgun( void );

	virtual void	PrimaryAttack() {}

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Precache( void );
	virtual void	Fire( void );
	virtual void	ItemPostFrame( void );
	virtual void	UpdateOnRemove( void );

	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_RAILGUN; }

	CNetworkVar(int, m_iAmmoUsed);
	CNetworkVar(float, m_flStartTime);

#ifdef GAME_DLL

	// transmit to all clients
	virtual int		UpdateTransmitState() { return SetTransmitState(FL_EDICT_ALWAYS); }
	float m_flNextResupply;

#else
	virtual void	OnDataChanged(DataUpdateType_t updateType);
	void			UpdateRevSound(void);
	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );
	virtual RenderGroup_t GetRenderGroup( void ) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual bool IsTranslucent( void )			 { return true; }

private:
	CSoundPatch *m_sndRev;
	int	m_iAttachment1;
	int m_iAttachment2;

#endif	

	CFFWeaponRailgun( const CFFWeaponRailgun & );
};

//=============================================================================
// CFFWeaponRailgun tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFWeaponRailgun, DT_FFWeaponRailgun )

BEGIN_NETWORK_TABLE(CFFWeaponRailgun, DT_FFWeaponRailgun )
#ifdef CLIENT_DLL
	RecvPropTime( RECVINFO( m_flStartTime ) ),
	RecvPropInt( RECVINFO(m_iAmmoUsed) ),
#else
	SendPropTime( SENDINFO( m_flStartTime ) ),
	SendPropInt( SENDINFO(m_iAmmoUsed), 3, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CFFWeaponRailgun )
	DEFINE_PRED_FIELD(m_flStartTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( ff_weapon_railgun, CFFWeaponRailgun );
PRECACHE_WEAPON_REGISTER( ff_weapon_railgun );

//=============================================================================
// CFFWeaponRailgun implementation
//=============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFWeaponRailgun::CFFWeaponRailgun( void )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;

#ifdef GAME_DLL

	m_flNextResupply = 0.0f;

#else

	m_sndRev = NULL;

	m_iAttachment1 = m_iAttachment2 = -1;

	m_colorMuzzleDLight.r = g_uchRailColors[0][0];
	m_colorMuzzleDLight.g = g_uchRailColors[0][1];
	m_colorMuzzleDLight.b = g_uchRailColors[0][2];

#endif
}

//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
void CFFWeaponRailgun::UpdateOnRemove( void )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;
	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

#ifdef CLIENT_DLL

	UpdateRevSound();

#endif

	BaseClass::UpdateOnRemove();
}

//----------------------------------------------------------------------------
// Purpose: Deploy
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Deploy( void )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;
	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

#ifdef GAME_DLL

	// resupply every X seconds with the railgun out
	m_flNextResupply = gpGlobals->curtime + RAILGUN_RESUPPLY_INTERVAL;

#else

	UpdateRevSound();

#endif

	return BaseClass::Deploy();
}

//----------------------------------------------------------------------------
// Purpose: Holster
//----------------------------------------------------------------------------
bool CFFWeaponRailgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_iAmmoUsed = 0;
	m_flStartTime = 0.0f;
	m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;

#ifdef CLIENT_DLL

	UpdateRevSound();

#endif

	return BaseClass::Holster( pSwitchingTo );
}

//----------------------------------------------------------------------------
// Purpose: Precache
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Precache( void )
{
	PrecacheScriptSound( "railgun.single_shot" );		// SINGLE
	PrecacheScriptSound( "railgun.charged_shot" );		// WPN_DOUBLE
	PrecacheScriptSound( "railgun.chargeloop" );		// SPECIAL1
	PrecacheScriptSound( "railgun.halfcharge" );		// SPECIAL2 - half charge notification
	PrecacheScriptSound( "railgun.fullcharge" );		// SPECIAL3 - full charge notification
	PrecacheScriptSound( "railgun.overcharge" );		// BURST - overcharge

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Fire a rail
//----------------------------------------------------------------------------
void CFFWeaponRailgun::Fire( void )
{
	CFFPlayer *pPlayer = GetPlayerOwner();

	if (!pPlayer)
		return;

	CANCEL_IF_BUILDING();
	CANCEL_IF_CLOAKED();

	pPlayer->m_flTrueAimTime = gpGlobals->curtime;

	Vector vecForward, vecRight, vecUp;
	pPlayer->EyeVectors( &vecForward, &vecRight, &vecUp);
	VectorNormalizeFast( vecForward );

	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	float flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, RAILGUN_MAXCHARGETIME);
	float flPercent = flClampedChargeTime / RAILGUN_MAXCHARGETIME;

	// Push them backwards
	pPlayer->ApplyAbsVelocityImpulse(vecForward * -(RAILGUN_PUSHFORCE_MIN + ( (RAILGUN_PUSHFORCE_MAX - RAILGUN_PUSHFORCE_MIN) * flPercent )));
	
	if (m_bMuzzleFlash)
		pPlayer->DoMuzzleFlash();

	if( IsPlayerUsingNonFallbackNewViewmodel(pPlayer) )
	{
		if ( gpGlobals->curtime - m_flStartTime >= RAILGUN_MAXCHARGETIME )
		{
			SendWeaponAnim( ACT_VM_SECONDARYATTACK );
		}
		else
		{
			SendWeaponAnim( ACT_VM_PRIMARYATTACK );
		}
	}
	else
	{
		SendWeaponAnim( GetPrimaryAttackActivity() );
	}

	//Player "shoot" animation
	pPlayer->DoAnimationEvent(PLAYERANIMEVENT_FIRE_GUN_PRIMARY);

	pPlayer->ViewPunch( -QAngle( RAILGUN_RECOIL_MIN + ((RAILGUN_RECOIL_MAX - RAILGUN_RECOIL_MIN) * flPercent), 0, 0 ) );

	// cycletime is based on charge level
	if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE || pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0)
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_ZEROCHARGE;
	else if (m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE)
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_HALFCHARGE;
	else
		m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_FULLCHARGE;

#ifdef GAME_DLL
	// Determine Speed of rail projectile by: railspeed = min + [ ( ( max - min ) * chargetime ) / maxchargetime ] 
	float flSpeed = RAIL_SPEED_MIN + ( (RAIL_SPEED_MAX - RAIL_SPEED_MIN) * flPercent );

	// Now determine damage the same way
	float flDamage = RAIL_DAMAGE_MIN + ( (RAIL_DAMAGE_MAX - RAIL_DAMAGE_MIN) * flPercent );

	const int iDamageRadius = 100;
	CFFProjectileRail *pRail = CFFProjectileRail::CreateRail( this, vecSrc, pPlayer->EyeAngles(), pPlayer, flDamage, iDamageRadius, flSpeed, flClampedChargeTime );	
	Omnibot::Notify_PlayerShoot(pPlayer, Omnibot::TF_WP_RAILGUN, pRail);
#endif

	// MUST call sound before removing a round from the clip of a CMachineGun
	if (gpGlobals->curtime - m_flStartTime >= RAILGUN_MAXCHARGETIME)
	{
		WeaponSound(WPN_DOUBLE);
	}
	else
	{
		WeaponSound(SINGLE);
	}

	m_flStartTime = 0.0f;

#ifdef CLIENT_DLL
	UpdateRevSound();
#endif

	if (pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_iAmmoUsed = 0;
}

//----------------------------------------------------------------------------
// Purpose: Handle all the chargeup stuff here
//----------------------------------------------------------------------------
void CFFWeaponRailgun::ItemPostFrame(void)
{
	CFFPlayer* pPlayer = ToFFPlayer(GetOwner());
	if (!pPlayer)
		return;

	if (m_flStartTime != 0.0f)
	{
		float flTimeSinceStart = gpGlobals->curtime - m_flStartTime;

		if (pPlayer->m_afButtonReleased & IN_ATTACK)
		{
			// Fire now
			Fire();
		}
		else if (flTimeSinceStart >= RAILGUN_OVERCHARGETIME)
		{
			m_flStartTime = 0.0f;
			m_iAmmoUsed = 0;

#ifdef CLIENT_DLL
			UpdateRevSound();
#else
			// remove one more rail on overcharge
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);

			// give cells on overcharge
			pPlayer->GiveAmmo(RAILGUN_RESUPPLY_CELLS, AMMO_CELLS, true);
#endif
			// play an overcharge sound
			WeaponSound(BURST);

			if( IsPlayerUsingNonFallbackNewViewmodel(pPlayer) )
			{
				SendWeaponAnim( ACT_VM_RELEASE );
			}

			m_flNextPrimaryAttack = gpGlobals->curtime + RAILGUN_COOLDOWNTIME_OVERCHARGE;
		}
		else if ((flTimeSinceStart >= RAILGUN_MAXCHARGETIME * 0.5f && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE) || (flTimeSinceStart >= RAILGUN_MAXCHARGETIME && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_FULLCHARGE))
		{
			int iClampedTime = clamp(flTimeSinceStart, 0, 2);

			// play a charge sound
			WeaponSound( WeaponSound_t(SPECIAL1 + iClampedTime) );
#ifdef GAME_DLL
			// remove additional ammo at each charge level
			pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
#endif
			// client needs to know, too
			m_iAmmoUsed++;
		}
	}
#ifdef GAME_DLL
	// time to resupply?
	if (m_flNextResupply <= gpGlobals->curtime)
	{
		// give ammo already, GOSH!
		pPlayer->GiveAmmo(RAILGUN_RESUPPLY_RAILS, m_iPrimaryAmmoType, true);

		// resupply every X seconds with the railgun out
		m_flNextResupply = gpGlobals->curtime + RAILGUN_RESUPPLY_INTERVAL;
	}
#endif

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pPlayer->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	if ((pPlayer->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		// allow players to continue to charge if they've hit the halfway mark
		// and don't make it immediately switch, causing shot sounds to stop
		if ( pPlayer->GetAmmoCount(GetPrimaryAmmoType()) <= 0 && m_iAmmoUsed < RAILGUN_AMMOAMOUNT_HALFCHARGE )
		{
			if ( m_flStartTime != 0.0f )
			{
				// Fire now
				Fire();
			}
			else if( m_flNextPrimaryAttack < gpGlobals->curtime )
			{
				// HEV suit - indicate out of ammo condition
				HandleFireOnEmpty();

				pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
			}
		}
		else
		{
			if ((pPlayer->m_afButtonPressed & IN_ATTACK) || (pPlayer->m_afButtonReleased & IN_ATTACK2))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			if (m_flStartTime <= 0)
			{
				// save that we had the attack button down
				m_flStartTime = gpGlobals->curtime;

#ifdef CLIENT_DLL
				UpdateRevSound();
#else
				// remove ammo immediately
				pPlayer->RemoveAmmo(1, m_iPrimaryAmmoType);
#endif

				if( IsPlayerUsingNonFallbackNewViewmodel(pPlayer) )
				{
					SendWeaponAnim( ACT_VM_PULLBACK );
				}

				// client needs to know, too
				m_iAmmoUsed++;
			}
		}
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (! ((pPlayer->m_nButtons & IN_ATTACK) || /* (pOwner->m_nButtons & IN_ATTACK2) ||*/ (pPlayer->m_nButtons & IN_RELOAD))) // |-- Mirv: Removed attack2 so things can continue while in menu
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}


#ifdef CLIENT_DLL
void CFFWeaponRailgun::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	UpdateRevSound();
}


void CFFWeaponRailgun::UpdateRevSound( void )
{
	CFFPlayer *pPlayer = ToFFPlayer( GetOwner() );
	if ( !pPlayer )
		return;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();

	if ( m_flStartTime != 0.0f )
	{
		if ( !m_sndRev )
		{
			CLocalPlayerFilter filter;
			m_sndRev = controller.SoundCreate( filter, pPlayer->entindex(), GetShootSound( SPECIAL1 ) );
			controller.Play( m_sndRev, RAILGUN_REVSOUND_VOLUME_LOW, RAILGUN_REVSOUND_PITCH_LOW );
			controller.SoundChangePitch( m_sndRev, RAILGUN_REVSOUND_PITCH_HIGH, RAILGUN_MAXCHARGETIME );
			controller.SoundChangeVolume( m_sndRev, RAILGUN_REVSOUND_VOLUME_HIGH, RAILGUN_MAXCHARGETIME);
		}
	}
	else
	{
		if ( m_sndRev )
		{
			controller.SoundDestroy( m_sndRev );
			m_sndRev = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: This takes place after the viewmodel is drawn. We use this to
//			create the glowing glob of stuff inside the railgun and the faint
//			glow at the barrel.
//-----------------------------------------------------------------------------
void CFFWeaponRailgun::ViewModelDrawn( C_BaseViewModel *pBaseViewModel )
{
	// Not charging at all or even much, so no need to draw shit
	if (m_flStartTime <= 0.0)
		return;

	float flClampedChargeTime = clamp(gpGlobals->curtime - m_flStartTime, 0, RAILGUN_MAXCHARGETIME);

	// We'll get these done and out of the way
	if (m_iAttachment1 == -1 || m_iAttachment2 == -1)
	{
		m_iAttachment1 = pBaseViewModel->LookupAttachment("railgunFX1");
		m_iAttachment2 = pBaseViewModel->LookupAttachment("railgunFX2");
	}

	Vector vecStart, vecEnd, vecMuzzle;
	QAngle tmpAngle;

	pBaseViewModel->GetAttachment(m_iAttachment1, vecStart, tmpAngle);
	pBaseViewModel->GetAttachment(m_iAttachment2, vecEnd, tmpAngle);
	pBaseViewModel->GetAttachment(1, vecMuzzle, tmpAngle);

	::FormatViewModelAttachment(vecStart, true);
	::FormatViewModelAttachment(vecEnd, true);
	::FormatViewModelAttachment(vecMuzzle, true);

	float flPercent = clamp(flClampedChargeTime / RAILGUN_MAXCHARGETIME, 0.0f, 1.0f);
	flPercent = sqrtf( flPercent );
	float effectScale = flPercent == 1.0f ? 5.0f : 2.0f;

	// Haha, clean this up pronto!
	Vector vecControl = (vecStart + vecEnd) * 0.5f + Vector(random->RandomFloat(-flPercent, flPercent), random->RandomFloat(-flPercent, flPercent), random->RandomFloat(-flPercent, flPercent));

	float flScrollOffset = gpGlobals->curtime - (int) gpGlobals->curtime;

	CMatRenderContextPtr pMatRenderContext(g_pMaterialSystem);

	IMaterial *pMat = materials->FindMaterial("sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS);
	pMatRenderContext->Bind(pMat);

	DrawBeamQuadratic(vecStart, vecControl, vecEnd, effectScale * flPercent, Vector(0.51f, 0.89f, 0.95f), flScrollOffset);

	float colour[3] = { 0.51f, 0.89f, 0.95f };

	pMat = materials->FindMaterial("effects/stunstick", TEXTURE_GROUP_CLIENT_EFFECTS);
	pMatRenderContext->Bind(pMat);
	
	DrawHalo(pMat, vecMuzzle, 0.58f * effectScale, colour);
}
#endif