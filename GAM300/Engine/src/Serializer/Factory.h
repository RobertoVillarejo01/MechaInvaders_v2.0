#pragma once
#include <utility>
#include <vector>

#include "Utilities/Singleton.h"
#include "json.hpp"
#include "System/Scene/SceneSystem.h"

class IBase;
class IComp;
class GameObject;


class ICreator
{
public:
	virtual IComp* Create(Space* space) = 0;
};

template <typename T>
class TCreator : public ICreator
{
public:
	virtual IComp* Create(Space* space) {
		return space->CreateComp<T>();
	}
};

class Factory 
{
	MAKE_SINGLETON(Factory)

private:
	std::map<std::string, ICreator*> creators;
	std::vector<std::string> registeredComps;

public:
	~Factory();

	const std::vector<std::string>& getRegistered() const;
	void Initialize();
	void Register(const char* typeName, ICreator* creator)
	{
		if (creators.find(typeName) == creators.end())
		{
			creators[typeName] = creator;
			registeredComps.push_back(typeName);
		}
	}
	IComp* Create(const char* typeName, Space* space)
	{
		// IMPORTANT: FIND THE CREATOR HERE
		if (creators.find(typeName) != creators.end())
			return creators[typeName]->Create(space);
		// NO CREATOR REGISTERED
		return nullptr;
	}

	template <typename T> void Register() {
		Register(TypeInfo(typeid(T)).get_name().c_str(), new TCreator<T>());
	}


	template <typename T> T* Create() {
		return dynamic_cast<T*>(Create(TypeInfo(typeid(T)).get_name()));
	}

	void AddComponent(const char* typeName, ICreator* c);

	void LoadFromJson(nlohmann::json& j);

	void LoadFromJson(GameObject* go, nlohmann::json& j);

	void SaveToJson(const std::vector<GameObject*>& _go, nlohmann::json& _j, 
		std::string filename = "Testing.json") const;

	void SaveToJson(GameObject& go, nlohmann::json& j, std::string filename = "Testing.json");

};

class Serializer 
{
	MAKE_SINGLETON(Serializer)

public:
	void LoadLevel(const char* _levelName);
	void LoadArchetype(const char* _archetypeName, GameObject* _go);
	//void LoadMeta(const char* _metaName, GameObject* _go);
	int LoadStackUndo(const char* _objectName, GameObject* _go, int& _action);
	int LoadStackUndo(const char* _objectName, std::vector<std::pair<int, glm::vec3>>& _objects);
	//int LoadStackUndo(const char* _levelName, std::vector<GameObject*>& _goVect);

	void GetLevels(std::vector<std::string>& _levels);
	void SaveLevels(std::vector<std::string>& _levels);
	void SaveLevel(std::string level);

	void SaveArchetype(const char* _archetypeName, GameObject* _go);
	void Clipboard(GameObject* _go);
	void LoadClipboard(GameObject* _go);
	
	//void SaveMeta(const char* _metaName, GameObject* _go);
	void SaveStackUndo(const char* _objectName, GameObject* _go, int _id, int _action);
	void SaveStackUndo(const char* _objectName, std::vector<std::pair<int, glm::vec3>>& _objects, int _action);
	//void SaveStackUndo(const char* _levelName, std::vector<GameObject*>& _goVect, int _action);

	void Initialize();
	void ShutDown();
	void Write(std::string& path, nlohmann::json& j);
};

#define factory (Factory::Instance())
#define serializer (Serializer::Instance())
