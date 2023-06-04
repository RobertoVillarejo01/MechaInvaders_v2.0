#pragma once

#include "LogicSystem/Logic.h"
#include <queue>

class WaveSystem;

struct EnemyStats
{
	EnemyStats(float _health = 10.0f, float _velocity = 10) :
		health(_health), velocity(_velocity) {}

	float health;
	float velocity;
};

struct SpawnInfo
{
	SpawnInfo(std::string _name = std::string("Robot"), EnemyStats _stats = EnemyStats(), int _spawn_num = 1, float _time = 0.0f)
		: name(_name), spawn_num(_spawn_num), time(_time) {
		stats = _stats;
	}

	std::string name;
	EnemyStats stats;
	int spawn_num;
	float time;
};

class Spawner : public ILogic
{
public:
	void Initialize();
	void Update();

	void CreateOrder(SpawnInfo info);
	void PrepareOrders();
	void CreateEnemies(SpawnInfo info);
	int GetRoom() { return room; }

	void ClearQueues();
	void ClearWSQueue();
	void ClearTQueue();
	size_t GetWSQueueSize() { return WS_Queue.size(); }
	size_t GetTQueueSize() { return T_Queue.size(); }

	IComp* Clone();
	void Activate();
	void Deactivate();


#ifdef EDITOR
	bool   Edit();
#endif

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	bool activated = false;
	bool task = false;
	bool onTask = false;

	int getID() { return id; }
	void setID(int _id) { id = _id; }

private:
	void MoveEnemies(std::vector<GameObject*> e);

	SpawnInfo actual_info;
	std::queue<SpawnInfo> WS_Queue;
	std::queue<SpawnInfo> T_Queue;


	glm::vec3 robot_begin1 = {};
	glm::vec3 robot_begin2 = {};
	glm::vec3 robot_begin3 = {};

	glm::vec3 robot_end1 = {};
	glm::vec3 robot_end2 = {};
	glm::vec3 robot_end3 = {};

	float lerp_dt = 0;

	float velocityIncrease = 10.0f;
	float timer = 0.0f;
	bool creating = false;

	bool enemies_to_move = false;
	std::vector<GameObject*> spawned_enemies;

	int room = 0;
	int id = 0;
};