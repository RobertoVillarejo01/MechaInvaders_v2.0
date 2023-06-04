#ifdef EDITOR
#include <fstream>
#include <filesystem>

#include "Editor.h"
#include "System/Scene/SceneSystem.h"
#include "Objects/GameObject.h"
#include "Utilities/Input/Input.h"
#include "BasicFunctionalities/ObjectSelection/MousePicker/MousePicker.h"
#include "../ImGui/ImGuizmo.h"

#include "Window/Window.h"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_sdl.h"
#include "../ImGui/imgui_impl_opengl3.h"
#include "resourcemanager/Resourcemanager.h"
#include "Serializer/Factory.h"
#include "GameStateManager/GameStateManager.h"
#include "GameStateManager/MenuManager/MenuManager.h"

#include "Graphics/DebugDraw/DebugDrawing.h"
#include "Graphics/Texture/Texture.h"
#include "../GamePlay/src/WaveSystem/WaveSystem.h"
#include "Undo/Undo.h"

void ImGuiManager::Initialize() const
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	const char* glsl_version = "#version 130";
	ImGui_ImplSDL2_InitForOpenGL(WindowMgr.GetHandle(), WindowMgr.GetContext());
	ImGui_ImplOpenGL3_Init(glsl_version);

}

void ImGuiManager::StartFrame() const
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(WindowMgr.GetHandle());
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
	ImGuizmo::Enable(true);
}

void ImGuiManager::EndFrame() const
{
	ImGui::EndFrame();
}

void ImGuiManager::Render() const
{
	ImGui::Render();		
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::CleanUp() const
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void Editor::Initialize()
{
	ImGuiMgr.Initialize();

	selectedObj = nullptr;
	resourceToDragDrop = "none";
	mbInEditor = true;

	mCamera.SetProjection(60.0f, WindowMgr.GetResolution(), 0.25f, 2000.0f);
	mCamera.SetPosition(mConfig.mCamPos);
	mCamera.GFX::Camera::SetTarget(mConfig.mCamTarg);
	mCamera.GFX::Camera::Update();
}

void Editor::Load()
{
	mConfig.Load();
}

void Editor::StartFrame()
{
	ImGuiMgr.StartFrame();
}

void Editor::EndFrame()
{
	ImGuiMgr.EndFrame();
}

void Editor::Update()
{	
	if (selectedObj)
	{
		if (KeyTriggered(Key::Delete))
		{
			Scene.DestroyObject(selectedObj, selectedObj->GetSpace());
			selectedObj = nullptr;
		}
		if (mbInEditor && KeyTriggered(Key::Esc))
			selectedObj = nullptr;
	}

	if (KeyTriggered(Key::F5))
		SetPause(false);
	
	if (KeyTriggered(Key::Esc))
	{
		GSM.WS_initialized = false;
		SetPause(true);
	}	
	
	
	if (KeyDown(Key::Control) && KeyDown(Key::Num1)) 
	{
		mCamera.SetUpVec({ 0.0f, 1.0f, 0.0f });
		mCamera.SetMode(EditorCamera::eCamType::Regular, mCamera.GetTarget());
	}

	if (KeyDown(Key::Control) && KeyDown(Key::Num2))
	{
		mCamera.SetUpVec({ 0.0f, 1.0f, 0.0f });
		mCamera.SetMode(EditorCamera::eCamType::FreeMovement, mCamera.GetTarget());
	}

	if (selectedObj != nullptr)
	{
		if (KeyDown(Key::Control) && KeyDown(Key::Num3))
			mCamera.SetMode(EditorCamera::eCamType::Spherical, selectedObj->mTransform.mPosition);
	}

	if (KeyDown(Key::Control) && KeyDown(Key::S))
		serializer.SaveLevel(GSM.GetCurrentLevel().c_str());

	if (KeyDown(Key::Control) && KeyDown(Key::C))
		if (selectedObj)
			Copy(selectedObj);

	if (KeyDown(Key::Control) && KeyTriggered(Key::V))
		Paste();

	//render colliders
	if (KeyTriggered(Key::F1))
		mConfig.mbCollidersVisible = !mConfig.mbCollidersVisible;

	if (KeyDown(Key::Control) && KeyTriggered(Key::Z))
		Undo::Instance().UndoChange();

	//	undo

	//the cube gizmo at the top right
	CubeGizmo();

	mCamera.Move();
	if (mConfig.mColliderPicking) PickCollider();
	else						  PickObj();
	MainBar();
	LevelTree();
	EditObj();
}

void Editor::Render()
{
	if (mConfig.mbCollidersVisible)
		RenderColliders();
	ImGuiMgr.Render();
}

void Editor::Unload()
{
	mConfig.mCamPos = mCamera.GetPosition();
	mConfig.mCamTarg = mCamera.GetTarget();
	mConfig.Save();
}

void Editor::SetPause(bool set)
{
	if (set == mbInEditor) return;
	mbInEditor = set;

	if (mbInEditor)
	{
		SDL_ShowCursor(SDL_TRUE);
		SDL_SetRelativeMouseMode(SDL_FALSE);

		mbInEditor = true;
		mbFromEditor = false;

	//	MenuMgr.SetInMenu(false);

		Scene.ShutDown();

		serializer.LoadLevel(GSM.GetCurrentLevel().c_str());

		Scene.Initialize();

		gAudioMgr.StopAll();
	}
	else
	{
		SDL_ShowCursor(SDL_FALSE);
		SDL_SetRelativeMouseMode(SDL_TRUE);

		mbInEditor = false;
		mbFromEditor = true;
		serializer.SaveLevel(GSM.GetCurrentLevel());
		Scene.ShutDown();
		serializer.LoadLevel(GSM.GetCurrentLevel().c_str());
		Scene.Initialize();
		//WaveSys.Initialize();

		selectedObj = nullptr;

	//	MenuMgr.SetInMenu(false);
	}
}

void Editor::RenderColliders()
{
	std::vector<IComp*>& colliders = Scene.GetComponentsType<StaticCollider>();

	for (unsigned i = 0; i < colliders.size(); i++)
	{
		StaticCollider* collider = dynamic_cast<StaticCollider*>(colliders[i]);
		switch (collider->mShape)
		{
			case shape::AABB:
			case shape::OBB:
			{
				geometry::obb obb;
				obb.halfSize = collider->mScale / 2.0f;
				obb.position = collider->mOwner->mTransform.mPosition + collider->mOffset;
				obb.orientation = collider->mOrientationMtx;
				Debug::DrawOBB(obb, glm::vec4(1.0f));
				break;
			}
			case shape::SPHERICAL:
			{
				geometry::sphere sphere;
				sphere.mCenter = collider->mOwner->mTransform.mPosition + collider->mOffset;
				sphere.mRadius = collider->mScale.x;
				Debug::DrawSphere(sphere, glm::vec4(1.0f));
				break;
			}
		}
	}

	std::vector<IComp*>& dyn_colliders = Scene.GetComponentsType<DynamicCollider>();

	for (unsigned i = 0; i < dyn_colliders.size(); i++)
	{
		DynamicCollider* collider = dynamic_cast<DynamicCollider*>(dyn_colliders[i]);
		switch (collider->mShape)
		{
			case shape::AABB:
			case shape::OBB:
			{
				geometry::obb obb;
				obb.halfSize = collider->mScale / 2.0f;
				obb.position = collider->mOwner->mTransform.mPosition + collider->mOffset;
				obb.orientation = collider->mOrientationMtx;
				Debug::DrawOBB(obb, glm::vec4(1.0f));
				break;
			}
			case shape::SPHERICAL:
			{
				geometry::sphere sphere;
				sphere.mCenter = collider->mOwner->mTransform.mPosition + collider->mOffset;
				sphere.mRadius = collider->mScale.x;
				Debug::DrawSphere(sphere, glm::vec4(1.0f));
				break;
			}
		}
	}
	
}

void Editor::CubeGizmo()
{

	//cubito stuf
	auto& io = ImGui::GetIO();
	auto W2C = mCamera.GetW2Cam();
	ImGuizmo::ViewManipulate(&W2C[0][0], 10.0f, ImVec2(io.DisplaySize.x - 128, 0), ImVec2(128, 128), 0x10101010);
	
	mCamera.SetW2Cam(W2C, selectedObj);
}

#endif 

void EditorConfig::Load()
{
	std::ifstream inFile(mSavePath.c_str());

	assert(inFile.good());

	json j;

	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		operator>>(j);
		inFile.close();
	}
}

void EditorConfig::Save()
{
	json j;
	operator<<(j);

	std::ofstream outFile(mSavePath.c_str());
	if (outFile.good() && outFile.is_open()) {
		outFile << std::setw(4) << j;
		outFile.close();
	}
}

nlohmann::json& EditorConfig::operator<<(nlohmann::json& j) const
{
	j["System Configuration"]["Camera Position"] << mCamPos;
	j["System Configuration"]["Camera Target"] << mCamTarg;
	j["System Configuration"]["Colliders Visible"] << mbCollidersVisible;
	j["System Configuration"]["Lighting Mode"] << static_cast<int>(mLightConfig);

	return j;
}

void EditorConfig::operator>>(nlohmann::json& j)
{
	int config;
	j["System Configuration"]["Camera Position"] >> mCamPos;
	j["System Configuration"]["Camera Target"] >> mCamTarg;
	j["System Configuration"]["Colliders Visible"] >> mbCollidersVisible;
	j["System Configuration"]["Lighting Mode"] >> config;

	switch (config)
	{
	case 0:
		mLightConfig = GFX::LConfig::Diffuse;
		break;
	case 1:
		mLightConfig = GFX::LConfig::LightComponent;
		break;
	case 2:
		mLightConfig = GFX::LConfig::LightComponentShadows;
		break;
	default:
		mLightConfig = GFX::LConfig::Diffuse;
		break;
	}
}
