#include "Robot.h"
#include "Robot.h"
#include "../DamageZones.h"
#include "../PathFinding/NavMesh.h"
#include "WaveSystem/WaveSystem.h"
#include "Engine.h"
#include "Player\Player.h"
#include "AudioManager/Audio.h"


#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"

#endif // EDITOR

#include <limits>       // std::numeric_limits

//Im crazy I didnt know how to use the lerp function
glm::vec3 mlerp(glm::vec3 x, glm::vec3 y, float t) {
	return x * (1.f - t) + y * t;
}


void Robot::StateMachineInitialize()
{
	SetBrainSize(SMTypes::TOTAL_SM);

	AddState(&Robot::SpawnInit, &Robot::SpawnUpdate, &Robot::SpawnEnd, "Spawn", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::MoveStart, &Robot::MoveUpdate, &Robot::MoveEnd, "Move", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::SimpleMoveStart, &Robot::SimpleMoveUpdate, &Robot::SimpleMoveEnd,"MoveSimple",SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::MoveToFirstNodeStart, &Robot::MoveToFirstNodeUpdate, &Robot::MoveToFirstNodeEnd, "MoveToFirst", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::RetrackStart, &Robot::RetrackUpdate, &Robot::RetrackEnd, "Retrack", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::AttackInit, &Robot::AttackUpdate, nullptr, "Attack", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::SpinInit, &Robot::SpinUpdate, nullptr, "Spin", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::StunInit, &Robot::StunUpdate, nullptr, "Stun", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::DieInit, &Robot::DieUpdate, nullptr, "Die", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::ChargeInit, &Robot::ChargeUpdate, nullptr, "Charge", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::RepositionInit, &Robot::RepositionUpdate, nullptr, "Reposition", SMTypes::MOVE_PATHFINDING);
	AddState(&Robot::IdleInit, &Robot::IdleUpdate, nullptr, "Idle", SMTypes::MOVE_PATHFINDING);
	
	AddState(nullptr, &Robot::CheckAltitude, nullptr, "CheckAltitude", SMTypes::DIE_CHECK);
	
	SetMainState("Spawn", SMTypes::MOVE_PATHFINDING);
	SetMainState("CheckAltitude", SMTypes::DIE_CHECK);

	mEmmitter = mOwner->GetComponentType<SoundEmitter>();
	rb = mOwner->GetComponentType<Rigidbody>();
	mCol = mOwner->GetComponentType<DynamicCollider>();
	mOwner->mTag = Tags::Enemy;
	killtimer = 0.0f;
	maxTimeAllive = 120.0f;
	toSimpleRange = 220.0f;
	attackRange = 30.0f;
	chargeRange = 60.0f;
	NodeBeforeStun = -1; //for knowing where to go after stun 
	FramesBeforeNodeChange = 3u;
	MaxAltitude = 12.0f;

	serverItIsI = NetworkingMrg.AmIServer();

	players = Scene.FindObjects(Tags::Player);
	PlayerPtr = players[0];
 	GetClosestPlayer();
	subscribe_collision_event();
}

void Robot::ToJson(nlohmann::json& j) const 
{
	j["velocity"] << velocity;
	j["cooldown"] << cooldown;
	j["range"] << range;
	j["stunAnim"] << stunAnimation;
}

void Robot::FromJson(nlohmann::json& j) 
{
	j["velocity"] >> velocity;
	j["cooldown"] >> cooldown;
	j["range"] >> range;
	j["stunAnim"] >> stunAnimation;
}

void Robot::EndUpdate()
{
	Debug::DrawAABB(geometry::aabb{ {mOwner->mTransform.mPosition - 5.0f} , 
		{mOwner->mTransform.mPosition + 5.0f} }, glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
}

void Robot::setFollowingPlayerId(unsigned int id)
{
	following_player_id = id;
	PlayerPtr = players[id];
}

unsigned int Robot::getFollowingPlayerId()
{
	return following_player_id;
}

#ifdef EDITOR
bool Robot::Edit()
{
	ImGui::DragFloat("Velocity", &velocity, 1.f, 0.f);
	ImGui::DragFloat("Cool Down", &cooldown, 1.f, 0.f);
	ImGui::DragFloat("Range", &range, 1.f, 0.f);
	ImGui::DragInt("StunAnim", &stunAnimation, 0, 1);

	return false;
}
#endif // EDITOR

void Robot::SetVelocity(float _velocity)
{
	velocity = _velocity;
}

void Robot::StunEnemy()
{
	stunTimer = 0.0f;
	NodeBeforeStun = this->nodeToGoIdx;
	ChangeState("Stun");
}

void Robot::OnCollisionStarted(const Collider* coll)
{	
	if (!rb)
		return;
	
	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			on_ramp = true;
	}
}

void Robot::OnCollisionPersisted(const Collider* coll)
{
	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			on_ramp = true;
	}
}

void Robot::OnCollisionEnded(const Collider* coll)
{
	if (!rb)
		return;
}

void Robot::SpawnInit()
{
	if (rb)
	{
		old_grav = rb->mGravity;
		rb->mGravity = { 0,0,0 };
	}
	if (mCol)
		mCol->mbGhost = true;
}

void Robot::SpawnUpdate()
{
	if(spawned)
		ChangeState("MoveToFirst");
}

void Robot::SpawnEnd()
{
	if (mCol)
		mCol->mbGhost = false;
	if (rb)
	{
		rb->mVelocity = { 0,0,0 };
		rb->mGravity = old_grav;
	}
}

void Robot::SimpleMoveStart()
{
	FramesBeforeRayCheck = 30u;
}

void Robot::SimpleMoveUpdate()
{

	if (CheckAliveTime() || !PlayerPtr) return;

	//if (FramesBeforeRayCheck++ >= 30u)
	//{
	//	bool WallFree = CheckWall();
	//
	//	if (!WallFree)
	//	{
	//		ChangeState("Move");
	//		return;
	//	}
	//	FramesBeforeRayCheck = 0u;
	//}

	attackCooldowTimer += FRC.GetFrameTime();

	GetClosestPlayer();

	if (!PlayerPtr) return;
	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}
	
	float _range = glm::length(mOwner->mTransform.mPosition - PlayerPtr->mTransform.mPosition);
	//ChangeState("charge");

	if (_range < attackRange)
	{
		if (attackCooldowTimer >= attackCooldown && !PlayerPtr->GetComponentType<Player>()->mbDead)
			attackCooldowTimer = 0.0f;
	}
	else
	{
		auto& tr = mOwner->mTransform;
		glm::vec3 direction = (PlayerPtr->mTransform.mPosition - tr.mPosition);
		float distance = glm::length(direction);

		direction = glm::normalize(direction);
		direction.y = 0.0f;
		if (range < distance)
		{
			tr.mPosition += direction * velocity * 0.016f;
		}
	}

	//tr.mViewVector = direction;
	glm::vec3 target = glm::vec3(PlayerPtr->mTransform.mPosition.x, mOwner->mTransform.mPosition.y, PlayerPtr->mTransform.mPosition.z);
	mOwner->mTransform.LookAt(target);
	//glm::vec3 mViewVector = glm::normalize(direction);
	//tr.LookAt(PlayerPtr->mTransform.mPosition);
	//tr.mViewVector = mlerp(mViewVector, tr.mViewVector, 0.15f);
	//tr.mRightVector = glm::normalize(glm::cross(mViewVector, glm::vec3(0.0f, 1.0f, 0.0f)));
	//tr.mUpVect = { 0.0f,1.0f,0.0f };

	attackCooldowTimer += FRC.GetFrameTime();
	float range = glm::length(mOwner->mTransform.mPosition - PlayerPtr->mTransform.mPosition);

	if (!CheckIfInRange(mOwner->mTransform.mPosition, PlayerPtr->mTransform.mPosition, toSimpleRange * 1.5f))
		ChangeState("Move");

	if (CheckIfInRange(mOwner->mTransform.mPosition, PlayerPtr->mTransform.mPosition, chargeRange))
		ChangeState("Charge");

	if (CheckIfInRange(mOwner->mTransform.mPosition, PlayerPtr->mTransform.mPosition, attackRange))
	{
		if (attackCooldowTimer >= attackCooldown) 
			ChangeState("Attack");
	}

}

void Robot::SimpleMoveEnd()
{
}

void Robot::MoveToFirstNodeStart()
{
	FindNavMeshHolder();
}

void Robot::MoveToFirstNodeUpdate()
{
	if (CheckAliveTime()) return;

	Rigidbody* mrb = mOwner->GetComponentType<Rigidbody>();
	
	if (mrb)
		mrb->mGravity.y = -100.0f;

	//I put it in update because of spawning pos of enemy
	auto& mPos = mOwner->mTransform.mPosition;

	if (NavMeshC == nullptr) return; //sanity check

	FindClosestNode(&_1stNode);
	glm::vec3 dir = glm::normalize(_1stNode - mPos);
	dir.y = 0.0f;
	mOwner->mTransform.mPosition += dir * velocity * 0.016f;
	mOwner->mTransform.mViewVector = dir;

	mOwner->mTransform.LookAt(mPos + dir);

	glm::vec3 compareVec = _1stNode - mPos;
	compareVec.y = 0.0f;
	//ChangeState("MoveSimple", SMTypes::MOVE_PATHFINDING);
	if (glm::length(compareVec) < 2.5f) {			//change to normal pathfinding once we reach the node
		ChangeState("Move", SMTypes::MOVE_PATHFINDING);
	}

}

void Robot::MoveToFirstNodeEnd()
{
}

//MOVE START
void Robot::MoveStart()
{
	mCol->mbGhost = false;
	rb->mGravity = glm::vec3(0.0f, -100.0f, 0.0f);
		
	GetClosestPlayer();

	if (!PlayerPtr) return;

	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	//In start, we compute the path that we will follow	
	FindNavMeshHolder();
	
	if (NodeBeforeStun == -1)
		UpdatePath();
	else {
		nodeToGoIdx = NodeBeforeStun;
		NodeBeforeStun = -1;
	}

	FramesBeforeNodeChange = 30u;
	//mEmmitter->PlayCue("./../Resources/Audio/robotmovement.mp3", 0.1f, false, false, false); //play sound of walking
}

//FOLLOW THE PATH
void Robot::MoveUpdate()
{
	GetClosestPlayer();

	if (!PlayerPtr) return;

	FollowPath();

#ifdef EDITOR
	//NavMeshC->ShowNodes();
#endif
}

void Robot::MoveEnd()
{
	mEmmitter->default_pause = true;
}

void Robot::RetrackStart()
{
	GetClosestPlayer();

	if (!PlayerPtr) return;

	FindNavMeshHolder();
	auto endNode = FindClosestPlayer();

	retrackPrev = PathIndices[nodeToGoIdx - 1];
	retrackNext = PathIndices[nodeToGoIdx];

	std::vector<unsigned> pathToFill;
	
	NavMeshC->GetClosetPath(retrackPrev, endNode, &pathToFill);
	auto CostPrev = pathToFill.size();

	pathToFill.clear();
	
	NavMeshC->GetClosetPath(retrackNext, endNode, &pathToFill);
	auto CostNext = pathToFill.size();


	//if (CostPrev < CostNext) 
	//	retrackPos = PathPositions[nodeToGoIdx - 1];
	//else if (CostNext < CostPrev) 
		retrackPos = PathPositions[nodeToGoIdx];
	
	

}

void Robot::RetrackUpdate()
{
	GetClosestPlayer();

	if (!PlayerPtr) return;

	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	auto& tr = mOwner->mTransform;
	auto& playerTr = PlayerPtr->mTransform;


	glm::vec3 dirToNode = retrackPos - tr.mPosition;
	glm::vec3 compareNode = { dirToNode.x,0.0f,dirToNode.z };
	//change back
	if (glm::length2(compareNode) < 2.5f) {			//change to normal pathfinding once we reach the node
		ChangeState("Move", SMTypes::MOVE_PATHFINDING);
		return;
	}

	glm::vec3 comparePlayer = playerTr.mPosition - tr.mPosition;

	if (CheckIfInRange(playerTr.mPosition, tr.mPosition, toSimpleRange) && !TooMuchAltitude(playerTr.mPosition)) {
		ChangeState("MoveSimple", SMTypes::MOVE_PATHFINDING);
		return;
	}
	
	dirToNode = glm::normalize(dirToNode);
	dirToNode.y = 0.0f;

	tr.mPosition += dirToNode * velocity * 0.016f;

}

void Robot::RetrackEnd()
{
}

void Robot::FollowPath()
{
	if (CheckAliveTime()) return;

	GetClosestPlayer();
	if (!PlayerPtr) return;

	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	FramesBeforeNodeChange++;

	//Change path if players have moved every 3 frames 
	if (FramesBeforeNodeChange >= 30u) {
		unsigned endNode = FindClosestPlayer();

		//player has moved, reset and go back to closest node
		if (endNode != PathIndices.back()) {
			if (nodeToGoIdx > 0u) {
				ChangeState("Retrack", SMTypes::MOVE_PATHFINDING);
				return;
			}
		}
		FramesBeforeNodeChange = 0u;
	}

	auto& tr = mOwner->mTransform;
	auto& pos = mOwner->mTransform.mPosition; //position of the robot
	auto& nextPos = PathPositions[nodeToGoIdx]; //position of the node to gos
	
	auto  nextNodeDir = glm::normalize(nextPos - pos); //direction to next node
	nextNodeDir.y = 0.0f;
	pos += nextNodeDir * velocity * 0.016f; //update position

	//update rotation if close to node

	if (glm::distance2(pos, nextPos) > 2.0f) {
		mOwner->mTransform.LookAt(pos + nextNodeDir);
	}

	//Player is very close, change to simple behavior
	float aa = glm::distance2(pos, PlayerPtr->mTransform.mPosition);
	if (CheckIfInRange(pos, PlayerPtr->mTransform.mPosition, toSimpleRange) && !TooMuchAltitude(PlayerPtr->mTransform.mPosition)) 
	{
		mCol->mbGhost = false;
		rb->mGravity.y = -100.0f;
		ChangeState("MoveSimple", SMTypes::MOVE_PATHFINDING);
		return;
	}

	//we reached the destination node
	glm::vec3 compareVec = pos - nextPos;
	compareVec.y = 0.0f;

	if (glm::length2(compareVec) < 10.0f) {
		nodeToGoIdx++;

		//we reached the end of the path
		if (nodeToGoIdx > PathPositions.size()) {
			nodeToGoIdx = -1;
			ChangeState("MoveSimple", SMTypes::MOVE_PATHFINDING); //Change to following player normaly
		}
	}


}

void Robot::UpdatePath()
{
	GetClosestPlayer();
	if (!PlayerPtr) return;

	unsigned endNode = FindClosestPlayer(); //is for the node closest to player not for player itself e
	unsigned startNode = FindClosestNode(); //starting node is the 
	
	nodeToGoIdx = 0u;
	nodeToGo = startNode;
	PathPositions.clear();
	PathIndices.clear();
	PathPositions = NavMeshC->GetClosetPath(startNode, endNode,&PathIndices);
}

void Robot::FindNavMeshHolder()
{
	if (!NavMeshHolder) {
		NavMeshHolder = Scene.FindObject("NavMeshObj");
		NavMeshC = NavMeshHolder->GetComponentType<NavMesh>(); //get the nodes
	}
}

void Robot::ChargeInit()
{
	targetAngle = glm::linearRand(0.0f,PI);
	glm::vec3 targetdir = { cosf(targetAngle), 0.0f,sinf(targetAngle) };

	chargeVel = 0.5f;
}

void Robot::ChargeUpdate()
{

	glm::vec3 targetdir = { cosf(targetAngle), 0.0f,sinf(targetAngle) };
	targetdir *= 30.0f;

	auto& tr = mOwner->mTransform;

	glm::vec3 dirPlayer = PlayerPtr->mTransform.mPosition - tr.mPosition;
	dirPlayer.y = 0.0f;
	glm::vec3 direction = (dirPlayer + targetdir);
	
	float distance = glm::length(dirPlayer);

	if (distance < attackRange)
		ChangeState("Attack");

	direction = glm::normalize(direction);
	direction.y = 0.0f;

	chargeVel += FRC.GetFrameTime();
	chargeVel = chargeVel > 1.5f ? 1.5f : chargeVel;

	tr.mPosition += direction * velocity * FRC.GetFrameTime() * chargeVel;

	mOwner->mTransform.LookAt(tr.mPosition + dirPlayer);

}

unsigned Robot::FindClosestPlayer()
{
	//check we are following a player
	if (!followingAPlayer())
		return -1;

	auto& positions = NavMeshC->mGraph.GetPosArray();
	float MinDist = FLT_MAX;
	unsigned closestNode = -1;
	unsigned id = 1u;
	//compare positions and find
	for (auto& it : positions)
	{
		float dstSq = glm::distance2(PlayerPtr->mTransform.mPosition, it);
		
		if (dstSq < MinDist) {
			MinDist = dstSq;
			closestNode = id;
		}

		id++;
	}

	return closestNode;
}

unsigned Robot::FindClosestNode(glm::vec3* positionToFill)
{

	auto& positions = NavMeshC->mGraph.GetPosArray();
	glm::vec3 closestPos = { 0.0f,0.0f,0.0f };
	float MinDist = FLT_MAX;
	unsigned closestNode = -1;
	unsigned id = 1u;
	//compare positions and find closest node to enemy
	for (auto& it : positions)
	{
		float dstSq = glm::distance2(mOwner->mTransform.mPosition, it);

		if (dstSq < MinDist) {
			closestPos = it;
			MinDist = dstSq;
			closestNode = id;
		}

		id++;
	}

	previousNode = closestNode;

	if (positionToFill)
		(*positionToFill) = closestPos;

	return closestNode;
}

void Robot::AttackInit()
{
	//attackTimer = 0.0f
	attackAnticipationAngle = 0.0f;
	forwardBeforeAnticipation = mOwner->mTransform.mViewVector;

	GetClosestPlayer();
}

void Robot::AttackUpdate()
{
	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	attackAnticipationAngle = lerp(attackAnticipationAngle, -180.0f, 0.04f);
	
	auto mtx = glm::rotate(glm::radians(attackAnticipationAngle), mOwner->mTransform.mUpVect);
	glm::vec3 newView = mtx * glm::vec4(forwardBeforeAnticipation,0.0f);
	mOwner->mTransform.LookAt(mOwner->mTransform.mPosition + newView);

	attackTimer += FRC.GetFrameTime();
	
	//follow player 
	glm::vec3 PlayerVec = PlayerPtr->mTransform.mPosition - mOwner->mTransform.mPosition;
	PlayerVec.y = 0.0f;
	PlayerVec = glm::normalize(PlayerVec);
	mOwner->mTransform.mPosition += PlayerVec * FRC.GetFrameTime() * velocity * 0.2f;

	//timer for the attack animation
	if (attackAnticipationAngle <= -90.0f)
	{
		attackCooldowTimer = 0.0f;
		attackTimer = 0.0f;

		ChangeState("Spin");
	    //ChangeState("MoveSimple");
		
	}
}

void Robot::SpinInit()
{
	GetClosestPlayer();

	if (!PlayerPtr) return;
	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	spinAngle = 0.0f;

	//TO CHECK WITH BOX INSTEAD OF RANGE
	float _range = glm::length(mOwner->mTransform.mPosition - PlayerPtr->mTransform.mPosition);
	/*Collider* playerCollider = PlayerPtr->GetComponentType<Collider>();
	Collider* mCollider = mOwner->GetComponentType<Collider>();

	auto scale1 = playerCollider->mScale;
	auto scale2 = mCollider->mScale;

	auto origin1 = PlayerPtr->mTransform.mPosition + playerCollider->mOffset;
	auto origin2 = mOwner->mTransform.mPosition + mCollider->mOffset;

	geometry::aabb Box1{ origin1 - scale1,origin1 + scale1};
	geometry::aabb Box2{ origin2 - scale2, origin2 + scale2 };

	bool hit = CollisionManager.AABBvsAABB(Box1, Box2);*/

	if (PlayerPtr && _range < attackRange / 1.25f)
	{
		PlayerPtr->GetComponentType<Player>()->hit_direction = PlayerPtr->mTransform.mPosition - mOwner->mTransform.mPosition;
		PlayerPtr->GetComponentType<Player>()->ChangeState("Damaged", Player::SMTypes::COMBAT_SM);
	}
}

void Robot::SpinUpdate()
{
	//check we are following a player
	if (!PlayerPtr) return;
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	spinAngle += spinSpeed * FRC.GetFrameTime() * 60.0f;
	mOwner->mTransform.RotateAround(mOwner->mTransform.mUpVect, spinSpeed);
	
	//follow player 
	glm::vec3 PlayerVec = PlayerPtr->mTransform.mPosition - mOwner->mTransform.mPosition;
	PlayerVec.y = 0.0f;

	//if it is too close go back a little bit
	float distPlayer = glm::length(PlayerVec);
	
	PlayerVec = glm::normalize(PlayerVec);
	mOwner->mTransform.mPosition += PlayerVec * FRC.GetFrameTime() * velocity * 0.5f;

	if (spinAngle >= 360.0f * 1.0) //5 spins
	{
		mOwner->mTransform.LookAt(mOwner->mTransform.mPosition + forwardBeforeAnticipation);
		
		if (distPlayer < attackRange / 2.0f)
			ChangeState("Reposition");
		else
			ChangeState("MoveSimple");
	}
}

void Robot::RepositionInit()
{
	
}

void Robot::RepositionUpdate()
{
	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	//go back 
	glm::vec3 PlayerVec = PlayerPtr->mTransform.mPosition - mOwner->mTransform.mPosition;
	PlayerVec.y = 0.0f;

	if (glm::length(PlayerVec) >= attackRange)
		ChangeState("Charge");

	PlayerVec = glm::normalize(PlayerVec);
	mOwner->mTransform.mPosition -= PlayerVec * FRC.GetFrameTime() * velocity;
	mOwner->mTransform.LookAt(mOwner->mTransform.mPosition + PlayerVec);
}

void Robot::IdleInit()
{
}

void Robot::IdleUpdate()
{
	if (!followingAPlayer()) GetClosestPlayer();
	else
	{
		if (!PlayerPtr) return;
		if (CheckIfInRange(mOwner->mTransform.mPosition, PlayerPtr->mTransform.mPosition, attackRange))
			ChangeState("MoveSimple");
		else
			ChangeState("Move");
	}
}

bool Robot::followingAPlayer()
{
	if (PlayerPtr != nullptr) return true;
	return false;
}

void Robot::StunInit()
{
	stunAngle = 45.0f / 0.2f;
	forwardBeforeAnticipation = mOwner->mTransform.mViewVector;
	renderable* mRend = mOwner->GetComponentType<renderable>();
	DamageZonesHandler* dmg = mOwner->GetComponentType<DamageZonesHandler>();
	glm::vec3 hitDir = -dmg->hitDir; //get direction from where they hit you
	hitDir.y = 0.0f;

	StunRotationVec = glm::normalize(glm::cross(hitDir, mOwner->mTransform.mUpVect)); //get vector to rotate around

	//play stun sound 
	mEmmitter->PlayCue("./../Resources/Audio/stun.mp3", 1.0f, false, false, false); //play death anim
	
	mRend->SetColor({ 1.0f,0.0f,0.0f,1.0f });
}

void Robot::StunUpdate()
{
	//check we are following a player
	if (!followingAPlayer()) {
		ChangeState("Idle");
		return;
	}

	stunTimer += FRC.GetFrameTime();

	if(stunAnimation == 1) //play animation 1
		mOwner->mTransform.RotateAround(StunRotationVec,stunAngle * FRC.GetFrameTime());

	if (stunTimer > maxStunTime)
	{
		mOwner->mTransform.LookAt(mOwner->mTransform.mPosition + forwardBeforeAnticipation);

		if (CheckIfInRange(mOwner->mTransform.mPosition, PlayerPtr->mTransform.mPosition, attackRange))
			ChangeState("MoveSimple");
		else
			ChangeState("Move");

		renderable* mRend = mOwner->GetComponentType<renderable>();
		mRend->SetColor({ 1.0f,1.0f,1.0f,1.0f }); //reset to normal color

	}

}

void Robot::DieInit()
{
	mEmmitter->PlayCue("./../Resources/Audio/Robot_Death_Powerdown.wav", 0.6f, false, false, false); //play death anim
	
	forwardBeforeAnticipation = mOwner->mTransform.mViewVector;
	DieTimer = 0.0f;
	dieAngle = 90.0f / DieTime;
}

void Robot::DieUpdate()
{
	DieTimer += FRC.GetFrameTime();
	mOwner->mTransform.RotateAround(mOwner->mTransform.mViewVector, dieAngle * FRC.GetFrameTime());

	
	if (DieTimer > DieTime) {
		Scene.DestroyObject(mOwner);
		WaveSys.defeated_enemies += 1;
	}
}

void Robot::CheckAltitude()
{
	if (on_ramp)
		rb->mDrag = glm::vec3(0.0F);
	else
		rb->mDrag = glm::vec3(0.99f);
	on_ramp = false;

	//KILL IF THEY FALL
	if (mOwner->mTransform.mPosition.y < -2000.0f)
	{
		Scene.DestroyObject(mOwner);
		WaveSys.spawned_enemies -= 1;
	}
}

bool Robot::CheckAliveTime()
{
	if (followingAPlayer() && glm::distance2(mOwner->mTransform.mPosition, PlayerPtr->mTransform.mPosition) < 5000.0f)
		return false;
	
	killtimer += FRC.GetFrameTime();

	if (killtimer > maxTimeAllive) {
		ChangeState("Die");
		return true;
	}

	return false;
}

bool Robot::CheckWall()
{
	//check we are following a player
	if (!followingAPlayer())
		return false;

	glm::vec3 _rayVec = glm::normalize(PlayerPtr->mTransform.mPosition - mOwner->mTransform.mPosition);
	geometry::ray _ray({ mOwner->mTransform.mPosition}, { _rayVec });
	_ray.mOrigin -= _rayVec; //get it a little bit to the back of the robot

	auto colliders = mOwner->GetSpace()->GetComponentsType<StaticCollider>();
	float t = FLT_MAX;
	float static_t = FLT_MAX;

	//check with all the static colliders in the level
	std::for_each(colliders.begin(), colliders.end(),
		[&](IComp* obj)
		{
			StaticCollider* coll = dynamic_cast<StaticCollider*>(obj);
			float temp_t = -1.0f;

			if (coll->mShape == shape::AABB)
			{
				geometry::aabb hitbox = { coll->mOwner->mTransform.mPosition + coll->mOffset - coll->mScale / 2.0f ,
									  coll->mOwner->mTransform.mPosition + coll->mOffset + coll->mScale / 2.0f };
				temp_t = geometry::intersection_ray_aabb(_ray, hitbox);
			}
			else if (coll->mShape == shape::OBB)
			{
				geometry::obb hitbox = { coll->mOwner->mTransform.mPosition + coll->mOffset ,
										 coll->mScale / 2.0f , coll->mOrientationMtx };
				temp_t = geometry::intersection_ray_obb(_ray, hitbox);
			}
			else if (coll->mShape == shape::SPHERICAL)
			{
				geometry::sphere hitbox = { coll->mOwner->mTransform.mPosition + coll->mOffset , coll->mScale.x };
				temp_t = geometry::intersection_ray_sphere(_ray, hitbox);
			}

			if (temp_t < t && temp_t != -1.0f)
				t = temp_t;
		});

	//geometry::segment r; 
	//r.p1 = mOwner->mTransform.mPosition;
	//r.p2 = mOwner->mTransform.mPosition + _ray.mDirection * 100.0f;
	//Debug::DrawLine(r, { 1.0f,1.0f,1.0f,1.0f });
	//store the closest static collision
	static_t = t;

	auto& playerPos = PlayerPtr->mTransform.mPosition;
	auto& playerScale = PlayerPtr->mTransform.mScale;
	geometry::sphere PlayerSphere{playerPos, playerScale.x/2.0f};
	
	float player_t = geometry::intersection_ray_sphere(_ray, PlayerSphere);

	if (player_t < 0.0f) return false;

	return player_t < static_t;
}

void Robot::GetClosestPlayer()
{
	if (serverItIsI) 
	{
		float smallest_dist = FLT_MAX;
		float y_dist_curr = FLT_MAX;
		PlayerPtr = nullptr;
		for(unsigned int id = 0; id < players.size(); id++) {
			if (!players[id]->GetComponentType<Player>()->mbDead) {
				glm::vec3 feet = players[id]->mTransform.mPosition - players[id]->mTransform.mScale.y / 2.5f; //get path from feet
				float distance = glm::length(mOwner->mTransform.mPosition - feet);

				float y_dist_this = glm::length(mOwner->mTransform.mPosition.y - feet.y);

				if(PlayerPtr)
					y_dist_curr = glm::length(mOwner->mTransform.mPosition.y - PlayerPtr->mTransform.mPosition.y);

				if (distance < smallest_dist /*&& y_dist_this < y_dist_curr*/) {
					PlayerPtr = players[id];
					following_player_id = id;
					smallest_dist = distance;
				}
			}
		}
	}
}


bool Robot::CheckIfInRange(glm::vec3 mPos, glm::vec3 dest, float _range)
{
	float y_range = _range / 3.0f;
	y_range *= y_range;
	_range *= _range;
	float distance_xz = glm::distance2(glm::vec3(mPos.x, 0, mPos.z), glm::vec3(dest.x, 0, dest.z));
	float distance_y = glm::distance2(mPos.y, dest.y);

	if (distance_xz <= _range && distance_y <= y_range)
		return true;
	return false;
}

bool Robot::TooMuchAltitude(glm::vec3 playerPos)
{
	float Ydiff = mOwner->mTransform.mPosition.y - playerPos.y;
	if (Ydiff < 0.0f) Ydiff *= -1.0f;

	if (Ydiff > MaxAltitude) return true;

	return false;
}
