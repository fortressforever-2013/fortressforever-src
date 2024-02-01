//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef CDLL_CLIENT_INT_H
#define CDLL_CLIENT_INT_H
#ifdef _WIN32
#pragma once
#endif

#include "iclientnetworkable.h"
#include "utllinkedlist.h"
#include "cdll_int.h"
#include "eiface.h"


class IVModelRender;
class IVEngineClient;
class IVModelRender;
class IVEfx;
class IVRenderView;
class IVDebugOverlay;
class IMaterialSystem;
class IMaterialSystemStub;
class IDataCache;
class IMDLCache;
class IVModelInfoClient;
class IEngineVGui;
class ISpatialPartition;
class IBaseClientDLL;
class ISpatialPartition;
class IFileSystem;
class IStaticPropMgrClient;
class IShadowMgr;
class IEngineSound;
class IMatSystemSurface;
class IMaterialSystemHardwareConfig;
class ISharedGameRules;
class IEngineTrace;
class IGameUIFuncs;
class IGameEventManager2;
class IPhysicsGameTrace;
class CGlobalVarsBase;
class IClientTools;
class C_BaseAnimating;
class IColorCorrectionSystem;
class IInputSystem;
class ISceneFileCache;
class IXboxSystem;	// Xbox 360 only
class IMatchmaking;
class IVideoServices;
class CSteamAPIContext;
class IClientReplayContext;
class IReplayManager;
class IEngineReplay;
class IEngineClientReplay;
class IReplayScreenshotManager;
class CSteamID;

//=============================================================================
// HPE_BEGIN
// [dwenger] Necessary for stats display
//=============================================================================

class AchievementsAndStatsInterface;

//=============================================================================
// HPE_END
//=============================================================================

extern IVModelRender *modelrender;
extern IVEngineClient	*engine;
extern IVModelRender *modelrender;
extern IVEfx *effects;
extern IVRenderView *render;
extern IVDebugOverlay *debugoverlay;
extern IMaterialSystem *materials;
extern IMaterialSystemStub *materials_stub;
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;
extern IDataCache *datacache;
extern IMDLCache *mdlcache;
extern IVModelInfoClient *modelinfo;
extern IEngineVGui *enginevgui;
extern ISpatialPartition* partition;
extern IBaseClientDLL *clientdll;
extern IFileSystem *filesystem;
extern IStaticPropMgrClient *staticpropmgr;
extern IShadowMgr *shadowmgr;
extern IEngineSound *enginesound;
extern IMatSystemSurface *g_pMatSystemSurface;
extern IEngineTrace *enginetrace;
extern IGameUIFuncs *gameuifuncs;
extern IGameEventManager2 *gameeventmanager;
extern IPhysicsGameTrace *physgametrace;
extern CGlobalVarsBase *gpGlobals;
extern IClientTools *clienttools;
extern IInputSystem *inputsystem;
extern ISceneFileCache *scenefilecache;
extern IXboxSystem *xboxsystem;	// Xbox 360 only
extern IMatchmaking *matchmaking;
extern IVideoServices *g_pVideo;
extern IUploadGameStats *gamestatsuploader;
extern CSteamAPIContext *steamapicontext;
extern IReplaySystem *g_pReplay;
extern IClientReplayContext *g_pClientReplayContext;
extern IReplayManager *g_pReplayManager;
extern IReplayScreenshotManager *g_pReplayScreenshotManager;
extern IEngineReplay *g_pEngineReplay;
extern IEngineClientReplay *g_pEngineClientReplay;

//=============================================================================
// HPE_BEGIN
// [dwenger] Necessary for stats display
//=============================================================================

extern AchievementsAndStatsInterface* g_pAchievementsAndStatsInterface;

//=============================================================================
// HPE_END
//=============================================================================

// Set to true between LevelInit and LevelShutdown.
extern bool	g_bLevelInitialized;
extern bool g_bTextMode;

// Kyle says: this is here to obsfucate our accessing of the g_bTextMode variable and for no other purpose.
//			  See the mess of TF_ANTI_IDLEBOT_VERIFICATION code. If that code doesn't exist anymore, this
//			  probably also doesn't need to exist anymore.
//
//			  On a suggestion from Joe, we also point it to an incomplete type.
extern class IClientPurchaseInterfaceV2 *g_pClientPurchaseInterface;

//-----------------------------------------------------------------------------
// Purpose: engine to client .dll interface
//-----------------------------------------------------------------------------
// FF: Class definition moved here from the .cpp so that we can inherit from it
class CHLClient : public IBaseClientDLL
{
public:
	CHLClient();

	virtual int						Init(CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase* pGlobals);

	virtual void					PostInit();
	virtual void					Shutdown(void);

	virtual bool					ReplayInit(CreateInterfaceFn fnReplayFactory);
	virtual bool					ReplayPostInit();

	virtual void					LevelInitPreEntity(const char* pMapName);
	virtual void					LevelInitPostEntity();
	virtual void					LevelShutdown(void);

	virtual ClientClass* GetAllClasses(void);

	virtual int						HudVidInit(void);
	virtual void					HudProcessInput(bool bActive);
	virtual void					HudUpdate(bool bActive);
	virtual void					HudReset(void);
	virtual void					HudText(const char* message);

	// Mouse Input Interfaces
	virtual void					IN_ActivateMouse(void);
	virtual void					IN_DeactivateMouse(void);
	virtual void					IN_Accumulate(void);
	virtual void					IN_ClearStates(void);
	virtual bool					IN_IsKeyDown(const char* name, bool& isdown);
	virtual void					IN_OnMouseWheeled(int nDelta);
	// Raw signal
	virtual int						IN_KeyEvent(int eventcode, ButtonCode_t keynum, const char* pszCurrentBinding);
	virtual void					IN_SetSampleTime(float frametime);
	// Create movement command
	virtual void					CreateMove(int sequence_number, float input_sample_frametime, bool active);
	virtual void					ExtraMouseSample(float frametime, bool active);
	virtual bool					WriteUsercmdDeltaToBuffer(bf_write* buf, int from, int to, bool isnewcommand);
	virtual void					EncodeUserCmdToBuffer(bf_write& buf, int slot);
	virtual void					DecodeUserCmdFromBuffer(bf_read& buf, int slot);


	virtual void					View_Render(vrect_t* rect);
	virtual void					RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw);
	virtual void					View_Fade(ScreenFade_t* pSF);

	virtual void					SetCrosshairAngle(const QAngle& angle);

	virtual void					InitSprite(CEngineSprite* pSprite, const char* loadname);
	virtual void					ShutdownSprite(CEngineSprite* pSprite);

	virtual int						GetSpriteSize(void) const;

	virtual void					VoiceStatus(int entindex, qboolean bTalking);

	virtual void					InstallStringTableCallback(const char* tableName);

	virtual void					FrameStageNotify(ClientFrameStage_t curStage);

	virtual bool					DispatchUserMessage(int msg_type, bf_read& msg_data);

	// Save/restore system hooks
	virtual CSaveRestoreData* SaveInit(int size);
	virtual void			SaveWriteFields(CSaveRestoreData*, const char*, void*, datamap_t*, typedescription_t*, int);
	virtual void			SaveReadFields(CSaveRestoreData*, const char*, void*, datamap_t*, typedescription_t*, int);
	virtual void			PreSave(CSaveRestoreData*);
	virtual void			Save(CSaveRestoreData*);
	virtual void			WriteSaveHeaders(CSaveRestoreData*);
	virtual void			ReadRestoreHeaders(CSaveRestoreData*);
	virtual void			Restore(CSaveRestoreData*, bool);
	virtual void			DispatchOnRestore();
	virtual void			WriteSaveGameScreenshot(const char* pFilename);

	// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
	//  the appropriate close caption if running with closecaption = 1
	virtual void			EmitSentenceCloseCaption(char const* tokenstream);
	virtual void			EmitCloseCaption(char const* captionname, float duration);

	virtual CStandardRecvProxies* GetStandardRecvProxies();

	virtual bool			CanRecordDemo(char* errorMsg, int length) const;

	virtual void			OnDemoRecordStart(char const* pDemoBaseName);
	virtual void			OnDemoRecordStop();
	virtual void			OnDemoPlaybackStart(char const* pDemoBaseName);
	virtual void			OnDemoPlaybackStop();

	virtual bool			ShouldDrawDropdownConsole();

	// Get client screen dimensions
	virtual int				GetScreenWidth();
	virtual int				GetScreenHeight();

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshotOfSize(const char* pFilename, int width, int height, bool bCreatePowerOf2Padded/*=false*/, bool bWriteVTF/*=false*/);

	// Gets the location of the player viewpoint
	virtual bool			GetPlayerView(CViewSetup& playerView);

	// Matchmaking
	virtual void			SetupGameProperties(CUtlVector< XUSER_CONTEXT >& contexts, CUtlVector< XUSER_PROPERTY >& properties);
	virtual uint			GetPresenceID(const char* pIDName);
	virtual const char* GetPropertyIdString(const uint id);
	virtual void			GetPropertyDisplayString(uint id, uint value, char* pOutput, int nBytes);
	virtual void			StartStatsReporting(HANDLE handle, bool bArbitrated);

	virtual void			InvalidateMdlCache();

	virtual void			ReloadFilesInList(IFileList* pFilesToReload);

	// Let the client handle UI toggle - if this function returns false, the UI will toggle, otherwise it will not.
	virtual bool			HandleUiToggle();

	// Allow the console to be shown?
	virtual bool			ShouldAllowConsole();

	// Get renamed recv tables
	virtual CRenamedRecvTableInfo* GetRenamedRecvTableInfos();

	// Get the mouthinfo for the sound being played inside UI panels
	virtual CMouthInfo* GetClientUIMouthInfo();

	// Notify the client that a file has been received from the game server
	virtual void			FileReceived(const char* fileName, unsigned int transferID);

	virtual const char* TranslateEffectForVisionFilter(const char* pchEffectType, const char* pchEffectName);

	virtual void			ClientAdjustStartSoundParams(struct StartSoundParams_t& params);

	// Returns true if the disconnect command has been handled by the client
	virtual bool DisconnectAttempt(void);
public:
	void PrecacheMaterial(const char* pMaterialName);

	virtual bool IsConnectedUserInfoChangeAllowed(IConVar* pCvar);

private:
	void UncacheAllMaterials();
	void ResetStringTablePointers();

	CUtlVector< IMaterial* > m_CachedMaterials;
};

// Returns true if a new OnDataChanged event is registered for this frame.
bool AddDataChangeEvent( IClientNetworkable *ent, DataUpdateType_t updateType, int *pStoredEvent );

void ClearDataChangedEvent( int iStoredEvent );

//-----------------------------------------------------------------------------
// Precaches a material
//-----------------------------------------------------------------------------
void PrecacheMaterial( const char *pMaterialName );

//-----------------------------------------------------------------------------
// Converts a previously precached material into an index
//-----------------------------------------------------------------------------
int GetMaterialIndex( const char *pMaterialName );

//-----------------------------------------------------------------------------
// Converts precached material indices into strings
//-----------------------------------------------------------------------------
const char *GetMaterialNameFromIndex( int nIndex );

//-----------------------------------------------------------------------------
// Precache-related methods for particle systems
//-----------------------------------------------------------------------------
void PrecacheParticleSystem( const char *pParticleSystemName );
int GetParticleSystemIndex( const char *pParticleSystemName );
const char *GetParticleSystemNameFromIndex( int nIndex );


//-----------------------------------------------------------------------------
// Called during bone setup to test perf
//-----------------------------------------------------------------------------
void TrackBoneSetupEnt( C_BaseAnimating *pEnt );

bool IsEngineThreaded();

#ifndef NO_STEAM

/// Returns Steam ID, given player index.   Returns an invalid SteamID upon
/// failure
extern CSteamID GetSteamIDForPlayerIndex( int iPlayerIndex );

#endif

#endif // CDLL_CLIENT_INT_H
