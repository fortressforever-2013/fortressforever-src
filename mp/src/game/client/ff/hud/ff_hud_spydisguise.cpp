//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_spydisguise.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 09/01/2006
//	@brief Hud Disguise Indicator
//
//	REVISIONS
//	---------
//	09/01/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

//#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "c_ff_player.h"
#include "ff_utils.h"
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#define SPY_DISGUISE_TIME 3.5f

#define SPY_DISGUISE_BOX_BACKGROUND_TEXTURE "hud/SpyDisguiseBoxBG"
#define SPY_DISGUISE_BOX_FOREGROUND_TEXTURE "hud/SpyDisguiseBoxFG"

#define SPY_DISGUISE_PROGRESS_BAR_BACKGROUND_TEXTURE "hud/SpyDisguiseProgressBarBG"
#define SPY_DISGUISE_PROGRESS_BAR_FOREGROUND_TEXTURE "hud/SpyDisguiseProgressBarFG"

inline void MapClassToGlyph( int iClass, char& cGlyph )
{
	/* Straight from the horses mouth
	!  *	Scout
	@  *	Sniper
	#  *	Soldier
	$  *	Demoman
	%  *	Medic
	^  *	HWGuy
	?  *	Pyro
	*  *	Spy
	(  *	Engineer
	)  *	Civilian
	_  *	Random
	*/
	switch( iClass )
	{
		case CLASS_SCOUT: cGlyph = '!'; break;
		case CLASS_SNIPER: cGlyph = '@'; break;
		case CLASS_SOLDIER: cGlyph = '#'; break;
		case CLASS_DEMOMAN: cGlyph = '$'; break;
		case CLASS_MEDIC: cGlyph = '%'; break;
		case CLASS_HWGUY: cGlyph = '^'; break;
		case CLASS_PYRO: cGlyph = '?'; break;
		case CLASS_SPY: cGlyph = '*'; break;
		case CLASS_ENGINEER: cGlyph = '('; break;
		case CLASS_CIVILIAN: cGlyph = ')'; break;
		default: cGlyph = '_'; break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudSpyDisguise : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudSpyDisguise, vgui::Panel );

	CHudSpyDisguise( const char *pElementName );
	virtual ~CHudSpyDisguise( void );
	
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );

			void CacheTextures( void );

private:
	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hDisguiseFont, "DisguiseFont", "ClassGlyphs" );
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

	// For the disguising progress bar
	CPanelAnimationVar( Color, m_BarColor, "bar_color", "HUD_Tone_Default" );

	CPanelAnimationVarAliasType( float, bar_xpos, "bar_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_ypos, "bar_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_width, "bar_width", "75", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_height, "bar_height", "24", "proportional_float" );
	
	float m_flDisguiseStartTime;
	int	m_iDisguising;

	CHudTexture* m_pHudSpyDisguise;

	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSpyDisguise::CHudSpyDisguise( const char *pElementName ) : vgui::Panel( NULL, "HudSpyDisguise" ), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

	m_flDisguiseStartTime = 0.0f;
	m_iDisguising = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudSpyDisguise::~CHudSpyDisguise(void)
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

	if (m_pHudSpyDisguise)
	{
		delete m_pHudSpyDisguise;
		m_pHudSpyDisguise = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudSpyDisguise::Init(void)
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudSpyDisguise::VidInit( void )
{
	m_pHudSpyDisguise = new CHudTexture;
	m_pHudSpyDisguise->bRenderUsingFont = true;
	m_pHudSpyDisguise->hFont = m_hDisguiseFont;
	m_pHudSpyDisguise->cCharacterInFont = '_';

	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudSpyDisguise::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(SPY_DISGUISE_PROGRESS_BAR_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(SPY_DISGUISE_PROGRESS_BAR_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: When to draw and when to not
//-----------------------------------------------------------------------------
bool CHudSpyDisguise::ShouldDraw(void)
{
	if( !CHudElement::ShouldDraw() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return false;

	if( pPlayer->GetClassSlot() != CLASS_SPY || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return false;

	if( !pPlayer->IsDisguising() && !pPlayer->IsDisguised() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudSpyDisguise::Paint( void )
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	ConVarRef cl_teamcolourhud("cl_teamcolourhud");
	Color cColor = Color(255, 255, 255, 255);
	if( cl_teamcolourhud.GetBool() )
	{
		if ( g_PR )
			cColor = g_PR->GetTeamColor( pPlayer->GetTeamNumber() );
	}
	else
	{
		cColor = NON_TEAMCOLORED_HUD_COLOR;
	}
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, SPY_DISGUISE_PROGRESS_BAR_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, SPY_DISGUISE_PROGRESS_BAR_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	// Let's calculate and draw the disguising progress bar
	if ( pPlayer->IsDisguising() )
	{	
		if ( m_iDisguising != pPlayer->IsDisguising() )
		{
			m_iDisguising = pPlayer->IsDisguising();
			m_flDisguiseStartTime = gpGlobals->curtime;
		}

		float flRemainingTime = gpGlobals->curtime - m_flDisguiseStartTime;
		float iProgressPercent = ( ( 1 - ( SPY_DISGUISE_TIME - flRemainingTime ) / SPY_DISGUISE_TIME ) );

		// Draw progress bar
		surface()->DrawSetColor( m_BarColor );
		surface()->DrawFilledRect( bar_xpos, bar_ypos, ( bar_xpos + ( ( bar_width - bar_xpos ) * iProgressPercent ) ), bar_ypos + bar_height );
	}
	else
		m_iDisguising = 0;

	if( !pPlayer->IsDisguised() )
		return;	

	// Draw!
	if( m_pHudSpyDisguise )
	{
		// Figure out which glyph to use for the actual icon
		MapClassToGlyph( pPlayer->GetDisguisedClass(), m_pHudSpyDisguise->cCharacterInFont );

		Color clr = pPlayer->GetTeamColor();

		// Get disguised color
		if( g_PR )
			clr = g_PR->GetTeamColor( pPlayer->GetDisguisedTeam() );

		// Draw the icon
		m_pHudSpyDisguise->DrawSelf( image1_xpos, image1_ypos, clr );

		// Get the class as a string
		wchar_t szText[ 64 ];

		// Look up the resource string
		wchar_t *pszText = g_pVGuiLocalize->Find( Class_IntToResourceString( pPlayer->GetDisguisedClass() ) );

		// No valid resource string found
		if( !pszText )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( Class_IntToPrintString( pPlayer->GetDisguisedClass() ), szText, sizeof( szText ) );
			pszText = szText;
		}

		// Draw text
		surface()->DrawSetTextFont( m_hTextFont );
		surface()->DrawSetTextColor( clr );
		surface()->DrawSetTextPos( text1_xpos, text1_ypos );
		surface()->DrawUnicodeString( pszText );
	}
}

DECLARE_HUDELEMENT(CHudSpyDisguise);

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised weapon
//-----------------------------------------------------------------------------
class CHudSpyDisguise2 : public CHudElement, public vgui::Panel
{
public:
	DECLARE_CLASS_SIMPLE( CHudSpyDisguise2, vgui::Panel );

	CHudSpyDisguise2( const char* pElementName );
	virtual ~CHudSpyDisguise2( void );

	virtual void Init( void );
	virtual void VidInit( void );
	virtual void Paint( void );
	virtual bool ShouldDraw( void );

	void	CacheTextures( void );

private:
	CHudTexture* m_pBGTexture;
	CHudTexture* m_pFGTexture;

	CHudTexture* m_pWeaponIcon;

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hDisguisedWeaponFont, "WeaponFont", "WeaponIconsHUD" )
	CPanelAnimationVar( Color, m_hDisguisedWeaponColor, "WeaponColor", "HUD_Tone_Default" )

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSpyDisguise2::CHudSpyDisguise2( const char *pElementName ) : vgui::Panel( NULL, "HudSpyDisguise2" ), CHudElement( pElementName )
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );

	m_pBGTexture = NULL;
	m_pFGTexture = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CHudSpyDisguise2::~CHudSpyDisguise2()
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
void CHudSpyDisguise2::Init(void)
{
	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Once per map load
//-----------------------------------------------------------------------------
void CHudSpyDisguise2::VidInit( void )
{
	m_pWeaponIcon = new CHudTexture;
	m_pWeaponIcon->bRenderUsingFont = true;
	m_pWeaponIcon->hFont = m_hDisguisedWeaponFont;

	CacheTextures();
}

//-----------------------------------------------------------------------------
// Purpose: Load and precache the textures
//-----------------------------------------------------------------------------
void CHudSpyDisguise2::CacheTextures()
{
	if (!m_pBGTexture)
	{
		m_pBGTexture = new CHudTexture();
		m_pBGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(SPY_DISGUISE_BOX_BACKGROUND_TEXTURE);
	}

	if (!m_pFGTexture)
	{
		m_pFGTexture = new CHudTexture();
		m_pFGTexture->textureId = vgui::surface()->CreateNewTextureID();
		PrecacheMaterial(SPY_DISGUISE_BOX_FOREGROUND_TEXTURE);
	}
}

//-----------------------------------------------------------------------------
// Purpose: When to draw and when to not
//-----------------------------------------------------------------------------
bool CHudSpyDisguise2::ShouldDraw(void)
{
	if( !CHudElement::ShouldDraw() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return false;

	if( pPlayer->GetClassSlot() != CLASS_SPY || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return false;

	if( !pPlayer->IsDisguising() && !pPlayer->IsDisguised() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Paint
//-----------------------------------------------------------------------------
void CHudSpyDisguise2::Paint( void )
{
	C_FFPlayer* pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if ( !pPlayer )
		return;

	ConVarRef cl_teamcolourhud("cl_teamcolourhud");
	Color cColor = Color(255, 255, 255, 255);
	if( cl_teamcolourhud.GetBool() )
	{
		if ( g_PR )
			cColor = g_PR->GetTeamColor( pPlayer->GetTeamNumber() );
	}
	else
	{
		cColor = NON_TEAMCOLORED_HUD_COLOR;
	}
	cColor.setA(150);

	// draw our background first
	surface()->DrawSetTextureFile( m_pBGTexture->textureId, SPY_DISGUISE_BOX_BACKGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pBGTexture->textureId );
	surface()->DrawSetColor( cColor );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	surface()->DrawSetTextureFile( m_pFGTexture->textureId, SPY_DISGUISE_BOX_FOREGROUND_TEXTURE, true, false );
	surface()->DrawSetTexture( m_pFGTexture->textureId );
	surface()->DrawSetColor( GetFgColor() );
	surface()->DrawTexturedRect( 0, 0, GetWide(), GetTall() );

	if ( !pPlayer->IsDisguising() && !pPlayer->IsDisguised() )
		return;

	if ( pPlayer->IsDisguised() )
	{
		CFFWeaponBase *pWeapon = pPlayer->GetActiveFFWeapon();

		if(!pWeapon)
			return;

		int iSlot = pWeapon->GetFFWpnData().iSlot;
		int iClass = pPlayer->GetDisguisedClass();
		const char *disguisedWeaponName = "ff_weapon_crowbar";

		if(pPlayer->m_DisguisedWeapons[iClass][iSlot].szWeaponClassName[0] != NULL)
			disguisedWeaponName = pPlayer->m_DisguisedWeapons[iClass][iSlot].szWeaponClassName;
		
		if( Q_strnicmp( disguisedWeaponName, "ff_", 3 ) == 0 )
		{
			//UTIL_LogPrintf( "  begins with ff_, removing\n" );
			disguisedWeaponName += 3;
		}

		char disguised_weapon_name[256];
		Q_snprintf( disguised_weapon_name, sizeof(disguised_weapon_name), "weapon_%s", disguisedWeaponName );

		Color col = m_hDisguisedWeaponColor;

		// Shallow copy of the weapon scrolling icon
		m_pWeaponIcon = gHUD.GetIcon(disguised_weapon_name);
		// Change the font so it uses 28 size instead of 64
		m_pWeaponIcon->hFont = m_hDisguisedWeaponFont;
		m_pWeaponIcon->bRenderUsingFont = true;
		
		m_pWeaponIcon->DrawSelf( image1_xpos , image1_ypos, col);

	}
}

DECLARE_HUDELEMENT(CHudSpyDisguise2);