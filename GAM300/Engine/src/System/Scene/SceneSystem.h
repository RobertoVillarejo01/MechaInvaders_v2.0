#pragma once

#include <map>
#include <vector>

#include "Utilities/Singleton.h"
#include "Serializer/json.hpp"
#include "System/Space/Space.h"

class GameObject;
class IComp;

enum class Tags;

class SceneSystem
{
	MAKE_SINGLETON(SceneSystem)

public:
	/*! Should only be called once in the whole game */
	void Load();

	/*! Initializes the components of the whole scene	*/
	bool Initialize();
	void Update();
	void ShutDown();
	~SceneSystem();

	GameObject* get_base_player();
	void CreateNewLevel(const char* level = "Daniel");
	void ChangeLevel(const char* level);
	void CreateDefaultSpace();
	void SaveLevel(nlohmann::json& j);
	void LoadLevel(nlohmann::json& j);

	// OBJECTS FEATURES
	void AddObject(GameObject* object, Space* space = nullptr);
	GameObject* CreateObject(GameObject* object = nullptr, Space* space = nullptr);
	void DestroyObject(GameObject* object, Space* space = nullptr);
	void ChangeObjSpace(GameObject* object, Space* space);

	//OBJECT GETTORS
	GameObject* FindObject(std::string name, Space* space = nullptr);
	GameObject* FindObject(int id, Space* space = nullptr);
	std::vector<GameObject*> FindObjects(std::string name, Space* space = nullptr);
	std::vector<GameObject*> FindObjects(Tags tag, Space* space = nullptr);

	const std::vector<GameObject*>& GetAliveObjects(Space* space = nullptr);

	//COMPONENTS FEATURES
	void AddComp(IComp* comp, Space* space = nullptr);
	void DestroyComp(IComp* comp, Space* space = nullptr);

	template <typename T>
	T* CreateComp(Space* space = nullptr, T* comp = nullptr);
	template <typename T>
	std::vector<IComp*>& GetComponentsType(Space* space = nullptr);

	//SPACE FEATURES
	void SetDefaultSpace(Space* space);
	void SetDefaultSpace(const char* name);
	void AddSpace(Space* space);
	void DestroySpace(Space* space);
	Space* CreateSpace(const char* name = "MainArea");
	Space* GetSpace(const char* name);
	Space* GetDefaultSpace() { return mDefaultSpace; }
	std::vector<Space*>& GetSpaces() { return spaces; }

	void FreeDestroyedObjects();
	std::vector<std::string>& GetLevels() { return levels; }
	std::string& GetCurrentLevel() { return mCurrentLevel; }
private:
	Space* mDefaultSpace = nullptr;
	std::vector<Space*> spaces;

	std::string mCurrentLevel = "Daniel";
	std::vector<std::string> levels;
};

#define Scene (SceneSystem::Instance())


template <typename T>
T* SceneSystem::CreateComp(Space* space, T* comp)
{
	//check if there is a space created
	if (spaces.empty())
		CreateDefaultSpace();

	if (space == nullptr)
		return mDefaultSpace->CreateComp<T>(comp);
	return space->CreateComp<T>(comp);
}

template <typename T>
std::vector<IComp*>& SceneSystem::GetComponentsType(Space* space)
{
	if (space == nullptr)
		return mDefaultSpace->GetComponentsType<T>();
	return space->GetComponentsType<T>();
}