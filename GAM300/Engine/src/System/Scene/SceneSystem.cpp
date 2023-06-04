#include <filesystem>
#include <fstream>
#include "SceneSystem.h"
#include "System/Space/Space.h"
#include "Objects/GameObject.h"
#include "Objects/Components.h"
#include "Serializer/Factory.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "Graphics/RenderManager/RenderManager.h"
#include "GameStateManager/GameStateManager.h"
#include "../GamePlay/src/Player/Player.h"

#ifdef EDITOR
#include "../Editor/src/Editor.h"
#endif // EDITOR

void SceneSystem::Load()
{
}

bool SceneSystem::Initialize()
{
	// It should be a nullptr regardless, but just to make sure
	Debug::DebugRenderer::Instance().SetCamera(nullptr);

	std::for_each(spaces.begin(), spaces.end(),
		[](Space* space)
		{
			space->Initialize();
		});

	// If we are on editor mode, the camera should be the one from the Editor, otherwise the 
#ifdef EDITOR
	if (EditorMgr.mbInEditor)
		Debug::DebugRenderer::Instance().SetCamera(&EditorMgr.mCamera);
#endif

	return true;
}

void SceneSystem::Update()
{
	std::for_each(spaces.begin(), spaces.end(),
		[](Space* space)
		{
			space->Update();
		});
}

void SceneSystem::ShutDown()
{
	while (!spaces.empty())
	{
		Space* temp = spaces.back();
		spaces.pop_back();

		temp->ShutDown();
		//CHANGE ONCE THE MEMORY MANAGER IS DONE//
		MemoryMgr.DestroyEntity(temp);
	}
	mDefaultSpace = nullptr;

	GFX::RenderManager::Instance().Free();
	Debug::DebugRenderer::Instance().Clear();
}

void SceneSystem::CreateDefaultSpace()
{
	if (!spaces.empty()) return;
	CreateSpace("MainArea");
}

void SceneSystem::CreateNewLevel(const char* level)
{
	auto it = std::find(levels.begin(), levels.end(), std::string(level));
	if (it != levels.end()) return;

	if (!spaces.empty())
	{
		serializer.SaveLevel(GSM.GetCurrentLevel());
		ShutDown();
	}

	mCurrentLevel = level;
	levels.push_back(mCurrentLevel);
	CreateSpace("MainArea");
}

void SceneSystem::ChangeLevel(const char* level)
{
	if (level == mCurrentLevel) return;

	serializer.SaveLevel(GSM.GetCurrentLevel());
	ShutDown();

	mCurrentLevel = level;
	serializer.LoadLevel(level);
	if (spaces.empty())
		CreateDefaultSpace();
	else
		mDefaultSpace = spaces.front();
}

SceneSystem::~SceneSystem()
{
}

void SceneSystem::AddObject(GameObject* object, Space* space)
{
	if (space == nullptr)	mDefaultSpace->AddObject(object);
	else                    space->AddObject(object);
}

GameObject* SceneSystem::CreateObject(GameObject* object, Space* space)
{
	//check if there is a space created
	if (spaces.empty())
		CreateDefaultSpace();

	if (space == nullptr)	
		return mDefaultSpace->CreateObject(object);
	return space->CreateObject(object);
}

void SceneSystem::DestroyObject(GameObject* object, Space* space)
{
	if (space == nullptr)	mDefaultSpace->DestroyObject(object);
	else                    space->DestroyObject(object);
}

void SceneSystem::FreeDestroyedObjects()
{
	for (Space* space : spaces)
		space->FreeDestroyedObjects();
}


//-------------------GETTORS OF THE OBJECTS
GameObject* SceneSystem::FindObject(std::string name, Space* space)
{
	if(space) return space->FindObject(name);
	GameObject* result = nullptr;
	for (Space* sp : spaces)
	{
		result = sp->FindObject(name);
		if (result)
			return result;
	}
	return result;
}

GameObject* SceneSystem::FindObject(int id, Space* space)
{
	if (space) return space->FindObject(id);
	GameObject* result = nullptr;
	for (Space* sp : spaces)
	{
		result = sp->FindObject(id);
		if (result)
			return result;
	}
	return result;
}

std::vector<GameObject*> SceneSystem::FindObjects(std::string name, Space* space)
{
	if (space) return space->FindObjects(name);

	std::vector<GameObject*> result;
	for (Space* sp : spaces)
	{
		result = sp->FindObjects(name);
		if (result.size())
			return result;
	}
	return result;
}

std::vector<GameObject*> SceneSystem::FindObjects(Tags tag, Space* space)
{
	if (space) return space->FindObjects(tag);
	std::vector<GameObject*> result;
	for (Space* sp : spaces)
	{
		result = sp->FindObjects(tag);
		if (result.size())
			return result;
	}
	return result;
}

const std::vector<GameObject*>& SceneSystem::GetAliveObjects(Space* space)
{
	if (space == nullptr)
		return mDefaultSpace->GetAliveObjects();
	return space->GetAliveObjects();
}

void SceneSystem::AddComp(IComp* comp, Space* space)
{
	if (space == nullptr)	mDefaultSpace->AddComp(comp);
	else                    space->AddComp(comp);
}

void SceneSystem::DestroyComp(IComp* comp, Space* space)
{
	if (space == nullptr)	mDefaultSpace->DestroyComp(comp);
	else                    space->DestroyComp(comp);
}

//SPACE FEATURES
void SceneSystem::AddSpace(Space* space)
{
	if (space == nullptr) return;
	spaces.push_back(space);
}

void SceneSystem::DestroySpace(Space* space)
{
	if (space == nullptr) return;
	auto it = std::find(spaces.begin(), spaces.end(), space);
	if (it != spaces.end())
	{
		spaces.erase(it, it + 1);

		//destroy the gameobject
		space->ShutDown();

		if (space == mDefaultSpace) mDefaultSpace = spaces.empty() ? nullptr : spaces.front();
		//CHANGE ONCE THE MEMORY MANAGER IS DONE//
		MemoryMgr.DestroyEntity(space);
	}
}

Space* SceneSystem::CreateSpace(const char* name)
{
	//CHANGE ONCE THE MEMORY MANAGER IS DONE//
	Space* new_space = MemoryMgr.CreateEntity<Space>();
	new_space->SetSpaceName(name);

	spaces.push_back(new_space);
	if (mDefaultSpace == nullptr)
		SetDefaultSpace(new_space);
	return new_space;
}

Space* SceneSystem::GetSpace(const char* name)
{
	for (Space* space : spaces)
		if (space->GetSpaceName() == name)
			return space;
	return nullptr;
}

void SceneSystem::SetDefaultSpace(Space* space)
{
	if (space == nullptr) return;
	mDefaultSpace = space;
}

void SceneSystem::SetDefaultSpace(const char* name)
{
	for (Space* space : spaces)
		if (space->GetSpaceName() == name)
		{
			mDefaultSpace = space;
			break;
		}
}


void SceneSystem::ChangeObjSpace(GameObject* object, Space* space)
{
	if (space == nullptr)
		return;

	object->GetSpace()->RemoveObject(object);
	space->AddObject(object);
}

void SceneSystem::SaveLevel(nlohmann::json& j)
{
	for (auto _space : spaces)
	{
		json jTemp;
		jTemp << *_space;
		j["Spaces"].push_back(jTemp);
	}
}

void SceneSystem::LoadLevel(nlohmann::json& j)
{
	j["Current Level"] >> mCurrentLevel;

	if (j.find("Spaces") != j.end())
	{
		json& _spaces = *j.find("Spaces");

		for (auto it = _spaces.begin(); it != _spaces.end() && _spaces.size() != 0; ++it)
		{
			json& compVal = *it;
			Space* sp = Scene.CreateSpace();
			compVal >> *sp;
		}
	}
}

GameObject* SceneSystem::get_base_player()
{
	std::vector<GameObject*> players = FindObjects(Tags::Player);

	for (GameObject* player : players)
	{
		auto player_logic = player->GetComponentType<Player>();

		if (player_logic && player_logic->base_player)
			return player;
	}

	std::cerr << "Could not find base Player in function get_base_player. Returns nullptr" << std::endl;
	return nullptr;
}