#pragma once
#include "LogicSystem/Logic.h"

class Player;
class Spawner;
class TaskInfo;

struct Room
{
	Room() {}
	Room(GameObject* room) : mRoom{ room } {}

	GameObject* mRoom = nullptr;
	std::vector<Spawner*>  mSpawners;
	std::vector<TaskInfo*> mTasks;
	bool active = false;
	//rest of important things belonging to the room
};

class RoomSystem : public ILogic
{
public:
	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
	void ActivateRoom(Room room);
	void DeactivateRoom(Room room);

private:
	Player* mPlayer = nullptr;
	std::vector<GameObject*> players;
	std::unordered_map<std::string, Room> rooms;
};