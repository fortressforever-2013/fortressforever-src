// PORTED FROM OF / TF2C - AZZY

#include "cbase.h"
#include "fmtstr.h"

#ifdef GAME_DLL
#include "gameinterface.h"
extern CServerGameDLL g_ServerGameDLL;
#endif

enum EFFCVOverrideFlags
{
	OVERRIDE_SERVER = ( 1 << 0 ),
	OVERRIDE_CLIENT = ( 1 << 1 ),

	OVERRIDE_BOTH = ( OVERRIDE_SERVER | OVERRIDE_CLIENT ),
};

struct CFFConVarDefaultOverrideEntry
{
	EFFCVOverrideFlags nFlags; // flags
	const char *pszName;       // ConVar name
	const char *pszValue;      // new default value
};

static CFFConVarDefaultOverrideEntry s_FFConVarOverrideEntries[] =
{
	/* interpolation */
	{ OVERRIDE_CLIENT, "cl_interp",               "0.0" },
	{ OVERRIDE_CLIENT, "cl_interp_ratio",       "2.0" },

	/* lag compensation */
	{ OVERRIDE_CLIENT, "cl_lagcompensation",      "1" },

	/* client-side prediction */
	{ OVERRIDE_CLIENT, "cl_pred_optimize",        "2" },
	{ OVERRIDE_CLIENT, "cl_smooth",               "1" },
	{ OVERRIDE_CLIENT, "cl_smoothtime",        "0.05" },

	/* client rates */
	{ OVERRIDE_CLIENT, "rate",                "80000" },
	{ OVERRIDE_CLIENT, "cl_updaterate",          "66" },
	{ OVERRIDE_CLIENT, "cl_cmdrate",             "66" },

	/* server rates */
	{ OVERRIDE_SERVER, "sv_minrate",          "65536" }, // 0.5 mebibits per second
	{ OVERRIDE_SERVER, "sv_maxrate",              "0" }, // Real max is 1048576 == 8 mebibits per second to bytes per second
	{ OVERRIDE_SERVER, "sv_minupdaterate",       "30" },
	{ OVERRIDE_SERVER, "sv_maxupdaterate",       "66" },
	{ OVERRIDE_SERVER, "sv_mincmdrate",          "30" },
	{ OVERRIDE_SERVER, "sv_maxcmdrate",          "66" },
	{ OVERRIDE_SERVER, "sv_client_predict",       "1" }, // Force clients to predict

	/* voice */
	{ OVERRIDE_SERVER, "sv_voicecodec", "steam" },
	//{ OVERRIDE_CLIENT, "voice_maxgain",           "1" },

	/* nav mesh generation */
	{ OVERRIDE_SERVER, "nav_area_max_size",      "10" },

	/* fix maps with large lightmaps crashing on load */
	{ OVERRIDE_BOTH, "r_hunkalloclightmaps", "0" },

	/* optimization for projected textures. broken for sdk2013 */
	{ OVERRIDE_CLIENT, "r_flashlightscissor", "0" },

	/* ff's interp ratio limits */
	{ OVERRIDE_SERVER, "sv_client_min_interp_ratio",      "0.1" },
	{ OVERRIDE_SERVER, "sv_client_max_interp_ratio",      "4.0" },
};


// override some of the more idiotic ConVar defaults with more reasonable values
class CFFConVarDefaultOverride : public CAutoGameSystem
{
public:
	CFFConVarDefaultOverride() : CAutoGameSystem( "FFConVarDefaultOverride" ) {}
	virtual ~CFFConVarDefaultOverride() {}

	virtual bool Init() OVERRIDE;
	virtual void PostInit() OVERRIDE;

private:
	void OverrideDefault( const CFFConVarDefaultOverrideEntry& entry );

#ifdef GAME_DLL
	static const char *DLLName() { return "SERVER"; }
#else
	static const char *DLLName() { return "CLIENT"; }
#endif

};
static CFFConVarDefaultOverride s_TFConVarOverride;

// There is a buffer overflow involving sv_downloadurl.
// We need to clamp it to 128 chars or less.
// This will get hardened and moved to sdk gigalib eventually.
// -sappho
void sv_downloadurl_changed(IConVar* var, const char* pOldValue, float flOldValue)
{
	ConVar* sv_downloadurl = cvar->FindVar("sv_downloadurl");
	const char* sv_downloadurl_cstr = sv_downloadurl->GetString();
	if (strlen(sv_downloadurl_cstr) > 127)
	{
		Warning("sv_downloadurl too long, malformed, or otherwise bad! Kicking clients...\n");

#ifdef CLIENT_DLL
		engine->ExecuteClientCmd("disconnect");
#endif
		sv_downloadurl->SetValue("https://0.0.0.0/sv_download_url_is_bad");
	}
}

#ifdef CLIENT_DLL
void runDLUrlWrangling()
{
	ConVarRef refDLUrl("sv_downloadurl");
	if (!refDLUrl.IsValid())
	{
		return;
	}
	ConVar* DLUrlPtr = static_cast<ConVar*>(refDLUrl.GetLinkedConVar());

	DLUrlPtr->InstallChangeCallback(sv_downloadurl_changed);
}
#endif

void CFFConVarDefaultOverride::PostInit()
{
#ifdef CLIENT_DLL
	runDLUrlWrangling();
#endif
}

bool CFFConVarDefaultOverride::Init()
{
	for ( const auto& entry : s_FFConVarOverrideEntries )
	{
		OverrideDefault( entry );
	}

#ifdef GAME_DLL
	/* ensure that server-side clock correction is ACTUALLY limited to 2 tick-intervals */
	static CFmtStrN<16> s_Buf( "%.0f", 2 * ( g_ServerGameDLL.GetTickInterval() * 1000.0f ) );
	OverrideDefault( { OVERRIDE_SERVER, "sv_clockcorrection_msecs", s_Buf } );
#endif
	return true;
}



void CFFConVarDefaultOverride::OverrideDefault( const CFFConVarDefaultOverrideEntry& entry )
{
#ifdef GAME_DLL
	if ( !( entry.nFlags & OVERRIDE_SERVER ) )
		return;
#else
	if ( !( entry.nFlags & OVERRIDE_CLIENT ) )
		return;
#endif

	ConVarRef ref( entry.pszName, true );
	if ( !ref.IsValid() )
	{
		DevWarning( "[%s] CFFConVarDefaultOverride: can't get a valid ConVarRef for \"%s\"\n", DLLName(), entry.pszName );
		return;
	}

	auto pConVar = dynamic_cast<ConVar *>( ref.GetLinkedConVar() );
	if ( pConVar == nullptr )
	{
		DevWarning( "[%s] CFFConVarDefaultOverride: can't get a ConVar ptr for \"%s\"\n", DLLName(), entry.pszName );
		return;
	}

	CUtlString strOldDefault( pConVar->GetDefault() );
	CUtlString strOldValue  ( pConVar->GetString()  );

	/* override the convar's default, and if it was at the default, keep it there */
	bool bWasDefault = FStrEq( pConVar->GetString(), pConVar->GetDefault() );
	pConVar->SetDefault( entry.pszValue );
	if ( bWasDefault )
		pConVar->Revert();

	DevMsg( "[%s] CFFConVarDefaultOverride: \"%s\" was \"%s\"/\"%s\", now \"%s\"/\"%s\"\n", DLLName(), entry.pszName,
		strOldDefault.Get(), strOldValue.Get(), pConVar->GetDefault(), pConVar->GetString() );
}

