#pragma once
#include "Objects/Components.h"
#include "Utilities/Singleton.h"

#include <unordered_map>

class GameClass
{
	MAKE_SINGLETON(GameClass);
public:
	void Initialize();
	void Update();
	void ShutDown();
	~GameClass() {}

#ifdef EDITOR
	bool Edit();
#endif // EDITOR


	GameObject* GetBasePlayer() { return base_player; }
	std::vector<GameObject*>& GetPlayers() { return players; }


private:

	GameObject* base_player;
	std::vector<GameObject*> players;
	std::unordered_map<std::string, GameObject*> rooms;
};

#define Game (GameClass::Instance())