#pragma once

#ifdef EDITOR

#include "Utilities/Singleton.h"
#include "Camera/EditorCamera.hpp"
#include "Graphics/GLEnums.h"
#include "Graphics/Texture/Texture.h"
#include "Collisions/Collider.h"
#include <string>

struct EditorConfig : public ISerializable
{
	glm::vec3 mCamPos{};
	glm::vec3 mCamTarg{};
	GFX::LConfig mLightConfig = {};
	bool mbCollidersVisible = false;
	bool mColliderPicking = false;
	bool mShowItemList = false;
	glm::vec2 mCurrentMousePos{};
	std::vector<GameObject*> mCandidateObjects;

	const std::string mSavePath = "./../Editor/config.json";

	void Load();
	void Save();

	nlohmann::json& operator<<(nlohmann::json& j) const;
	void            operator>>(nlohmann::json& j);

};

class Editor
{
	MAKE_SINGLETON(Editor)
public:

	// Base functions for editor
	void Initialize();
	void Load();
	void StartFrame();
	void EndFrame();
	void Update();
	void Render();
	void Unload();
	void SetPause(bool set);
	
	//windows in screen
	void MainBar();
	void CreateArchetypes();
	void CreateLevel();
	void GeneralSettings();
	void LevelTree();
	void CreateSpace();
	void DisplayMemory();
	void EditObj();
	void EditLevel();

	//functions for editing
	void RenderColliders();
	void MultipleSelection(const glm::vec2& initialPos, const glm::vec2& finalPos);
	void SelectObjects();
	void ItemList(const std::vector<GameObject*>& objects);
	void PickObj();
	void RenderPreviewObject(GameObject* _obj, const glm::vec4& _color);
	void PickCollider();
	void Copy(GameObject* obj);
	GameObject* Paste();
	void CubeGizmo();

	// Basic properties
	char newLevelName[50] = {};
	bool mbInEditor = true;
	bool mbFromEditor = true;
	bool MemoryOpen = false;
	bool new_space = false;

	EditorConfig mConfig;
	EditorCamera mCamera;
	GameObject* selectedObj = nullptr;
	glm::vec2* startingMousePos = nullptr;
	glm::vec2* endMousePos = nullptr;
	bool mDraggingMouse = false;
	bool IsDragging = false;
	bool mMultipleEdition = false;
	std::string resourceToDragDrop = "none";
	std::vector<GameObject*> mSelectedObjects;
	std::vector<Collider*> mCollidersInScene;

	// Lighting configuration
	GFX::CubemapRes mCubemap = nullptr;


};
#define EditorMgr (Editor::Instance())

class ImGuiManager 
{
	MAKE_SINGLETON(ImGuiManager)

public:
	void Initialize() const;
	void StartFrame() const;
	void EndFrame() const;
	void Render() const;
	void CleanUp() const;

private:
	// Map of windows
	// Component Creators?
};

#define ImGuiMgr (ImGuiManager::Instance())

#endif 
