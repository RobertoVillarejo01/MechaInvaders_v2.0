#include <filesystem>
#include "../Editor/src/Editor.h"
#include "resourcemanager/Resourcemanager.h"
#include "Serializer/Factory.h"
#include "GameStateManager/GameStateManager.h"
#include "Undo/Undo.h"
#include "Window/Window.h"
#include "Graphics/TextRendering/TextRender.h"

void Editor::MainBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		GeneralSettings();
		CreateLevel();
		CreateArchetypes();

		bool prev = mbInEditor;

		ImGui::Selectable("Pause", &mbInEditor, 0, ImVec2(35, 15));

		if (prev != mbInEditor)
			SetPause(mbInEditor);

		if (ImGui::BeginMenu("Resources"))
		{
			ResourceMgr.Edit();
			ImGui::EndMenu();
		}

		ImGui::Selectable("Memory", &MemoryOpen, 0, ImVec2(45, 15));
		
		if(MemoryOpen)
			DisplayMemory();

		if (ImGui::BeginMenu("Resolution"))
		{
			if (ImGui::Button("Fullscreen Toggle")) WindowMgr.ToggleFullScreen();
			ImGui::Separator();

			if (ImGui::Button("960x540"))	WindowMgr.ChangeResolution({  960,  540 });
			if (ImGui::Button("1280x720"))	WindowMgr.ChangeResolution({ 1280,  720 });
			if (ImGui::Button("1600x900"))	WindowMgr.ChangeResolution({ 1600,  900 });
			if (ImGui::Button("1920x1080")) WindowMgr.ChangeResolution({ 1920, 1080 });

			ImGui::EndMenu();
		}

		static bool edit_messages = false;
		if (ImGui::Button("Messages"))
		{
			edit_messages = !edit_messages;
		}
		if (edit_messages) TranslationMgr.Edit();

		std::string cam = "Current camera mode: ";
		switch (mCamera.GetMode())
		{
		case EditorCamera::eCamType::FreeMovement:
			cam += "Free movement";
			break;
		case EditorCamera::eCamType::Spherical:
			cam += "Spherical";
			break;
		case EditorCamera::eCamType::Regular:
			cam += "Regular";
			break;
		}
		ImGui::Text(cam.c_str());

		ImGui::EndMainMenuBar();
	}
}

void Editor::CreateArchetypes()
{
	if (ImGui::BeginMenu("Archetypes"))
	{
		for (auto& fs : std::filesystem::directory_iterator("../Resources/Archetypes/"))
		{
			std::string name = fs.path().filename().string();
			auto json = name.find(".json");
			if (json != std::string::npos) name.erase(json);
			std::string button = "Create " + name;
			if (ImGui::MenuItem(button.c_str()))
			{
				GameObject* arch = Scene.CreateObject();
				serializer.LoadArchetype(name.c_str(), arch);
			}
		}
		ImGui::EndMenu();
	}
}

void Editor::CreateLevel()
{
	if (ImGui::BeginMenu("Levels"))
	{
		//show level window
		if (ImGui::BeginMenu("Open Level"))
		{
			for (auto& level : GSM.GetLevels())
				if (ImGui::MenuItem(level.c_str()))
				{
					selectedObj = nullptr;
					GSM.SetNextLevel(level);
					Undo::Instance().Clear();
				}

			ImGui::EndMenu();

		}
		if (ImGui::BeginMenu("New Level"))
		{
			ImGui::InputText("name", &newLevelName[0], 50);
			if (ImGui::Button("Create"))
			{
				Scene.CreateNewLevel(newLevelName);
				selectedObj = nullptr;
				Undo::Instance().Clear();
			}
			ImGui::EndMenu();
		}
		if (ImGui::Button("Save"))
		{
			serializer.SaveLevel(GSM.GetCurrentLevel());
		}
		ImGui::EndMenu();
	}
}

void Editor::GeneralSettings()
{
	if (ImGui::BeginMenu("EditorSettings"))
	{
		if (ImGui::BeginMenu("Lighting"))
		{
			if (ImGui::Selectable("Diffuse", mConfig.mLightConfig == GFX::LConfig::Diffuse))
				mConfig.mLightConfig = GFX::LConfig::Diffuse;
			if (ImGui::Selectable("LightComponent", mConfig.mLightConfig == GFX::LConfig::LightComponent))
				mConfig.mLightConfig = GFX::LConfig::LightComponent;
			if (ImGui::Selectable("Shadows", mConfig.mLightConfig == GFX::LConfig::LightComponentShadows))
				mConfig.mLightConfig = GFX::LConfig::LightComponentShadows;

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Cubemap"))
		{
			// Default case
			if (ImGui::Selectable("** NONE **", nullptr == mCubemap)) {
				mCubemap = nullptr;
			}

			auto& cubemaps = ResourceMgr.GetResourcesOfType<GFX::Cubemap>();
			for (auto& map : cubemaps)
			{
				GFX::CubemapRes casted_map = std::reinterpret_pointer_cast<TResource<GFX::Cubemap>>(map.second);
				std::string filename = map.first.substr(map.first.find_last_of('/') + 1);
				if (ImGui::Selectable(filename.data(), casted_map == mCubemap))
					mCubemap = casted_map;
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Camera mode"))
		{
			if (ImGui::Button("Regular Camera"))
				mCamera.SetMode(EditorCamera::eCamType::Regular, mCamera.GetTarget());
			if (ImGui::Button("Free Camera"))
				mCamera.SetMode(EditorCamera::eCamType::FreeMovement, mCamera.GetTarget());
			if (ImGui::Button("Spherical Camera"))
				mCamera.SetMode(EditorCamera::eCamType::Spherical, mCamera.GetTarget());
			if (ImGui::Button("Move Camera to scene center"))
				mCamera.Reset();
			ImGui::EndMenu();
		}
		ImGui::Checkbox("Show all colliders", &mConfig.mbCollidersVisible);
		ImGui::Checkbox("Collider picking", &mConfig.mColliderPicking);

		

		ImGui::EndMenu();
	}
}
