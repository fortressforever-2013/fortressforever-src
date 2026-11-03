//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "fortressforeverteammenu.h"
#include "tier0/icommandline.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <filesystem.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/HTML.h>

#include <game/client/iviewport.h>
#include <stdlib.h> // MAX_PATH define
#include <stdio.h>
#include "byteswap.h"

#include <networkstringtabledefs.h>
#include "ff_button.h"
#include "c_ff_player.h"
#include "c_ff_playerresource.h"
#include "ff_utils.h"
#include "c_ff_team.h"

#include <convar.h>
#include "hud.h" // for gEngfuncs
#include "IGameUIFuncs.h" // for key bindings
#include "ienginevgui.h"

using namespace vgui;
extern INetworkStringTable *g_pStringTableInfoPanel;

#define TEAM_BUTTON_GAP	20

ConVar cl_autojointeam(
	"cl_autojointeam",
	"0",
	FCVAR_ARCHIVE | FCVAR_DEVELOPMENTONLY,
	"Select once per map for testing: 1 Blue, 2 Red, 3 Yellow, 4 Green, 5 Auto Assign, 6 Spectate. Other values disable it.");

//=============================================================================
// A team button has the following components:
//		A number (this is the button text itself so that hotkeys work automatically)
//		The team insignia image
//		Score
//		Player count
//		Avg ping
//=============================================================================
class TeamButton : public FFButton
{
public:
	DECLARE_CLASS_SIMPLE(TeamButton, FFButton);

	TeamButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
		m_pTeamInsignia = new ImagePanel(this, "TeamImage");
		m_pInfoDescriptions = new Label(this, "TeamDescription", (const char *) NULL);
		m_pInfoValues = new Label(this, "TeamValues", (const char *) NULL);

		m_iTeamID = -1;
	}

	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);


		SetTextInset(10, 7);
		SetContentAlignment(a_northwest);

		// Line up image
		int iInsigniaSize = GetTall() * 0.4f;

		m_pTeamInsignia->SetBounds(GetWide() / 2 - iInsigniaSize / 2, GetTall() / 2.5f - iInsigniaSize / 2, iInsigniaSize, iInsigniaSize);
		m_pTeamInsignia->SetShouldScaleImage(true);
		m_pTeamInsignia->SetMouseInputEnabled(false);

		// Line up info descriptions
		m_pInfoDescriptions->SetContentAlignment(a_northwest);
		m_pInfoDescriptions->SetBounds(GetWide() * 0.1f, GetTall() * 0.6f, GetWide() * 0.4f, GetTall() * 0.4f);
		m_pInfoDescriptions->SetMouseInputEnabled(false);

		// Line up info values
		m_pInfoValues->SetContentAlignment(a_northeast);
		m_pInfoValues->SetBounds(GetWide() * 0.5f, GetTall() * 0.6f, GetWide() * 0.4f, GetTall() * 0.4f);
		m_pInfoValues->SetMouseInputEnabled(false);

		m_pInfoDescriptions->SetText("#TEAM_STATS");
		m_pInfoValues->SetText("0\n0\n0");
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set the team id used by this button for updating itself
	//			Also load the team insignia image.
	//-----------------------------------------------------------------------------
	void SetTeamID(int iTeamID)
	{
		Assert(iTeamID >= FF_TEAM_BLUE && iTeamID <= FF_TEAM_GREEN);
		m_iTeamID = iTeamID;

		UpdateTeamIcon(iTeamID);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update teams' icons
	//-----------------------------------------------------------------------------
	void UpdateTeamIcon(int iTeamID)
	{
		const char *pszInsignias[] = { "hud_team_blue", "hud_team_red", "hud_team_yellow", "hud_team_green" };
		C_FFTeam *pFFTeam = GetGlobalFFTeam(iTeamID);

		m_pTeamInsignia->SetShouldScaleImage(true);

		if (pFFTeam)
			m_pTeamInsignia->SetImage(pFFTeam->GetTeamIcon());
		else
			m_pTeamInsignia->SetImage(pszInsignias[iTeamID - FF_TEAM_BLUE]);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the various sections of the team button
	//-----------------------------------------------------------------------------
	void OnThink()
	{
		if ( g_FF_PR == NULL )
			return;

		int iScore = g_FF_PR->GetTeamScore(m_iTeamID);
		int nPlayers = 0;
		int iLag = 0;
		
		for (int iClient = 1; iClient <= gpGlobals->maxClients; iClient++)
		{
			if (!g_FF_PR->IsConnected(iClient) || g_FF_PR->GetTeam(iClient) != m_iTeamID)
				continue;

			nPlayers++;
			iLag += g_FF_PR->GetPing(iClient);
		}

		if (nPlayers > 0)
			iLag /= nPlayers;

		m_pInfoValues->SetText(VarArgs("%d\n%d\n%d", iScore, nPlayers, iLag));
		
		BaseClass::OnThink();
	}

private:

	ImagePanel	*m_pTeamInsignia;
	Label		*m_pInfoDescriptions;
	Label		*m_pInfoValues;

	int			m_iTeamID;
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFTeamMenu::CFFTeamMenu( IViewPort *pViewPort ) : CTeamMenu( pViewPort )
{
	// ServerInfo elements
/*	m_pServerInfoHost = new HTML(this, "ServerInfoHost");
	m_pServerInfoHost->SetScrollbarsEnabled( false );

	char *pszButtons[] = { "BlueTeamButton", "RedTeamButton", "YellowTeamButton", "GreenTeamButton" };

	for (int iTeamIndex = 0; iTeamIndex < ARRAYSIZE(pszButtons); iTeamIndex++)
	{
		m_pTeamButtons[iTeamIndex] = new TeamButton(this, pszButtons[iTeamIndex], (const char *) NULL, this, pszButtons[iTeamIndex]);
		m_pTeamButtons[iTeamIndex]->SetTeamID(iTeamIndex + FF_TEAM_BLUE);
	}

	m_pFlythroughButton = new FFButton(this, "FlythroughButton", (const char *) NULL, this, "mapguide");

	m_pMapScreenshotButton = new FFButton(this, "MapScreenshotButton", (const char *) NULL, this, "MapShot");

	// Mulch: Removed this
	//new Button(this, "screenshot", "screenshot", this, "jpeg");
*/
	m_pCancelButton = new Button( this, "CancelButton", "#FF_MENU_CANCEL" );
	m_pSpectateButton = new Button( this, "SpectateButton", "#FF_MENU_SPECTATOR" );

	// to get server name
	gameeventmanager->AddListener( this, "server_spawn", false );

	LoadControlSettings( "Resource/UI/TeamMenu.res" );
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFTeamMenu::~CFFTeamMenu()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFTeamMenu::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	Update();
}

// Retry while visible: the player and team resource may arrive after ShowPanel.
bool CFFTeamMenu::TryAutoJoin()
{
	// Honour testing launch options only with -dev; retain the development-only flag.
	static bool bLaunchOptionApplied = false;
	if (!bLaunchOptionApplied)
	{
		if (CommandLine()->CheckParm("-dev") && CommandLine()->FindParm("+cl_autojointeam"))
			cl_autojointeam.SetValue(CommandLine()->ParmValue("+cl_autojointeam", 0));
		bLaunchOptionApplied = true;
	}

	if (m_bAutoJoinAttempted)
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if (!pPlayer || !g_FF_PR)
		return false;

	const int iTeam = cl_autojointeam.GetInt();
	if (iTeam < 1 || iTeam > 6 || pPlayer->GetTeamNumber() >= FF_TEAM_BLUE)
	{
		m_bAutoJoinAttempted = true;
		return false;
	}

	if (iTeam == 6)
	{
		if (!mp_allowspectators.GetBool())
			return false;
		m_bAutoJoinAttempted = true;
		OnCommand("spectate");
		return true;
	}

	char teamSpaces[4];
	const int iFreeTeams = UTIL_GetTeamSpaces(teamSpaces);
	if ((iTeam == 5 && iFreeTeams == 0)
		|| (iTeam <= 4 && teamSpaces[iTeam - 1] <= 0))
		return false;

	const char *commands[] = { "team blue", "team red", "team yellow", "team green", "team auto" };
	m_bAutoJoinAttempted = true;
	OnCommand(commands[iTeam - 1]);
	return true;
}

void CFFTeamMenu::OnThink()
{
	BaseClass::OnThink();
	if (IsVisible())
		TryAutoJoin();
}

void CFFTeamMenu::ShowPanel( bool bShow )
{
	if (bShow && TryAutoJoin())
		return;

	if ( BaseClass::IsVisible() == bShow )
		return;

	m_pViewPort->ShowBackGround( true );

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
		SetEnabled( true );

		Update();

		MoveToFront();

		SetCloseButtonVisible( false );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: When a team button is pressed it triggers this function to 
//			cause the player to join a team
//-----------------------------------------------------------------------------
void CFFTeamMenu::OnCommand( const char *command )
{
	// A manual selection or dismissal ends automatic selection for this map.
	m_bAutoJoinAttempted = true;
	if ( Q_stricmp( command, "vguicancel" ) )
	{
		engine->ClientCmd( command );
	}

	if ( Q_strstr( command, "team " ) )
	{
		engine->ClientCmd( command );
		
		// Display the class panel if we have the 'team' substring, now
	}

	BaseClass::OnCommand( command );
	gViewPortInterface->ShowBackGround( false );
	OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: Get the server name
//-----------------------------------------------------------------------------
void CFFTeamMenu::FireGameEvent( IGameEvent *event )
{
	const char *type = event->GetName();

	if ( !Q_strcmp(type, "server_spawn") )
	{
		Reset();
		Q_strncpy( m_szServerName, event->GetString("hostname"), sizeof(m_szServerName) );
	}

	if( IsVisible() )
		Update();
}

//-----------------------------------------------------------------------------
// Purpose: called to update the menu with new information
//-----------------------------------------------------------------------------
void CFFTeamMenu::Update()
{
	BaseClass::Update();
	
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if ( pPlayer )
	{
		if ( m_pCancelButton )
			m_pCancelButton->SetVisible( pPlayer->GetTeamNumber() != TEAM_UNASSIGNED );
		if ( m_pSpectateButton )
			m_pSpectateButton->SetVisible( mp_allowspectators.GetBool() );
	}

	// TODO: Some of these should only happen once per map
	UpdateMapDescriptionText();
	UpdateServerInfo();
	UpdateTeamButtons();
	UpdateTeamIcons();
	// When a map 1st loads, the image proxy is in front of the button (so you can't click it) -- this seems to fix that
	//m_pMapScreenshotButton->SetZPos( 10 );
}


void CFFTeamMenu::OnKeyCodePressed( vgui::KeyCode code )
{/*
	// Show the scoreboard over this if needed
	if ( gameuifuncs->GetButtonCodeForBind( "showscores" ) == code )
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, true );

	// Support hiding the team menu by hitting your changeteam button again like TFC
	// 0001232: Or if the user presses escape, kill the menu
	if ( gameuifuncs->GetButtonCodeForBind( "changeteam" ) == code ||
		gameuifuncs->GetButtonCodeForBind("cancelselect") == code )
		gViewPortInterface->ShowPanel( this, false );

	// Bug #0000540: Can't changeclass while changeteam menu is up
	// Support bring the class menu back up if the team menu is showing
	if( gameuifuncs->GetButtonCodeForBind( "changeclass" ) == code &&
		( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() >= FIRST_GAME_TEAM ) )
	{
		m_pViewPort->ShowPanel( this, false );
		engine->ClientCmd( "changeclass" );
	}
	
	if ( gameuifuncs->GetButtonCodeForBind( "serverinfo" ) == code )
		engine->ClientCmd( "serverinfo" );

	BaseClass::OnKeyCodePressed( code );
*/}

void CFFTeamMenu::OnKeyCodeReleased( vgui::KeyCode code )
{
	// Bug #0000524: Scoreboard gets stuck with the class menu up when you first join
	// Hide the scoreboard now
	if ( gameuifuncs->GetButtonCodeForBind( "showscores") == code )
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, false );

	BaseClass::OnKeyCodeReleased( code );
}

void CFFTeamMenu::UpdateTeamButtons()
{
/*	if (g_FF_PR == NULL)
		return;

	char nTeamSpaces[4];
	UTIL_GetTeamSpaces(nTeamSpaces);

	int nActiveButtons = 0;

	for (int iTeamID = FF_TEAM_BLUE; iTeamID <= FF_TEAM_GREEN; iTeamID++)
	{
		int iTeamIndex = iTeamID - FF_TEAM_BLUE;
		TeamButton *pTeamButton = m_pTeamButtons[iTeamIndex];

		// This team doesn't exist at all
		if (nTeamSpaces[iTeamIndex] == -1)
		{
			pTeamButton->SetVisible(false);
			continue;
		}

		// Make sure team button is visible
		pTeamButton->SetVisible(true);

		// Need this for later
		nActiveButtons++;

		// The team is full, so disable 
		pTeamButton->SetEnabled((nTeamSpaces[iTeamIndex] != 0));

		// set the team number
		wchar_t wchTeamNumber = iTeamIndex + '1';

		// Set the team name
		wchar_t *wszTeamName = g_pVGuiLocalize->Find( g_FF_PR->GetTeamName( iTeamID ) );
		wchar_t	wszName[ 256 ];

		if (wszTeamName)
		{
			V_snwprintf( wszName, ARRAYSIZE(wszName), L"%c. %ls", wchTeamNumber, wszTeamName );
			wszTeamName = wszName;
		}
		else
		{
			// No localized text or team name not a resource string
			char szString[ 256 ];
			Q_snprintf( szString, 256, "%c. %s", wchTeamNumber, g_FF_PR->GetTeamName( iTeamID ) );
			g_pVGuiLocalize->ConvertANSIToUnicode( szString, wszName, sizeof( wszName ) );
			wszTeamName = wszName;
		}

		// one last check
		if ( !wszTeamName )
		{
			// no name, just use the number
			// what the FUCK are you DOING!!!!! - azzy
			// wszTeamName is a wchar_t fucking pointer !!!
			// V_snwprintf( wszTeamName, sizeof(wszTeamName), L"%c.", wchTeamNumber );
			V_snwprintf( wszName, ARRAYSIZE(wszName), L"%c.", wchTeamNumber );
			wszTeamName = wszName;

			pTeamButton->SetText(wszTeamName);
		}
		else
			pTeamButton->SetText(wszTeamName);

		pTeamButton->SetHotkey(wchTeamNumber);
	}

	int iTeamButtonGap = scheme()->GetProportionalScaledValue(TEAM_BUTTON_GAP);

	// Now that we know how many active buttons there are we can
	// move them into the correct location
	int iStartX = m_pTeamButtons[0]->GetWide() * nActiveButtons * 0.5f;
	iStartX += (nActiveButtons - 1) * (iTeamButtonGap * 0.5f);
	iStartX = (GetWide() * 0.5f) - iStartX;

	// Move active buttons in correct place
	for (int iTeamIndex = 0; iTeamIndex < 4; iTeamIndex++)
	{
		TeamButton *pTeamButton = m_pTeamButtons[iTeamIndex];

		if (pTeamButton->IsVisible())
		{
			int iXPos, iYPos;
			pTeamButton->GetPos(iXPos, iYPos);

			pTeamButton->SetPos(iStartX, iYPos);
			iStartX += pTeamButton->GetWide() + iTeamButtonGap;
		}
		
	}

	// Done here for now
	int iXPos, iYPos;

	m_pSpectateButton->GetPos(iXPos, iYPos);
	m_pSpectateButton->SetPos(GetWide() / 2 - (m_pAutoAssignButton->GetWide() + iTeamButtonGap / 2), iYPos);

	m_pAutoAssignButton->GetPos(iXPos, iYPos);
	m_pAutoAssignButton->SetPos(GetWide() / 2 + iTeamButtonGap / 2, iYPos);
*/
}

void CFFTeamMenu::UpdateServerInfo()
{
	//m_pServerInfoText->SetText(pszTitle);

	if (g_pStringTableInfoPanel == NULL)
		return;
/*
	int x,y;
	m_pServerInfoButton->GetPos( x,y );
	m_pServerInfoButton->SetPos( x, scheme()->GetProportionalScaledValue( 34 ) );

	int iIndex = g_pStringTableInfoPanel->FindStringIndex("host");

	if (iIndex == ::INVALID_STRING_INDEX)
	{
		m_pServerInfoHost->OpenURL( VarArgs("http://www.fortress-forever.com/defaulthost/index.php?name=%s", m_szServerName), NULL );
		return;
	}

	int nLength = 0;
	const char *pszMotd = (const char *) g_pStringTableInfoPanel->GetStringUserData(iIndex, &nLength);

	m_pServerInfoHost->OpenURL(pszMotd, NULL);
*/
}

void CFFTeamMenu::UpdateMapDescriptionText()
{
/*	char szMapName[MAX_MAP_NAME];
	Q_FileBase(engine->GetLevelName(), szMapName, sizeof(szMapName));

	const char *pszMapPath = VarArgs("maps/%s.txt", szMapName);

	// If no map specific description exists then escape for now
	if (!g_pFullFileSystem->FileExists(pszMapPath))
	{
//		VarArgs("maps/default.txt", szMapName); "This fallback idea was inspired by Zombie Panic: Source" -BreakinBenny
		m_pMapDescriptionHead->SetText("");
		m_pMapDescriptionText->SetText("");
		return;
	}

	// Read from local text from file
	FileHandle_t f = g_pFullFileSystem->Open(pszMapPath, "rb", "GAME");

	if (!f) 
		return;

	char szBuffer[2048];
				
	int size = min(g_pFullFileSystem->Size(f), sizeof(szBuffer) - 1);

	g_pFullFileSystem->Read(szBuffer, size, f);
	g_pFullFileSystem->Close(f);

	szBuffer[size] = 0;

	const char *pszEndOfHead = strstr(szBuffer, "\n");

	// Could not find a title for this, just stick everything in the normal spot
	// Or there was nothing after the title
	if ( !pszEndOfHead || ( (*(pszEndOfHead + 1)) == '\0' ) )
	{
		m_pMapDescriptionHead->SetText("Unknown map style");
		m_pMapDescriptionText->SetText(szBuffer);

		return;
	}

	int iEndOfHead = pszEndOfHead - szBuffer;

	// Put the rest of the text into the description richtext first.
	// We're then just going to stick in a null terminator and then add the start
	// to the head label.
	m_pMapDescriptionText->SetText(pszEndOfHead + 1);
	
	szBuffer[iEndOfHead] = 0;
	m_pMapDescriptionHead->SetText(szBuffer);
*/}


void CFFTeamMenu::UpdateTeamIcons()
{
/*	for (int iTeamID = 0; iTeamID < 4; iTeamID++)
	{
		m_pTeamButtons[iTeamID]->UpdateTeamIcon(iTeamID + FF_TEAM_BLUE);
	}
*/}