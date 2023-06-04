#include <map>
#include <direct.h>
#include <iostream>
#include <filesystem>
#include <fstream>

#include "Factory.h"
#include "Objects/GameObject.h"
#include "System/Space/Space.h"

#ifdef EDITOR
#include "../ImGui/imgui.h"
#endif // EDITOR


//TEMPORAL
#include "Graphics/Renderable/Renderable.h"
#include "Graphics/Camera/Camera.h"
#include "Collisions/Collider.h"
#include "AudioManager/Audio.h"
#include "Physics/Rigidbody/Rigidbody.h"
#include "AudioManager/Audio.h"
#include "Graphics/Lighting/Light.h"
#include "Graphics/ParticleSystem/Particle.h"
#include "Graphics/TextRendering/TextRender.h"
#include "GameStateManager/MenuManager/MenuManager.h"

#include "../GamePlay/src/Player/Player.h"
#include "../GamePlay/PathFinding/NavMesh.h"
#include "../GamePlay/src/Enemies/Robot/Robot.h"
#include "../GamePlay/src/Enemies/Fly/Fly.h"
#include "../GamePlay/src/Spawner/Spawner.h"

#include "../GamePlay/src/Componets/LobbyMrg.h"
#include "../GamePlay/src/Componets/NetGameplayMrg.h"
#include "../GamePlay/src/Weapon/Weapon.h"
#include "../GamePlay/src/Bullet/Bullet.h"
#include "../GamePlay/src/Enemies/Health/HealthComp.h"
#include "../GamePlay/src/Player/PlayerCamera.h"
#include "../GamePlay/src/Health/Health.h"
#include "../GamePlay/src/UtilityComponents/FadeOut.h"
#include "../GamePlay/src/UtilityComponents/FadeInOut.h"
#include "../GamePlay/src/Enemies/DamageZones.h"
#include "../GamePlay/src/Overlay/DamageOverlay.h"
#include "../GamePlay/src/TaskSystem/TaskInfo.h"
#include "../GamePlay/src/TaskSystem/FixingTask/FixingTask.h"
#include "../GamePlay/src/TaskSystem/CommunicationTask/CommunicationTask.h"
#include "../GamePlay/src/HUD/HUDBar.h"
#include "../GamePlay/src/Player/NetworkPlayer.h"
#include "../GamePlay/src/HUD/HUDlogic.h"
#include "../GamePlay/src/Doors/Doors.h"
#include "../GamePlay/src/Interaction/InteractionComp.h"
#include "../GamePlay/src/Interaction/InteractionText.h"
#include "../GamePlay/src/VendingMachine/VendingMachine.h"
#include "../GamePlay/src/Weapon/WeaponMachine.h"
#include "../GamePlay/src/Rooms/Rooms.h"
#include "../GamePlay/src/HUD/Score.h"

using json = nlohmann::json;

static std::string levelPath = "./../Resources/Levels/";
static std::string archetypePath = "../Resources/Archetypes/";
//static std::string metaPath = "../Resources/Metadata/";
static std::string stackPath = "../Resources/temp/";


#ifdef EDITOR

static std::string editorPath = "../Resources/Levels/Editor/";
#endif // EDITOR


Factory::~Factory()
{
	for (auto creator : creators)
	{
		delete creator.second;
	}
}

const std::vector<std::string>& Factory::getRegistered() const
{
	return registeredComps;
}

void Factory::Initialize()
{
	Register<renderable>();
	Register<StaticCollider>();
	Register<DynamicCollider>();
	Register<Rigidbody>();
	Register<SoundEmitter>();
	Register<CameraComp>();
	Register<Player>();
	Register<NetworkPlayer>();
	Register<SoundListener>();
	Register<LightComponent>();
	Register<ParticleSystem>();
	Register<TextComponent>();
	Register<MenuComp>();
	Register<NavMesh>();
	Register<Robot>();
	Register<Fly>();
	Register<Spawner>();
	Register<LobbyMrg>();
	Register<NetGameplayMrg>();
	Register<Weapon>();
	Register<Bullet>();
	Register<Projectile>();
	Register<PlayerCamera>();
	Register<Health>();
	Register<DamageZonesHandler>();
	Register<FadeOut>();
	Register<FadeInOut>();
	Register<DamageOverlay>();
	Register<TaskInfo>();
	Register<FixingTask>();
	Register<CommunicationTask>();
	Register<HUDBar>();
	Register<HUDlogic>();
	Register<Door>();
	Register<InteractionComp>();
	Register<InteractionText>();
	Register<RoomSystem>();
	Register<VendingMachine>();
	Register<WeaponMachine>();
	Register<IconLogic>();
	Register<HighestScore>();
}

void Factory::AddComponent(const char* _name, ICreator* _creator) {
	if (creators.find(_name) == creators.end()) {
		creators.insert(std::pair<std::string, ICreator*>(_name, _creator));
	}
}

void Factory::LoadFromJson(nlohmann::json& j)
{
	for (auto it = j.begin(); it != j.end(); ++it)
	{
		json& compVal = *it;
		Space* sp = Scene.CreateSpace();
		compVal["Space"] >> *sp;
	}
}

void Factory::LoadFromJson(GameObject* go, nlohmann::json& j)
{
	j >> *go;
}

void Factory::SaveToJson(const std::vector<GameObject*>& _go, nlohmann::json& _j, std::string filename) const
{
	std::string path;
	path += levelPath;
	path += filename;

	for (GameObject* go : _go)
	{
		json goJson;
		goJson["GameObject"] << *go;
		_j.push_back(goJson);

	}

	serializer.Write(path, _j);
	
	std::cerr << "";
}

void Factory::SaveToJson(GameObject& go, nlohmann::json& j, std::string filename)
{
	j["GameObject"] << go;

	std::string path;
	path += levelPath;
	path += filename;

	serializer.Write(path, j);

}

void Serializer::LoadLevel(const char* _levelName) {
	
	std::string path;
	path += levelPath;
	path += _levelName;
	path += ".json";

	std::ifstream inFile(path.c_str());
	
	assert(inFile.good());
	
	json j;
	
	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		Scene.LoadLevel(j);
		inFile.close();
	
	}
}

void Serializer::LoadArchetype(const char* _archetypeName, GameObject* _go)
{
	json j;

	std::string path;
	path += archetypePath;
	path += _archetypeName;
	path += ".json";

	std::ifstream inFile(path.c_str());
	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		inFile.close();
		factory.LoadFromJson(_go, j);
	}

	_go->InitComp();
}

int Serializer::LoadStackUndo(const char* _objectName, GameObject* _go, int& _action)
{
	json j;

	std::string path;
	path += stackPath;
	path += _objectName;
	path += ".json";

	std::ifstream inFile(path.c_str());
	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		inFile.close();
		j >> *_go;
		//remove(path.c_str()); change this so it gets deleted when a change that is not ctr+z (from the current state of the object it gets a new change)
		_action = j["action"];
		//_go->mSceneSpace = j["space"].get<std::string>();
		return j["ID"];
	}
	return NULL;
}

int Serializer::LoadStackUndo(const char* _objectName, std::vector<std::pair<int, glm::vec3>>& _objects)
{
	json j;

	std::string path;
	path += stackPath;
	path += _objectName;
	path += ".json";

	std::ifstream inFile(path.c_str());
	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		inFile.close();
		for (auto k = j["Objs"].begin(); k != j["Objs"].end(); k++) {
			glm::vec3 v;
			(*k)["mPosition"] >> v;
			_objects.push_back(std::pair<int, glm::vec3>((*k)["id"], v));
		}

		//remove(path.c_str()); change this so it gets deleted when a change that is not ctr+z (from the current state of the object it gets a new change)
	}
	return NULL;
}

//void Serializer::SaveLevel(const char* _levelName, const std::vector<GameObject*>& _goVect)
//{
//	std::string path;
//	path += levelPath;
// 	path += _levelName;
//
//	json j;
//
//	std::ofstream outFile(path, std::ofstream::trunc);
//
//	if (outFile.good() && outFile.is_open()) {
//
//		for (auto obj : _goVect)
//		{
//			json goJson;
//			goJson << *obj;
//			j["GameObjects"].push_back(goJson);
//
//		}
//
//		outFile << std::setw(4) << j;
//
//		outFile.close();
//	}
//
//	std::cerr << "";
//}

void Serializer::SaveLevel(std::string level)
{
	std::string path;
	path += levelPath;
	path += level;
	path += ".json";

	json j;
	
	j["Current Level"] << std::string(Scene.GetCurrentLevel());
	Scene.SaveLevel(j);
	
	Write(path, j);

	std::cerr << "";
}

void Serializer::SaveArchetype(const char* _archetypeName, GameObject* _go)
{
	json j;

	j << *_go;

	std::string path;
	path += archetypePath;
	path += _archetypeName;
	path += ".json";

	Write(path, j);
}

void Serializer::Clipboard(GameObject* _go)
{
	json j;

	j << *_go;

	std::string path;
	path += stackPath;
	path += "clipboard.json";

	Write(path, j);
}

void Serializer::LoadClipboard(GameObject* _go)
{
	if (!_go)
		return;

	json j;

	std::string path;
	path += stackPath;
	path += "clipboard.json";

	std::ifstream inFile(path.c_str());
	if (inFile.good() && inFile.is_open()) {
		inFile >> j;
		inFile.close();
		j >> *_go;
	}
}

void Serializer::SaveStackUndo(const char* _objectName, GameObject* _go, int _id, int _action)
{
	json j;

	j << _go;
	
	j["ID"] = _id;
	j["action"] = _action;
	
	std::string path;
	path += stackPath;
	path += _objectName;
	path += ".json";
	
	Write(path, j);
}

void Serializer::SaveStackUndo(const char* _objectName, std::vector<std::pair<int, glm::vec3>>& _objects, int _action)
{
	json j;

	j["action"] = _action;

	for (std::pair<int, glm::vec3> _pair : _objects) {
		json k;
		k["id"] = _pair.first;
		k["mPosition"] << _pair.second;
		j["Objs"].push_back(k);
	}

	std::string path;
	path += stackPath;
	path += _objectName;
	path += ".json";

	Write(path, j);
}

void Serializer::Initialize()
{
#ifdef EDITOR
	std::filesystem::create_directory("../Resources/temp/");
#endif
}

void Serializer::ShutDown()
{
#ifdef EDITOR

	if (!std::filesystem::is_directory("../Resources/temp/")) return;
	if (!std::filesystem::is_empty("../Resources/temp/"))
	{
		for (auto& it : std::filesystem::directory_iterator("../Resources/temp/"))
			std::filesystem::remove(it.path());
	}

	std::filesystem::remove("../Resources/temp/");
#endif
}

void Serializer::Write(std::string& path, nlohmann::json& j)
{
	std::ofstream outFile(path.c_str());
	if (outFile.good() && outFile.is_open()) {
		outFile << std::setw(4) << j;
		outFile.close();
	}
}


void Serializer::GetLevels(std::vector<std::string>& _levels)
{
	std::string path;
	path += levelPath;
	path += "levels";
	path += ".json";

	std::ifstream inFile(path.c_str());

	assert(inFile.good());

	json j;

	if (inFile.good() && inFile.is_open()) 
	{
		inFile >> j;

		if (j.find("Levels") != j.end())
		{
			json& levels = *j.find("Levels");
			for (auto it = levels.begin(); it != levels.end() && levels.size() != 0; ++it)
			{
				std::string level;
				*it >> level;
				_levels.push_back(level);
			}
		}
	}
}

void Serializer::SaveLevels(std::vector<std::string>& _levels)
{
	std::string path;
	path += levelPath;
	path += "levels";
	path += ".json";

	json j;

	std::ofstream outFile(path, std::ofstream::trunc);

	outFile.clear();
	for (std::string& string : _levels)
		j["Levels"].push_back(string);
	outFile << std::setw(4) << j;
	outFile.close();
	std::cerr << "";
}


#ifdef EDITOR

//void AutoSerialize::Edit()
//{
//	for (auto& it : mProperties)
//	{
//		it.second->get();
//
//		//Edit(it.second->get(), it.first);
//		ImGui::Separator();
//	}
//}

bool Edit(int& variable, std::string& name)
{
	return ImGui::DragInt(name.data(), &variable);
}

bool Edit(float& variable, std::string& name)
{
	return ImGui::DragFloat(name.data(), &variable);
}

bool Edit(double& variable, std::string& name)
{
	return ImGui::DragFloat(name.data(), (float*)(&variable));
}

bool Edit(bool& variable, std::string& name)
{
	return ImGui::Checkbox(name.data(), &variable);
}

bool Edit(std::string& variable, std::string& name)
{
	return ImGui::InputText(name.data(), variable.data(), 30);
}

bool Edit(glm::vec3& variable, std::string& name)
{
	return ImGui::DragFloat3(name.data(), &(variable[0]));
}

bool Edit(glm::vec4& variable, std::string& name)
{
	return ImGui::DragFloat4(name.data(), &(variable[0]));;
}

#endif