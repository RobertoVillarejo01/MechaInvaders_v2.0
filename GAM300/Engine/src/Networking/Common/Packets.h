#pragma once
#include "glm\glm.hpp"

const int max_clients = 4;

enum class Flag : unsigned char {
	NONE,
	SYN,
	FIN,
	e_move_input,
	e_new_client,
	e_network_event,
	e_status_report,
	e_enemy_report
};

struct Packet {
	unsigned short seq;
	unsigned short ack;
	Flag flag;
	//packet_type type;
	char msg[1024 - sizeof(seq) - sizeof(ack) - sizeof(flag)];
};

class Ibase_packet
{
public:
	unsigned short num;
	unsigned short id;
};

class new_client : public Ibase_packet {
public:
	unsigned int rand_seed;
	unsigned short new_id;	//new conection id
	//host
	std::array<bool, max_clients> connected_clients;
};

class move_input : public Ibase_packet {
public:
	enum player_info {
		empty = 1 << 0,
		forward = 1 << 1,
		back = 1 << 2,
		left = 1 << 3,
		right = 1 << 4,
		space = 1 << 5,
		shoot = 1 << 6,
		sprint = 1 << 7,
		interact = 1 << 8,
		reload = 1 << 9
	} info = empty;

	glm::vec3 pos{};
	glm::vec2 angle{};
};

class status_report : public Ibase_packet {
public:
	struct PlayerState
	{
		glm::vec3 position{};
		//glm::vec2 velocity;
		glm::vec2 aim_angle{};
		float current_health = 0.0f;

		///maybe bitset info
		bool jumping = false;
		bool sprinting = false;

		unsigned int money = 0;
		// unsigned int bullet_count;
		// unsigned int total_bullets;
	};

	std::array<PlayerState, max_clients> state;
};

class enemy_report : public Ibase_packet {
public:
	struct enemy {
		unsigned int enemy_id;
		//int state; //state machine
		unsigned int following_player_id;
		glm::vec3 pos;
		float health;
	};

	unsigned int enemy_count;
	std::array<enemy, 25> state;
};

class chat_message : public Ibase_packet {
public:
	std::array<char, 15> nickname;
	bool in_a_private_chat = false;
	int private_chat_other_id = -1;
	std::array<char, 512> text_channel;
};

//===================================================================

enum class event_type { none, level, shoot, interact, player_dead, respawn, game_over, pause,
						start_wave, end_wave, spawner_orde, start_task, end_task, 
						task_interact_request, task_interact, 
						door_movement, door_movement_request, 
						vending, vending_reqest,
						weapon_new_request, weapon_new, weapon_change_request, weapon_change
						};

class network_event : public Ibase_packet {
public:
	event_type type;

	std::array<char, 512 / 2> payload;
};

//=================== EVENTS ==================================

struct change_level {
	int level;
};

struct shoot {
	glm::vec3 position;
	glm::vec2 aim_angle;
};

struct spawn_info
{
	int spawner_id; //where to add the enemy

	//SpawnInfo
	int enemy_type;
	float health;
	float velocity;

	int amount;
	float time;
};

struct door_movement {
	int door_id;
	bool state; //closed or open
	bool temp; //temporal door movement
};

struct vending_machine {
	int machine_id;
	int player_id;
};

struct task_activation {
	unsigned task_tag;
	std::array<int, 3> task_id;
};

struct task_interaction {
	int task_id;
	bool state; //closed or open
};

struct pause_menu {
	bool enable;
};

struct cheat_code {
	int level;
};

struct buy_weapon {
	std::array<char, 15> name;
	unsigned int weapon_cost;
};
