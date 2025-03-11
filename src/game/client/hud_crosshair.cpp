//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "view.h"
#include "vgui_controls/Controls.h"
#include "vgui/ISurface.h"
#include "ivrenderview.h"
#include "materialsystem/imaterialsystem.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "client_virtualreality.h"
#include "sourcevr/isourcevirtualreality.h"

#include "ff_weapon_base.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_weapon_assaultcannon.h"
#include "ff_weapon_sniperrifle.h"
#include "ff_weapon_jumpgun.h"

#ifdef SIXENSE
#include "sixense/in_sixense.h"
#endif

#ifdef PORTAL
#include "c_portal_player.h"
#endif // PORTAL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar crosshair("crosshair", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
ConVar cl_observercrosshair("cl_observercrosshair", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
ConVar cl_acchargebar("cl_acchargebar", "0", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);
ConVar cl_pyro_fuelbar("cl_pyro_fuelbar", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL);

//Tie crosshair values to cheats -GreenMushy
ConVar cl_concaim("cl_concaim", "1", FCVAR_ARCHIVE | FCVAR_CLIENTDLL, "0 = always show crosshair in center. 1 = flash trueaim after shooting. 2 = hide crosshair when conced.");
ConVar cl_concaim_fadetime("cl_concaim_fadetime", "0.25", FCVAR_ARCHIVE | FCVAR_CLIENTDLL | FCVAR_CHEAT, "When cl_concaim is 1, controls the time the crosshair stays visible after shooting. Requires sv_cheats 1");
ConVar cl_concaim_showtrueaim("cl_concaim_showtrueaim", "0", FCVAR_CLIENTDLL | FCVAR_CHEAT, "Good way to learn how to concaim. If set to 1, when conced, the crosshair will show exactly where you will shoot. Requires sv_cheats 1");

#define FFDEV_CONCAIM cl_concaim.GetInt()
#define FFDEV_CONCAIM_FADETIME cl_concaim_fadetime.GetFloat()
#define FFDEV_CONCAIM_SHOWTRUEAIM cl_concaim_showtrueaim.GetBool()

using namespace vgui;

int ScreenTransform(const Vector& point, Vector& screen);

#ifdef TF_CLIENT_DLL
// If running TF, we use CHudTFCrosshair instead (which is derived from CHudCrosshair)
#else
DECLARE_HUDELEMENT(CHudCrosshair);
#endif

CHudCrosshair::CHudCrosshair(const char* pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudCrosshair")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_pCrosshair = 0;

	m_clrCrosshair = Color(0, 0, 0, 0);

	m_vecCrossHairOffsetAngle.Init();

	SetHiddenBits(/*HIDEHUD_PLAYERDEAD | */HIDEHUD_CROSSHAIR);
}

CHudCrosshair::~CHudCrosshair()
{
}

void CHudCrosshair::ApplySchemeSettings(IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	m_pDefaultCrosshair = gHUD.GetIcon("crosshair_default");
	SetPaintBackgroundEnabled(false);

	// --> Mirv
	vgui::HScheme CrossHairScheme = vgui::scheme()->LoadSchemeFromFile("resource/CrosshairScheme.res", "CrosshairScheme");

	for (int i = 0; i < CROSSHAIR_SIZES; i++)
	{
		m_hPrimaryCrosshairs[i] = vgui::scheme()->GetIScheme(CrossHairScheme)->GetFont(VarArgs("PrimaryCrosshairs%d", (i + 1)));
		m_hSecondaryCrosshairs[i] = vgui::scheme()->GetIScheme(CrossHairScheme)->GetFont(VarArgs("SecondaryCrosshairs%d", (i + 1)));
	}
	// <-- Mirv

	SetSize(ScreenWidth(), ScreenHeight());

	SetForceStereoRenderToFrameBuffer(true);
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHudCrosshair::ShouldDraw(void)
{
	bool bNeedsDraw;

	if (m_bHideCrosshair)
		return false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon && !pWeapon->ShouldDrawCrosshair())
		return false;

#ifdef PORTAL
	C_Portal_Player* portalPlayer = ToPortalPlayer(pPlayer);
	if (portalPlayer && portalPlayer->IsSuppressingCrosshair())
		return false;
#endif // PORTAL

	/* disabled to avoid assuming it's an HL2 player.
	// suppress crosshair in zoom.
	if ( pPlayer->m_HL2Local.m_bZooming )
		return false;
	*/

	// draw a crosshair only if alive or spectating in eye
	if (IsX360())
	{
		bNeedsDraw = m_pCrosshair &&
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() &&
			(!pPlayer->IsSuitEquipped() || g_pGameRules->IsMultiplayer()) &&
			g_pClientMode->ShouldDrawCrosshair() &&
			!(pPlayer->GetFlags() & FL_FROZEN) &&
			(pPlayer->entindex() == render->GetViewEntity()) &&
			(pPlayer->IsAlive() || (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE) || (cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING));
	}
	else
	{
		bNeedsDraw = m_pCrosshair &&
			crosshair.GetInt() &&
			!engine->IsDrawingLoadingImage() &&
			!engine->IsPaused() &&
			g_pClientMode->ShouldDrawCrosshair() &&
			!(pPlayer->GetFlags() & FL_FROZEN) &&
			(pPlayer->entindex() == render->GetViewEntity()) &&
			!pPlayer->IsInVGuiInputMode() &&
			(pPlayer->IsAlive() || (pPlayer->GetObserverMode() == OBS_MODE_IN_EYE) || (cl_observercrosshair.GetBool() && pPlayer->GetObserverMode() == OBS_MODE_ROAMING));
	}

	return (bNeedsDraw && CHudElement::ShouldDraw());
}

extern void GetCrosshair(FFWeaponID iWeapon, char& innerChar, Color& innerCol, int& innerSize, char& outerChar, Color& outerCol, int& outerSize);	// |-- Mirv

#ifdef TF_CLIENT_DLL
extern ConVar cl_crosshair_red;
extern ConVar cl_crosshair_green;
extern ConVar cl_crosshair_blue;
extern ConVar cl_crosshair_scale;
#endif


void CHudCrosshair::GetDrawPosition(float* pX, float* pY, bool* pbBehindCamera, QAngle angleCrosshairOffset)
{
	QAngle curViewAngles = CurrentViewAngles();
	Vector curViewOrigin = CurrentViewOrigin();

	int vx, vy, vw, vh;
	vgui::surface()->GetFullscreenViewport(vx, vy, vw, vh);

	float screenWidth = vw;
	float screenHeight = vh;

	float x = screenWidth / 2;
	float y = screenHeight / 2;

	bool bBehindCamera = false;

	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if ((pPlayer != NULL) && (pPlayer->GetObserverMode() == OBS_MODE_NONE))
	{
		bool bUseOffset = false;

		Vector vecStart;
		Vector vecEnd;

		if (UseVR())
		{
			// These are the correct values to use, but they lag the high-speed view data...
			vecStart = pPlayer->Weapon_ShootPosition();
			Vector vecAimDirection = pPlayer->GetAutoaimVector(1.0f);
			// ...so in some aim modes, they get zapped by something completely up-to-date.
			g_ClientVirtualReality.OverrideWeaponHudAimVectors(&vecStart, &vecAimDirection);
			vecEnd = vecStart + vecAimDirection * MAX_TRACE_LENGTH;

			bUseOffset = true;
		}

#ifdef SIXENSE
		// TODO: actually test this Sixsense code interaction with things like HMDs & stereo.
		if (g_pSixenseInput->IsEnabled() && !UseVR())
		{
			// Never autoaim a predicted weapon (for now)
			vecStart = pPlayer->Weapon_ShootPosition();
			Vector aimVector;
			AngleVectors(CurrentViewAngles() - g_pSixenseInput->GetViewAngleOffset(), &aimVector);
			// calculate where the bullet would go so we can draw the cross appropriately
			vecEnd = vecStart + aimVector * MAX_TRACE_LENGTH;
			bUseOffset = true;
		}
#endif

		if (bUseOffset)
		{
			trace_t tr;
			UTIL_TraceLine(vecStart, vecEnd, MASK_SHOT, pPlayer, COLLISION_GROUP_NONE, &tr);

			Vector screen;
			screen.Init();
			bBehindCamera = ScreenTransform(tr.endpos, screen) != 0;

			x = 0.5f * (1.0f + screen[0]) * screenWidth + 0.5f;
			y = 0.5f * (1.0f - screen[1]) * screenHeight + 0.5f;
		}
	}

	// MattB - angleCrosshairOffset is the autoaim angle.
	// if we're not using autoaim, just draw in the middle of the 
	// screen
	if (angleCrosshairOffset != vec3_angle)
	{
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		angles = curViewAngles + angleCrosshairOffset;
		AngleVectors(angles, &forward);
		VectorAdd(curViewOrigin, forward, point);
		ScreenTransform(point, screen);

		x += 0.5f * screen[0] * screenWidth + 0.5f;
		y += 0.5f * screen[1] * screenHeight + 0.5f;
	}

	*pX = x;
	*pY = y;
	*pbBehindCamera = bBehindCamera;
}


void CHudCrosshair::Paint(void)
{
	if (!m_pCrosshair)
		return;

	if (!IsCurrentViewAccessAllowed())
		return;

	C_FFPlayer* pActivePlayer = C_FFPlayer::GetLocalFFPlayerOrObserverTarget();

	if (!pActivePlayer)
		return;

	m_curViewAngles = CurrentViewAngles();
	m_curViewOrigin = CurrentViewOrigin();

	float x, y;
	x = ScreenWidth() / 2;
	y = ScreenHeight() / 2;

	float x_chargebar = x, y_chargebar = y;

	// MattB - m_vecCrossHairOffsetAngle is the autoaim angle.
	// if we're not using autoaim, just draw in the middle of the 
	// screen
	if (m_vecCrossHairOffsetAngle != vec3_angle)
	{
		Assert(0);	// |-- Mirv
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		angles = m_curViewAngles + m_vecCrossHairOffsetAngle;
		AngleVectors(angles, &forward);
		VectorAdd(m_curViewOrigin, forward, point);
		ScreenTransform(point, screen);

		x += 0.5f * screen[0] * ScreenWidth() + 0.5f;
		y += 0.5f * screen[1] * ScreenHeight() + 0.5f;
		x_chargebar += 0.5f * screen[0] * ScreenWidth() + 0.5f;
		y_chargebar += 0.5f * screen[1] * ScreenHeight() + 0.5f;
	}

	// AfterShock: Conc aim -> plot crosshair properly
	if ((FFDEV_CONCAIM_SHOWTRUEAIM) && ((pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0)))
	{
		QAngle angles;
		Vector forward;
		Vector point, screen;

		// this code is wrong
		// AfterShock: No, the code is now right!
		angles = pActivePlayer->EyeAngles();
		AngleVectors(angles, &forward);
		forward *= 10000.0f;
		VectorAdd(m_curViewOrigin, forward, point);
		ScreenTransform(point, screen);

		x = (screen[0] * 0.5 + 0.5f) * ScreenWidth();
		y = (1 - (screen[1] * 0.5 + 0.5f)) * ScreenHeight();
		x_chargebar = x;
		y_chargebar = y;
	}
	// hide crosshair
	else if ((FFDEV_CONCAIM == 2) && ((pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0)))
	{
		x = -1;
		y = -1;
	}
	// flash crosshair
	else if ((FFDEV_CONCAIM == 1) && ((pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0)))
	{
		// if should be flashing
		if (gpGlobals->curtime < pActivePlayer->m_flTrueAimTime + FFDEV_CONCAIM_FADETIME)
		{
			QAngle angles;
			Vector forward;
			Vector point, screen;

			// this code is wrong
			// AfterShock: No, the code is now right!
			angles = pActivePlayer->EyeAngles();
			AngleVectors(angles, &forward);
			forward *= 10000.0f;
			VectorAdd(m_curViewOrigin, forward, point);
			ScreenTransform(point, screen);

			x = (screen[0] * 0.5 + 0.5f) * ScreenWidth();
			y = (1 - (screen[1] * 0.5 + 0.5f)) * ScreenHeight();
		}
		// else don't draw xhair at all
		else
		{
			x = -1;
			y = -1;
		}
	}

	// --> Mirv: Crosshair stuff
	//m_pCrosshair->DrawSelf( 
	//		x - 0.5f * m_pCrosshair->Width(), 
	//		y - 0.5f * m_pCrosshair->Height(),
	//		m_clrCrosshair );

	C_FFWeaponBase* pWeapon = pActivePlayer->GetActiveFFWeapon();

	// No crosshair for no weapon
	if (!pWeapon)
		return;

	FFWeaponID weaponID = pWeapon->GetWeaponID();

	// Weapons other than these don't get crosshairs
	if (weaponID <= FF_WEAPON_NONE || weaponID > FF_WEAPON_TOMMYGUN)
		return;

	Color innerCol, outerCol;
	char innerChar, outerChar;
	int innerSize, outerSize;
	wchar_t unicode[2];

	//
	// TODO: Clean this up!!!!
	//

	HFont currentFont;
	GetCrosshair(weaponID, innerChar, innerCol, innerSize, outerChar, outerCol, outerSize);

	// concaim 1 = flash xhair when shooting
	if ((FFDEV_CONCAIM == 1) && ((pActivePlayer->m_flConcTime > gpGlobals->curtime) || (pActivePlayer->m_flConcTime < 0)))
	{
		//Get the weapon and see if you should draw the crosshair while conced
		if (weaponID == FF_WEAPON_ASSAULTCANNON ||
			weaponID == FF_WEAPON_SUPERNAILGUN ||
			weaponID == FF_WEAPON_FLAMETHROWER ||
			weaponID == FF_WEAPON_NAILGUN ||
			weaponID == FF_WEAPON_AUTORIFLE)
		{
			//If it was one of these weapons, just return before it tries to draw anything
			return;
		}

		// calculate alphas
		float flFlashAlpha = clamp(1.0f - (gpGlobals->curtime - pActivePlayer->m_flTrueAimTime) / FFDEV_CONCAIM_FADETIME, 0.0f, 1.0f);
		// set alphas
		outerCol[3] *= flFlashAlpha;
		innerCol[3] *= flFlashAlpha;
	}

	currentFont = m_hSecondaryCrosshairs[clamp(outerSize, 1, CROSSHAIR_SIZES) - 1];

	surface()->DrawSetTextColor(outerCol.r(), outerCol.g(), outerCol.b(), outerCol.a());
	surface()->DrawSetTextFont(currentFont);

	int charOffsetX = surface()->GetCharacterWidth(currentFont, outerChar) / 2;
	int charOffsetY = surface()->GetFontTall(currentFont) / 2;

	V_snwprintf(unicode, ARRAYSIZE(unicode), L"%c", outerChar);

	surface()->DrawSetTextPos(x - charOffsetX, y - charOffsetY);
	surface()->DrawUnicodeChar(unicode[0]);

	currentFont = m_hPrimaryCrosshairs[clamp(innerSize, 1, CROSSHAIR_SIZES) - 1];

	surface()->DrawSetTextColor(innerCol.r(), innerCol.g(), innerCol.b(), innerCol.a());
	surface()->DrawSetTextFont(currentFont);

	charOffsetX = surface()->GetCharacterWidth(currentFont, innerChar) / 2;
	charOffsetY = surface()->GetFontTall(currentFont) / 2;

	V_snwprintf(unicode, ARRAYSIZE(unicode), L"%c", innerChar);

	surface()->DrawSetTextPos(x - charOffsetX, y - charOffsetY);
	surface()->DrawUnicodeChar(unicode[0]);
	// <-- Mirv

	// Draw pyro fuel
	if (cl_pyro_fuelbar.GetBool() && pActivePlayer->GetClassSlot() == CLASS_PYRO && pActivePlayer->m_bCanUseJetpack)
	{
		float fuelPercent = pActivePlayer->m_iJetpackFuel / 200.0f;

		x = ScreenWidth() / 2;
		y = ScreenHeight() / 2;

		int iWidth = 32;
		int iHeight = 10;
		int iLeft = x - iWidth / 2;
		int iTop = y + charOffsetY + 20;
		int iRight = iLeft + (iWidth);
		int iBottom = iTop + iHeight;
		int iAlpha = 25 + (1.0f - fuelPercent * fuelPercent) * 200;
		float flRightInner = iLeft + ((float)(iRight - iLeft) * (fuelPercent));

		Color fuelBarColor = ColorFade(fuelPercent * 100, 0, 100, INTENSITYSCALE_COLOR_RED, INTENSITYSCALE_COLOR_GREEN);

		surface()->DrawSetColor(fuelBarColor.r(), fuelBarColor.g(), fuelBarColor.b(), iAlpha);
		surface()->DrawFilledRect(iLeft, iTop, flRightInner, iBottom);

		surface()->DrawSetColor(outerCol.r(), outerCol.g(), outerCol.b(), min(255, iAlpha + 50));
		surface()->DrawOutlinedRect(iLeft, iTop, iRight, iBottom);

		surface()->DrawLine(flRightInner, iTop, flRightInner, iBottom);
	}

	// Mulch: Draw charge bar!
	if ((weaponID == FF_WEAPON_ASSAULTCANNON) && (cl_acchargebar.GetBool()))
	{
		CFFWeaponAssaultCannon *pAC = (CFFWeaponAssaultCannon *) pWeapon;

		float flCharge =  pAC->m_flChargeTime / FF_AC_MAXCHARGETIME;
		flCharge = 100 * clamp(flCharge, 0.01f, 1.0f);

		if (flCharge <= 0.0f)
			return;

		int iLeft = x_chargebar - charOffsetX;
		int iTop = y_chargebar + charOffsetY;
		int iRight = iLeft + (charOffsetX * 2);
		int iBottom = iTop + 10;

		surface()->DrawSetColor(innerCol.r(), innerCol.g(), innerCol.b(), 150);
		surface()->DrawFilledRect(iLeft, iTop, iLeft + ((float)(iRight - iLeft) * (flCharge / 100.0f)), iBottom);

		surface()->DrawSetColor(outerCol.r(), outerCol.g(), outerCol.b(), 200);
		surface()->DrawOutlinedRect(iLeft, iTop, iRight, iBottom);
	}
	else if (weaponID == FF_WEAPON_SNIPERRIFLE)
	{
		CFFWeaponSniperRifle *pSniperRifle = (CFFWeaponSniperRifle *)pWeapon;

		if ( !pSniperRifle->IsInFire() )
			return;

		float flCharge = clamp( gpGlobals->curtime - pSniperRifle->GetFireStartTime(), 1.0f, FF_SNIPER_MAXCHARGE );
		flCharge = 100.0f * ( flCharge / FF_SNIPER_MAXCHARGE );

		if (flCharge <= 1.0f)
			return;

		int iLeft = x_chargebar - charOffsetX;
		int iTop = y_chargebar + charOffsetY;
		int iRight = iLeft + (charOffsetX * 2);
		int iBottom = iTop + 10;

		surface()->DrawSetColor(innerCol.r(), innerCol.g(), innerCol.b(), 150);
		surface()->DrawFilledRect(iLeft, iTop, iLeft + ((float)(iRight - iLeft) * (flCharge / 100.0f)), iBottom);

		surface()->DrawSetColor(outerCol.r(), outerCol.g(), outerCol.b(), 200);
		surface()->DrawOutlinedRect(iLeft, iTop, iRight, iBottom);
	}
	else if (weaponID == FF_WEAPON_JUMPGUN)
	{
		CFFWeaponJumpgun *pJump = (CFFWeaponJumpgun *) pWeapon;

		float flCharge = pJump->GetClampedCharge() / JUMPGUN_CHARGEUPTIME;
		flCharge = clamp(flCharge, 0.01f, 1.0f);

		if (flCharge <= 0.0f)
			return;

		x = ScreenWidth() / 2;
		y = ScreenHeight() / 2;

		int iLeft = x - charOffsetX;
		int iTop = y + charOffsetY;
		int iRight = iLeft + (charOffsetX * 2);
		int iBottom = iTop + 10;

		if (flCharge == 1.0f)
		{
			surface()->DrawSetColor(128, 255, 64, 150);
		}
		else
		{
			surface()->DrawSetColor(innerCol.r(), innerCol.g(), innerCol.b(), 150);
		}
		surface()->DrawFilledRect(iLeft, iTop, iLeft + ((float)(iRight - iLeft) * (flCharge)), iBottom);

		surface()->DrawSetColor(outerCol.r(), outerCol.g(), outerCol.b(), 200);
		surface()->DrawOutlinedRect(iLeft, iTop, iRight, iBottom);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshairAngle(const QAngle& angle)
{
	VectorCopy(angle, m_vecCrossHairOffsetAngle);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudCrosshair::SetCrosshair(CHudTexture* texture, const Color& clr)
{
	m_pCrosshair = texture;
	m_clrCrosshair = clr;
}

//-----------------------------------------------------------------------------
// Purpose: Resets the crosshair back to the default
//-----------------------------------------------------------------------------
void CHudCrosshair::ResetCrosshair()
{
	SetCrosshair(m_pDefaultCrosshair, Color(255, 255, 255, 255));
}
