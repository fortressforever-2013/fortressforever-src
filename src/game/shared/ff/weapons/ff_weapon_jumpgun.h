#ifndef FF_WEAPON_JUMPGUN_H
#define FF_WEAPON_JUMPGUN_H

#ifdef CLIENT_DLL
	#define CFFWeaponJumpgun C_FFWeaponJumpgun
	#define JUMPGUN_CHARGETIMEBUFFERED_UPDATEINTERVAL 0.02f
#elif GAME_DLL

#endif

//ConVar ffdev_jumpgun_chargeuptime("ffdev_jumpgun_chargeuptime", "6", FCVAR_REPLICATED | FCVAR_CHEAT);
#define JUMPGUN_CHARGEUPTIME 6 //ffdev_jumpgun_chargeuptime.GetFloat()

//=============================================================================
// CFFWeaponJumpgun
//=============================================================================

class CFFWeaponJumpgun : public CFFWeaponBase
{
public:
	DECLARE_CLASS( CFFWeaponJumpgun, CFFWeaponBase );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	
	CFFWeaponJumpgun( void );

	virtual void	PrimaryAttack() {}

	virtual bool	Deploy( void );
	virtual bool	Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void	Precache( void );
	virtual void	Fire( void );
	virtual void	ItemPostFrame( void );
	virtual void	UpdateOnRemove( void );

	virtual FFWeaponID GetWeaponID( void ) const { return FF_WEAPON_JUMPGUN; }

	float	GetClampedCharge( void );

	int m_nRevSound;
	int m_iShockwaveTexture;

#ifdef GAME_DLL

	void JumpgunEmitSound( const char* szSoundName );

	bool m_bPlayRevSound;
	float m_flRevSoundNextUpdate;
	EmitSound_t m_paramsRevSound;

	float m_flStartTime;
	float m_flLastUpdate;

#else

	virtual void	ViewModelDrawn( C_BaseViewModel *pBaseViewModel );

private:
	int	m_iAttachment1;
	int m_iAttachment2;
	float m_flTotalChargeTimeBuffered;
	float m_flClampedChargeTimeBuffered;
	float m_flChargeTimeBufferedNextUpdate;

#endif	

private:
	CFFWeaponJumpgun( const CFFWeaponJumpgun & );
	CNetworkVar( float, m_flTotalChargeTime );
	CNetworkVar( float, m_flClampedChargeTime );
};

#endif // FF_WEAPON_JUMPGUN_H