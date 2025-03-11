/********************************************************************
	created:	2006/09/22
	created:	22:9:2006   17:25
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_modelpanel.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_modelpanel
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	ModelPanel - display a static model
				PlayerModelPanel - display an animated player model
*********************************************************************/

#include "cbase.h"
#include "ff_modelpanel.h"
#include "model_types.h"
#include "dlight.h"
#include "iefx.h"
#include "view.h"
#include "view_shared.h"
#include "iviewrender.h"

#include "ff_playerclass_parse.h"
#include "ff_weapon_parse.h"

#include "ff_utils.h"

#include <igameresources.h>

extern IFileSystem **pFilesystem;

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ModelPanel::ModelPanel(Panel *pParent, const char *pName) : BaseClass(pParent, pName)
{
	m_szModelName[0] = 0;
	m_hModel = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ModelPanel::~ModelPanel()
{
	if (m_hModel)
	{
		m_hModel->Release();
		m_hModel = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: The engine will automatically free the models on map change
//			Must reset pointer manually
//-----------------------------------------------------------------------------
void ModelPanel::Reset()
{
	if (m_hModel)
	{
		m_hModel->Release();
		m_hModel = NULL;
	}

	m_szModelName[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: The model needs to have lighting set up, positioning taken place
//			and the correct view3d mode pushed.
//-----------------------------------------------------------------------------
void ModelPanel::Paint()
{
	if (m_hModel == NULL)
		return;

	Vector vecOrigin;

	SetupPositioningAndLighting(vecOrigin);
	DoRendering(vecOrigin);
}

//-----------------------------------------------------------------------------
// Purpose: Create a dynamic light and position the model
//-----------------------------------------------------------------------------
void ModelPanel::SetupPositioningAndLighting(Vector &vecOrigin)
{
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();
	
	vecOrigin = pLocalPlayer->EyePosition();
	
	CMatRenderContextPtr pRenderContext(materials);

	pRenderContext->SetLightingOrigin(vecOrigin);
	pRenderContext->SetAmbientLight(0.4, 0.4, 0.4);

	static Vector white[6] =
	{
		Vector(0.4, 0.4, 0.4),
		Vector(0.4, 0.4, 0.4),
		Vector(0.4, 0.4, 0.4),
		Vector(0.4, 0.4, 0.4),
		Vector(0.4, 0.4, 0.4),
		Vector(0.4, 0.4, 0.4),
	};

	g_pStudioRender->SetAmbientLightColors(white);
	g_pStudioRender->SetLocalLights(0, NULL);

	Vector vecMins, vecMaxs;
	m_hModel->GetRenderBounds(vecMins, vecMaxs);
	LightDesc_t spotLight(vecOrigin + Vector(0, 0, 200), Vector(1, 1, 1), m_hModel->GetAbsOrigin() + Vector(0, 0, (vecMaxs.z - vecMins.z) * 0.75), 0.035, 0.873);
	g_pStudioRender->SetLocalLights(1, &spotLight);

	// Move model in front of our view
	m_hModel->SetAbsOrigin(vecOrigin);
	m_hModel->SetAbsAngles(QAngle(0, /*210*/ 100.0f * gpGlobals->curtime, 0));
}

//-----------------------------------------------------------------------------
// Purpose: Set up the view, push it, draw the models and pop it
//-----------------------------------------------------------------------------
void ModelPanel::DoRendering(Vector vecOrigin)
{
	int x, y, width, height;

	GetBounds(x, y, width, height);
	//LocalToScreen(x, y);

	CViewSetup view;
	view.x = x;
	view.y = y;
	view.width = width;
	view.height = height;
	view.m_bOrtho = false;
	view.fov = 54;
	view.origin = vecOrigin + Vector(-110, -5, -5);

	// Ensure that we see all the player model
	Vector vMins, vMaxs;
	m_hModel->C_BaseAnimating::GetRenderBounds(vMins, vMaxs);
	view.origin.z += (vMins.z + vMaxs.z) * 0.55f;

	view.angles.Init();
	//view.m_vUnreflectedOrigin = view.origin;
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	//render->Get
	Frustum dummyFrustum;
	render->Push3DView(view, 0, NULL, ::view->GetFrustum());
	DrawModels();
	render->PopView(::view->GetFrustum());
}

//-----------------------------------------------------------------------------
// Purpose: Models to be drawn are here so they can be overridden easily
//-----------------------------------------------------------------------------
void ModelPanel::DrawModels()
{
	modelrender->SuppressEngineLighting(true);
	float color[3] = { 1.0f, 1.0f, 1.0f };
	render->SetColorModulation(color);
	render->SetBlend(1.0f);
	m_hModel->DrawModel(STUDIO_RENDER);
	modelrender->SuppressEngineLighting(false);
}

//-----------------------------------------------------------------------------
// Purpose: Set the model to be shown in this model thing.
//			These are just static prop models without an anim
//-----------------------------------------------------------------------------
void ModelPanel::SetModel(const char *pszModelName)
{
	if (m_hModel == NULL || Q_strcmp(pszModelName, m_szModelName) != 0)
	{
		if (m_hModel)
			m_hModel->Remove();

		C_BaseAnimatingOverlay *pModel = m_hModel.Get();

		pModel = new C_BaseAnimatingOverlay;
		pModel->InitializeAsClientEntity(pszModelName, RENDER_GROUP_OPAQUE_ENTITY);
		pModel->AddEffects(EF_NODRAW);

		m_hModel = pModel;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
PlayerModelPanel::PlayerModelPanel(Panel *pParent, const char *pName) : BaseClass(pParent, pName)
{
	m_hWeaponModel = NULL;
	m_szWeaponModelName[0] = m_szLowerAnim[0] = m_szUpperAnim[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
PlayerModelPanel::~PlayerModelPanel()
{
	if (m_hWeaponModel)
	{
		m_hWeaponModel->Release();
		m_hWeaponModel = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: The engine will automatically free the models on map change
//			Reset our pointer
//			This is not an issue anymore now that we're using CHandles
//			but left in anyway.
//-----------------------------------------------------------------------------
void PlayerModelPanel::Reset()
{
	BaseClass::Reset();

	if (m_hWeaponModel)
	{
		m_hWeaponModel->Release();
		m_hWeaponModel = NULL;
	}
	
	m_szWeaponModelName[0] = m_szLowerAnim[0] = m_szUpperAnim[0] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Set a class for a PlayerClassModelPanel. It will load the relevent
//			classinfo file and get the correct weapon + anims to show and
//			load all the models and anims
//-----------------------------------------------------------------------------
void PlayerModelPanel::SetClass(const char *pszClassname)
{
	// First get the class
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();

	if (pLocalPlayer == NULL)
		return;

	PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
	bool bReadInfo = ReadPlayerClassDataFromFileForSlot(*pFilesystem, pszClassname, &hClassInfo, g_pGameRules->GetEncryptionKey());

	if (!bReadInfo)
		return;

	const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

	if (!pClassInfo)
		return;

	// Use the last weapon as their primary
	const char *pszWeapon = FF_GetDefaultWeapon(pszClassname); //pClassInfo->m_aWeapons[pClassInfo->m_iNumWeapons - 1];

	// Now load the weapon info
	WEAPON_FILE_INFO_HANDLE hWeaponInfo;
	bReadInfo = ReadWeaponDataFromFileForSlot(*pFilesystem, VarArgs("ff_weapon_%s", pszWeapon), &hWeaponInfo, g_pGameRules->GetEncryptionKey());

	if (!bReadInfo)
		return;

	const CFFWeaponInfo *pWeaponInfo = (CFFWeaponInfo *) GetFileWeaponInfoFromHandle(hWeaponInfo);

	if (!pWeaponInfo)
		return;

	SetModel(pClassInfo->m_szModel);
	SetWeaponModel(pWeaponInfo->szWorldModel);
	SetAnimations("idle_lower", VarArgs("idle_upper_%s", pWeaponInfo->m_szAnimExtension));

	IGameResources *pGR = GameResources();

	if (pGR)
	{
		m_hModel->m_nSkin = pGR->GetTeam(pLocalPlayer->entindex()) - TEAM_BLUE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Store the anims for later and set the pose parameters and overall
//			sequence
//-----------------------------------------------------------------------------
void PlayerModelPanel::SetAnimations(const char *pszLowerAnim, const char *pszUpperAnim)
{
	Q_strncpy(m_szUpperAnim, pszUpperAnim, 127);
	Q_strncpy(m_szLowerAnim, pszLowerAnim, 127);

	m_hModel->SetSequence(m_hModel->LookupSequence(m_szLowerAnim));
	m_hModel->SetPoseParameter(0, 0.0f); // move_yaw
	m_hModel->SetPoseParameter(1, 10.0f); // body_pitch, look down a bit
	m_hModel->SetPoseParameter(2, 0.0f); // body_yaw
	m_hModel->SetPoseParameter(3, 0.0f); // move_y
	m_hModel->SetPoseParameter(4, 0.0f); // move_x
}

//-----------------------------------------------------------------------------
// Purpose: Do player layered animations each frame
//-----------------------------------------------------------------------------
void PlayerModelPanel::Paint()
{
	if (m_hModel == NULL)
		return;

	m_hModel->m_SequenceTransitioner.UpdateCurrent(
		m_hModel->GetModelPtr(), 
		m_hModel->LookupSequence(m_szLowerAnim), 
		m_hModel->GetCycle(), 
		m_hModel->GetPlaybackRate(), 
		gpGlobals->realtime);

	m_hModel->SetNumAnimOverlays(2);

	int numOverlays = m_hModel->GetNumAnimOverlays();

	for (int i = 0; i < numOverlays; i++)
	{
		C_AnimationLayer *layer = m_hModel->GetAnimOverlay(i);
		layer->m_flCycle = m_hModel->GetCycle();
		layer->m_nSequence = m_hModel->LookupSequence((i == 0 ? m_szLowerAnim : m_szUpperAnim));
		layer->m_flPlaybackRate = 1.0;
		layer->m_flWeight = 1.0f;
		layer->SetOrder(i);
	}

	m_hModel->FrameAdvance(gpGlobals->frametime);

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: Draw the weapon model as well
//-----------------------------------------------------------------------------
void PlayerModelPanel::DrawModels()
{
	BaseClass::DrawModels();

	if (m_hWeaponModel)
		m_hWeaponModel->DrawModel(STUDIO_RENDER);
}

//-----------------------------------------------------------------------------
// Purpose: Load the weapon model, make sure it is always following the
//			correct entity
//-----------------------------------------------------------------------------
void PlayerModelPanel::SetWeaponModel(const char *pszWeaponModelName)
{
	if (m_hWeaponModel == NULL || Q_strcmp(pszWeaponModelName, m_szWeaponModelName) != 0)
	{
		if (m_hWeaponModel)
			m_hWeaponModel->Remove();

		C_BaseAnimating *pWeaponModel = m_hWeaponModel.Get();

		pWeaponModel = new C_BaseAnimatingOverlay;
		pWeaponModel->InitializeAsClientEntity(pszWeaponModelName, RENDER_GROUP_OPAQUE_ENTITY);
		pWeaponModel->AddEffects(EF_NODRAW);

		m_hWeaponModel = pWeaponModel;
	}

	if (m_hWeaponModel)
		m_hWeaponModel->FollowEntity(m_hModel);
}
