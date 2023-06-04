#pragma once
#include "LogicSystem\LogicSystem.h"

class Player;
class NetworkPlayer;
class PlayerCamera;
class Health;

class LobbyMrg : public ILogic
{
public:
	LobbyMrg();
	~LobbyMrg();

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
	void CreatePlayer(int id);
	void getComponents(int id);

	bool serverItIsI = false;
	bool not_singleplayer = false;
	int my_id = 0;

	std::array<GameObject*, 4>	spawnPoints{ nullptr };
	std::array<GameObject*, 4>	players{ nullptr };

	std::array<Player*, 4>			PlayerComps{ nullptr };
	std::array<PlayerCamera*, 4>	PlayerCameraComps{ nullptr };
	std::array<Health*, 4>			PlayerHealthComps{ nullptr };

	std::array< glm::vec3, 4> color{	glm::vec3(250, 15, 15),
										glm::vec3(15, 15, 250),
										glm::vec3(15, 250, 15),
										glm::vec3(250, 15, 150) };


	std::array< glm::vec2, 4> init_aim{		glm::vec2(-270, -88),
											glm::vec2(-255, -93),
											glm::vec2(-280, -90),
											glm::vec2(-107, -95) };

};