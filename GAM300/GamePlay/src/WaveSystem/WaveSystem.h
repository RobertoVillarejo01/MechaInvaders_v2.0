#include "../Engine/src/Utilities/Singleton.h"
#include "Utilities/Math.h"
#include <vector>
#include <string>

class Spawner;
class GameObject;
class SoundEmitter;
class FadeInOut;
class TextComponent;

struct SpawnInfo;
struct spawn_info;

class WaveSystem
{
	MAKE_SINGLETON(WaveSystem)
public:
	~WaveSystem();

	void Initialize();
	void Update();
	void Shutdown();

	void StartWave();
	void UpdateWave();
	void EndWave();

	void UpdateStats();
	std::vector<Spawner*> ChooseSomeSpawners(std::vector<Spawner*> spawners, int amount);
	void SendOrder(std::vector<Spawner*> spawners, float time = 0, unsigned int amount = 0);
	//void RepositionEnemies();
	void ClearSpawns();
	
	void CheatCodes();

	//searching methods
	std::vector<Spawner*> ChooseNearSpawners(int amount, glm::vec3 position);
	std::vector<Spawner*> GetSpawnersFromRoom(int room);
	
	//Enemy tracking
	std::unordered_map<unsigned int, GameObject*> enemies;
	int spawned_enemies = 0;
	int defeated_enemies = 0;
	int enemies_this_round = 0;
	int max_enemies_ingame = 0;

	size_t max_num_orders = 3;
	float spawn_move_vel = 1.5f;

	int round_num = 0;

	void SendOrdePacket(spawn_info& info);
	void addEnemy(GameObject* enemy);
	unsigned int base_id = 0;
	std::vector<Spawner*> active_spawners;
	int highest_round = 0;
private:
	int order_per_iteration = 0;
	//Round counting
	unsigned int actual_round = 0;
	//Spawners
	std::vector<GameObject*> oSpawners;
	//std::vector<Spawner*> cSpawners;
	std::unordered_map<unsigned int, Spawner*> cSpawners;

	int spawn_amount = 0;
	int range = 0;
	float max_velocity = 0.0f;
	float min_velocity = 0.0f;
	float max_time_spawn = 0.0f;
	float min_time_spawn = 0.0f;
	float health = 0.0f;
	//Players
	std::vector<GameObject*> players;
	//Timers
	float timer_rounds = 0.0f;
	float time_between_rounds = 0.0f;
	float timer_spawning = 0.0f;
	float spawning_deltat = 0.0f;
	//Checkers
	bool onWave = false;

	SoundEmitter* emitter = nullptr;
	FadeInOut* letter = nullptr;

	bool serverItIsI = false;
	void WaveStartEvent();
	void WaveEndEvent();
	void SendOrdeEvent(Spawner* spawner, SpawnInfo& info);
};

#define WaveSys (WaveSystem::Instance())