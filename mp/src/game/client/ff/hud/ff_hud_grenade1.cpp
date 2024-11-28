#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"

#include "iclientmode.h" //for animation stuff
#include "c_ff_player.h" //for gettuing ff player
#include "ff_playerclass_parse.h" //for parseing ff player txts
#include "ff_grenade_parse.h" //for parseing ff gren txts

#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define GREN1_BACKGROUND_TEXTURE "hud/Gren1BG"
#define GREN1_FOREGROUND_TEXTURE "hud/Gren1FG"

extern Color GetCustomClientColor(int iPlayerIndex, int iTeamIndex/* = -1*/);

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudGrenade1 : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudGrenade1, CHudNumericDisplay);

public:
	CHudGrenade1(const char *pElementName);
	~CHudGrenade1( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Reset( void );
	virtual void Paint( void );
	virtual void ApplySchemeSettings( IScheme* pScheme );
	virtual bool ShouldDraw( void );
			void SetGrenade( int Grenade, bool playAnimation );
			void CacheTextures( void );

private:
	int		m_iGrenade;
	int		m_iClass;

	CHudTexture *m_pIconTexture;

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};

DECLARE_HUDELEMENT(CHudGrenade1);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenade1::CHudGrenade1(const char *pElementName) : BaseClass(NULL, "HudGrenade1"), CHudElement(pElementName) 
{
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudGrenade1::~CHudGrenade1()
{
	if (m_pBGTexture)
	{
		delete m_pBGTexture;
		m_pBGTexture = NULL;
	}

	if (m_pFGTexture)
	{
		delete m_pFGTexture;
		m_pFGTexture = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade1::Init() 
{
	m_iGrenade		= -1;
	m_iClass		= 0;

	SetLabelText(L"");
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade1::VidInit()
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade1::ApplySchemeSettings(IScheme* pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// blame CHudNumericDisplay
	Panel::SetFgColor( GetSchemeColor( "HUD_Tone_Default", Color( 199, 219, 255, 215 ), pScheme ) );
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudGrenade1::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(GREN1_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(GREN1_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudGrenade1::Reset() 
{
	BaseClass::Reset();

	m_iGrenade = -1;
	m_iClass = -1;
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
bool CHudGrenade1::ShouldDraw()
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if (!pPlayer)
		return false;

	int iClass = pPlayer->GetClassSlot();
	int iGrenade1 = pPlayer->m_iPrimary;

	//if no class or class doesn't have grenades
	if(iClass == 0 || iGrenade1 == -1)
	{
		m_iClass = iClass;
		return false;
	}
	else if(m_iClass != iClass)
	{
		m_iClass = iClass;

		const char *szClassNames[] = { 
			"scout", "sniper", "soldier", 
			"demoman", "medic", "hwguy", 
			"pyro", "spy", "engineer", 
			"civilian" 
		};

		PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
		bool bReadInfo = ReadPlayerClassDataFromFileForSlot( filesystem, szClassNames[m_iClass - 1], &hClassInfo, g_pGameRules->GetEncryptionKey() );

		if (!bReadInfo)
			return false;

		const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

		if (!pClassInfo)
			return false;

		SetGrenade(iGrenade1, false);
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasGrenades");

		if ( strcmp( pClassInfo->m_szPrimaryClassName, "None" ) != 0 )
		{
			const char *grenade_name = pClassInfo->m_szPrimaryClassName;

			GRENADE_FILE_INFO_HANDLE hGrenInfo = LookupGrenadeInfoSlot(grenade_name);
			if (!hGrenInfo)
				return false;

			CFFGrenadeInfo *pGrenInfo = GetFileGrenadeInfoFromHandle(hGrenInfo);
			if (!pGrenInfo)
				return false;

			m_pIconTexture = pGrenInfo->iconHud;
		}
		else // Player doesn't have grenades
			return false;
	}
	else
	{
		// Same class, just update counts
		SetGrenade(iGrenade1, true);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Updates Grenade display
//-----------------------------------------------------------------------------
void CHudGrenade1::SetGrenade(int iGrenade, bool playAnimation) 
{
	if (iGrenade != m_iGrenade) 
	{
		if (iGrenade == 0) 
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeEmpty");
		}
		else if (iGrenade < m_iGrenade) 
		{
			// Grenade has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeIncreased");
		}
	}
	m_iGrenade = iGrenade;

	SetDisplayValue(m_iGrenade);
}

void CHudGrenade1::Paint() 
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	Color cColor = GetCustomClientColor( -1, pPlayer->GetTeamNumber() );
	cColor.setA( 150 );

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, GREN1_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, GREN1_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	if(m_pIconTexture)
	{
		Color iconColor( 255, 255, 255, 125 );
		int iconWide = m_pIconTexture->Width();
		int iconTall = m_pIconTexture->Height();

		//If we're using a font char, this will ignore iconTall and iconWide
		m_pIconTexture->DrawSelf( icon_xpos, icon_ypos - (iconTall / 2), iconWide, iconTall, iconColor );
	}

	BaseClass::Paint();
}