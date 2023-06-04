#pragma once

#include "LogicSystem/Logic.h"
class NavMesh;
class SoundEmitter;

class Robot : public SMComponent<Robot>
{
public:
	enum SMTypes { MOVE_PATHFINDING, DIE_CHECK, TOTAL_SM};

	void StateMachineInitialize() override;

	// Public Wrapper aroud protected method for testing purposes
	void ChangeBrainSize(size_t _size) { SetBrainSize(_size); }

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	//stun public
	void StunEnemy();

	void EndUpdate() override;

	void setFollowingPlayerId(unsigned int id);
	unsigned int getFollowingPlayerId();

#ifdef EDITOR
	bool Edit();
#endif

	unsigned int getEnemyID() { return enemy_id; }
	void setEnemyID(unsigned int _id) { enemy_id = _id; }
	void SetVelocity(float _velocity);

	bool spawned = false;
	bool farmeable = true;
private:
	Rigidbody* rb;
	bool on_ramp = false;
	GameObject* PlayerPtr = nullptr;
	GameObject* NavMeshHolder = nullptr;
	NavMesh*	NavMeshC = nullptr;
	DynamicCollider* mCol = nullptr;
	SoundEmitter* mEmmitter = nullptr;

	glm::vec3 retrackPos;

	unsigned FramesBeforeNodeChange = 30u;
	unsigned FramesBeforeRayCheck = 30u;

	unsigned nodeToGoIdx = -1;
	unsigned nodeToGo = -1;
	unsigned previousNode = -1;
	unsigned NodeBeforeStun = -1; //for choosing the correct node after stun
	
	unsigned retrackPrev = -1;
	unsigned retrackNext = -1;

	unsigned RoomIndex = -1;	//useless
	int		 stunAnimation = 0; //stun anim

	float velocity = 3.0f;
	float range;
	float toSimpleRange = 40.0f; //range to change from pathfinding to simple
	float cooldown;
	float timer;

	glm::vec3 _1stNode;
	bool dead;
	std::vector<glm::vec3> PathPositions;
	std::vector<unsigned> PathIndices;

	//spawning
	void SpawnInit();
	void SpawnUpdate();
	void SpawnEnd();
	glm::vec3 old_grav = {};

	bool CheckIfInRange(glm::vec3 mPos, glm::vec3 dest, float range);
	bool TooMuchAltitude(glm::vec3 playerPos);

	//slopes
	void OnCollisionStarted(const Collider* coll);
	void OnCollisionPersisted(const Collider* coll);
	void OnCollisionEnded(const Collider* coll);


	//Simple enemy following
	void SimpleMoveStart();
	void SimpleMoveUpdate();
	void SimpleMoveEnd();

	//before starting with everything, move to closest node. States
	void MoveToFirstNodeStart();
	void MoveToFirstNodeUpdate();
	void MoveToFirstNodeEnd();

	// Combat States
	void MoveStart();
	void MoveUpdate();
	void MoveEnd();

	//Move when player movess
	void RetrackStart();
	void RetrackUpdate();
	void RetrackEnd();

	//makes the robot move to next node
	void FollowPath();
	//runs the algorithm
	void UpdatePath();
	//finds closest node from player to follow
	unsigned FindClosestPlayer();
	unsigned FindClosestNode(glm::vec3 * positionToFill = nullptr);
	void FindNavMeshHolder();
	
	void ChargeInit();
	void ChargeUpdate();

	void AttackInit();
	void AttackUpdate();

	void SpinInit();
	void SpinUpdate();

	void RepositionInit();
	void RepositionUpdate();

	void IdleInit();
	void IdleUpdate();
	bool followingAPlayer();

	void StunInit();
	void StunUpdate();

	void DieInit();
	void DieUpdate();

	void CheckAltitude();
	bool CheckAliveTime(); //to kill when a lot of time active

	//Return true if not hitting wall, return false if hitting
	bool CheckWall();

	//damage to player
	float attackRange = 25.0f;
	float chargeRange = 250.0f;
	float chargeVel = 1.0f;
	float MaxAltitude = 10.0f;

	//timer to kill
	float killtimer = 0.0f;
	float maxTimeAllive = 120.0f; //max time to kill
	//atack cooldown
	float attackCooldowTimer = 0.0f;
	float attackCooldown = 2.0f;
	//timer for the attack animation
	float attackTimer = 0.0f; 
	float attackTime = 0.0f;;
	float attackAnticipationAngle = 0.0f;
	float spinAngle = 0.0f;
	float spinSpeed = 20.0f;

	float stunAngle = 0.0f;
	float stunTimer = 0.0f;
	float maxStunTime = 0.2f;

	float dieAngle = 0.0f;
	float DieTimer = 0.0f;
	float DieTime = 0.5f;

	glm::vec3 forwardBeforeAnticipation{};
	glm::vec3 StunRotationVec{};

	float targetAngle;

	void GetClosestPlayer();
	std::vector<GameObject*> players;
	unsigned int following_player_id;
	bool serverItIsI = false;

	unsigned int enemy_id;
};