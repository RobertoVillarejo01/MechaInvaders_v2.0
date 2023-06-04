#pragma once
#include "LogicSystem\LogicSystem.h"
#include "Serializer/Factory.h"
#include <unordered_map>

class Player;
class PlayerCamera;
class Health;
class Weapon;
class Door;
class VendingMachine;
class TaskInfo;
class FadeInOut;

class NetGameplayMrg : public ILogic
{
public:
	NetGameplayMrg();
	~NetGameplayMrg();

	void Initialize() override;	
	void Update() override;		
	void Shutdown() override;	

#ifdef EDITOR
	// Edit function to be override by child class, for ImGui
	bool Edit() override;
#endif

	//Clone function for deep copys in case is necesary
	IComp* Clone() override;

protected:
	void ToJson(nlohmann::json& j) const override;
	void FromJson(nlohmann::json& j) override;

private:

	void UpdateClients();

	void CreatePlayer(int id);

	void IsGameOverYet();

	void respawnPlayer(int dead_player_id);

	void getComponents(int id);

	std::array<GameObject*, 4>		spawnPoints{ nullptr };
	std::array<GameObject*, 4>		players{ nullptr };

	std::array<Player*, 4>			PlayerComps{ nullptr };
	std::array<PlayerCamera*, 4>	PlayerCameraComps{ nullptr };
	std::array<Health*, 4>			PlayerHealthComps{ nullptr };
	std::array<Weapon*, 4>			WeaponComps{ nullptr };
	
	std::unordered_map<unsigned int, Door*> mDoors;
	std::unordered_map<unsigned int, VendingMachine*> mVendingMachines;

	std::array< glm::vec3, 4> color{	glm::vec3 ( 250, 15, 15 ),
										glm::vec3 ( 15, 15, 250 ),
										glm::vec3 ( 15, 250, 15 ),
										glm::vec3 ( 250, 15, 150) };


	std::array< glm::vec2, 4> init_aim {	glm::vec2( 0, -90 ), 
											glm::vec2( 0, -90),
											glm::vec2( 0, -90),
											glm::vec2( 0, -90) };

	bool serverItIsI = false;
	bool not_singleplayer = false;

	//temp
	int max_clients = 4;
	bool gameOver = false;
	std::array<bool, 4> alive_player{ false };
	int my_id = 0;
	bool update_y = false;
	FadeInOut* fader_comp = nullptr;
};