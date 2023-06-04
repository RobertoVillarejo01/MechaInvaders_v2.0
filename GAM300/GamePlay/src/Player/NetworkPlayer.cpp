#include "NetworkPlayer.h"
#include "Weapon/Weapon.h"
#include "Physics/Rigidbody/Rigidbody.h"
#include "Serializer/Factory.h"
#include "Graphics/Renderable/Renderable.h"
#include "Health/Health.h"
#include "Geometry/Geometry.h"
#include "Utilities/Utils.h"
#include "GameStateManager/GameStateManager.h"
#include "AudioManager/Audio.h"

#include "Networking\Networking.h"

void NetworkPlayer::StateMachineInitialize()
{
	mOwner->mTag = Tags::Player;

	camera = mOwner->FindChild("Camera");
	mWeapon = mOwner->FindChild("Weapon");

	mRigidBody = mOwner->GetComponentType<Rigidbody>();
	playercolls = mOwner->GetComponentsType<DynamicCollider>();
	mHealth = mOwner->GetComponentType<Health>();
	mEmitter = mOwner->GetComponentType<SoundEmitter>();

	subscribe_collision_event();
	SetBrainSize(SMTypes::TOTAL_SM);

	AddState(&NetworkPlayer::MoveInit, &NetworkPlayer::MoveUpdate, nullptr, "Move", SMTypes::COMBAT_SM);
	SetMainState("Move", 0);
	AddState(&NetworkPlayer::DamagedInit, &NetworkPlayer::DamagedUpdate, &NetworkPlayer::DamagedEnd, "Damaged", SMTypes::COMBAT_SM);
	AddState(&NetworkPlayer::DieInit, &NetworkPlayer::DieUpdate, nullptr, "Die", SMTypes::COMBAT_SM);
	AddState(&NetworkPlayer::ReviveInit, &NetworkPlayer::ReviveUpdate, nullptr, "Revive", SMTypes::COMBAT_SM);
	AddState(&NetworkPlayer::Silent, nullptr, nullptr, "Silent", SMTypes::VOICES_SM);
	AddState(nullptr, &NetworkPlayer::HealUpdate, nullptr, "Heal", SMTypes::HEAL_SM);
	AddState(nullptr, &NetworkPlayer::CheatCodes, nullptr, "Cheats", SMTypes::CHEATS_SM);

	mbDead = false;
	base_player = false;

	//guarrada para conseguir mi id (Expected format: Player#)
	std::string name = mOwner->GetName();
	if (name.find("Player") != std::string::npos && name.length() == 7) {
		name.erase(0, 6); // "Player" lenght == 6
		id = stoi(name);
	}

	if (mRigidBody) {
		PlayerGravity = glm::vec3(0.0F, -200.0f, 0.0F);
		mRigidBody->mGravity = glm::vec3(0.0F, -9.8f, 0.0F);
		firtcoll = true;
	}
}

void NetworkPlayer::StateMachineShutDown()
{
	if (gameOver != nullptr)
	{
		Scene.DestroyObject(gameOver);
		gameOver = nullptr;
	}
}

void NetworkPlayer::ToJson(nlohmann::json& j) const
{
	// j["MoveSpeed"] << MoveSpeed;
	// j["SprintSpeed"] << RunSpeed;
	// j["SlopeGravity"] << mGravitySlope;
	// 
	// j["SprintCooldown"] << sprintCooldown;
	// j["SprintLimit"] << sprintLimit;
	// j["JumpForce"] << jumpForce;
	// j["StunTime"] << stun_time;
	// j["StunDisplacement"] << stun_displacement;
	// 
	// j["JumpSpeed"] << Jump_speed;
	// j["AirSpeed"] << Air_speed;
	// 
	// j["Run_Fov"] << run_fov;
	// 
	// j["heal_speed"] << heal_speed;
	// j["damaged_cooldown"] << damaged_cooldown;
}

void NetworkPlayer::FromJson(nlohmann::json& j)
{
	// if (j.find("MoveSpeed") != j.end())
	// 	j["MoveSpeed"] >> MoveSpeed;
	// 
	// if (j.find("SprintSpeed") != j.end())
	// 	j["SprintSpeed"] >> RunSpeed;
	// 
	// if (j.find("SlopeGravity") != j.end())
	// 	j["SlopeGravity"] >> mGravitySlope;
	// 
	// if (j.find("SprintCooldown") != j.end())
	// 	j["SprintCooldown"] >> sprintCooldown;
	// 
	// if (j.find("SprintLimit") != j.end())
	// 	j["SprintLimit"] >> sprintLimit;
	// 
	// if (j.find("JumpForce") != j.end())
	// 	j["JumpForce"] >> jumpForce;
	// 
	// if (j.find("StunTime") != j.end())
	// 	j["StunTime"] >> stun_time;
	// 
	// if (j.find("StunDisplacement") != j.end())
	// 	j["StunDisplacement"] >> stun_displacement;
	// 
	// if (j.find("JumpSpeed") != j.end())
	// 	j["JumpSpeed"] >> Jump_speed;
	// 
	// if (j.find("AirSpeed") != j.end())
	// 	j["AirSpeed"] >> Air_speed;
	// 
	// if (j.find("Run_Fov") != j.end())
	// 	j["Run_Fov"] >> run_fov;
	// 
	// if (j.find("heal_speed") != j.end())
	// 	j["heal_speed"] >> heal_speed;
	// 
	// if (j.find("damaged_cooldown") != j.end())
	// 	j["damaged_cooldown"] >> damaged_cooldown;
}

void NetworkPlayer::OnCollisionStarted(const Collider* coll)
{
	if (firtcoll)
	{
		mRigidBody->mGravity = PlayerGravity;
		firtcoll = false;
	}

	//fly enemy projectile
	if (coll->GetOwner()->mTag == Tags::EnemyBullet)
	{
		ChangeState("Damaged", NetworkPlayer::SMTypes::COMBAT_SM);
	}

	if (!mRigidBody)
		return;

	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			mRigidBody->mDrag = glm::vec3(0.0F);
	}
	else
	{
		mRigidBody->mDrag = glm::vec3(0.99f);
	}
}

void NetworkPlayer::OnCollisionPersisted(const Collider* coll)
{
	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			mRigidBody->mDrag = glm::vec3(0.0F);
	}
}

void NetworkPlayer::OnCollisionEnded(const Collider* coll)
{
	if (!mRigidBody)
		return;

	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			mRigidBody->mDrag = glm::vec3(0.99f);
	}
}

#ifdef EDITOR

bool NetworkPlayer::Edit()
{
	ImGui::Text("MOVEMENT");
	ImGui::DragFloat("MoveSpeed", &MoveSpeed, 0.02F);
	ImGui::DragFloat("Slope Gravity", &mGravitySlope);

	ImGui::Separator();
	ImGui::Text("SPRINT");
	ImGui::DragFloat("SprintSpeed", &RunSpeed, 0.02F);
	ImGui::DragFloat("SprintCooldown", &sprintCooldown, 0.02F);
	ImGui::DragFloat("SprintLimit", &sprintLimit, 0.02F);

	ImGui::Separator();
	ImGui::Text("JUMP");
	ImGui::DragFloat("JumpSpeed", &Jump_speed, 0.01F);
	ImGui::DragFloat("AirSpeed", &Air_speed, 0.01F);
	ImGui::DragFloat("JumpForce", &jumpForce, 0.02F);

	ImGui::Separator();
	ImGui::Text("STUN");
	ImGui::DragFloat("Stun Time", &stun_time, 0.01f);
	ImGui::DragFloat("Stun Force", &stun_displacement, 0.1f);

	ImGui::Separator();
	ImGui::Text("HEAL");
	ImGui::DragFloat("Heal Speed", &heal_speed, 0.01f);
	ImGui::DragFloat("Damaged Cooldown", &damaged_cooldown, 0.1f, 1.0f, 30.0f);

	return false;
}

#endif

void NetworkPlayer::MoveInit()
{
}

void NetworkPlayer::MoveUpdate()
{
	//update the jump
	if (mbJumping)
	{
		mbIdle = false;
		if (Direction != glm::vec3(0.0f))
			current_jump_speed = dir_before_jump * Jump_speed + Direction * Air_speed;
		mOwner->mTransform.mPosition = mOwner->mTransform.mPosition + current_jump_speed;
		jumptimer += FRC.GetFrameTime();
	}
	if (jumptimer >= 0.7f)
	{
		if (mRigidBody->mVelocity.y < 1.0f && mRigidBody->mVelocity.y > -1.0f)
		{
			mbJumping = false;
			jumptimer = 0.0f;
		}
	}

	if (mbSprinting)
	{
		//if (!KeyDown(Key::LShift) && base_player)
		//	mbSprinting = false;
		if (sprintTime <= sprintLimit)
			sprintTime += FRC.GetFrameTime();
		else
			mbSprinting = false;
	}
	else
	{
		mbWalking = true;
		sprintTime = 0.0F;
		cooldown += FRC.GetFrameTime();
	}
}

//------------DAMAGED-----------
void NetworkPlayer::DamagedInit()
{
	if (mbDead) return;

	stun_dt = 0.0f;
	if (!immortal)
		mHealth->GetDamaged(20.0f);
	hit_direction.y = 0.0f;

	if (hit_direction != glm::vec3(0.0f))
		hit_direction = glm::normalize(hit_direction);

	mRigidBody->AddForce(hit_direction * stun_displacement);
	//stun_new_pos = mOwner->mTransform.mPosition + hit_direction * stun_displacement;
	mbDamaged = true;
	damaged_timer = 0.0f;
	mRigidBody->mDrag = glm::vec3(0.80f);
	if (mEmitter)
		mEmitter->PlayCue("./../Resources/Audio/damage.mp3", 1.0f, false, false, false);
}

void NetworkPlayer::DamagedUpdate()
{
	stun_dt += FRC.GetFrameTime();

	//mOwner->mTransform.mPosition = lerp(mOwner->mTransform.mPosition, stun_new_pos, 0.25f);

	//float dist = glm::length2((mOwner->mTransform.mPosition - stun_new_pos));

	if (mHealth->getCurrentHealth() <= 0.0f)
		ChangeState("Die");


	else if (stun_dt >= stun_time)
		ChangeState("Move");
}

void NetworkPlayer::DamagedEnd()
{
	mRigidBody->mDrag = glm::vec3(0.99f);
}

void NetworkPlayer::DieInit()
{
	mbDead = true;
	time_dead = 0.0f;

	//assuming that the first collider is the one in the feet
	collOffset = playercolls[0]->mOffset;
	init_pos = mOwner->mTransform.mPosition;
	playercolls[0]->mOffset = glm::vec3(0.0f);
	playercolls[1]->mbGhost = true;
}

void NetworkPlayer::DieUpdate()
{
	time_dead += FRC.GetFrameTime();


	if (KeyTriggered(Key::R))
	{
		Scene.DestroyObject(gameOver);
		gameOver = nullptr;
	}
}

//Base Player preemptively apply their movement input locallyand will send said input data to the server.

void NetworkPlayer::ReviveInit()
{
	mbDead = false;
	mHealth->RestoreHealthMax();

	mOwner->mTransform.mPosition = init_pos;

	//assuming that the first collider is the one in the feet
	playercolls[0]->mOffset = collOffset;
	playercolls[1]->mbGhost = false;
}

void NetworkPlayer::ReviveUpdate()
{
	ChangeState("Move");
}

void NetworkPlayer::moveUpdate(move_input& p)
{
	if (mbDead)
		return;

	Direction = {};
	float speed = MoveSpeed;
	//============================ move forward ==============================================
	if (p.info & move_input::forward)
	{
		Direction += mMovementFor;
		if ((p.info & move_input::sprint) && !mbSprinting && !mbShooting) {
			if (cooldown >= sprintCooldown)
			{
				cooldown = 0.0F;
				mbSprinting = true;
				mbWalking = false;
			}
			if (sprintTime <= sprintLimit)
				sprintTime += FRC.GetFrameTime();
			else
				mbSprinting = false;
		}
		if (mbSprinting) speed = RunSpeed;
	}

	//============================ move to the left ==========================================
	if (p.info & move_input::left)
		Direction -= mMovementRight;

	//============================ move backforward ==========================================
	if (p.info & move_input::back)
		Direction -= mMovementFor;

	//============================ move to the left ==========================================
	if (p.info & move_input::right)
		Direction += mMovementRight;

	if (Direction != glm::vec3(0.0f))
		Direction = glm::normalize(Direction);

	if (!mbJumping)
	{
		Direction *= speed;
		mOwner->mTransform.mPosition = mOwner->mTransform.mPosition + Direction;
	}

	//============================ shoot =====================================================
	//mWeaponComp = p.angle;
	//mPlayerCameraComp->UpdateVectors();

	/*{
		// update Front, Right and Up Vectors using the updated Euler angles
		glm::vec3 target;
		glm::vec3 my_pos = mOwner->mTransform.mPosition;

		//computing the new view vector based on rotations
		target.x = my_pos.x + sin(glm::radians(p.angle.y)) * cos(glm::radians(p.angle.x));
		target.y = my_pos.y + cos(glm::radians(p.angle.y));
		target.z = my_pos.z + sin(glm::radians(p.angle.y)) * sin(glm::radians(p.angle.x));

		mOwner->mTransform.LookAt(target);
		mWeapon->mTransform.LookAt(-target);
	}*/

	if (p.info & move_input::shoot)
	{
		mbSprinting = false;
		shoot();
	}

	//============================ jump ======================================================
	if (!mRigidBody)
		return;

	if (p.info & move_input::space)
	{
		if (!mbJumping)
		{
			dir_before_jump = Direction != glm::vec3(0.0f) ? glm::normalize(Direction) : glm::vec3(0.0f);
			current_jump_speed = dir_before_jump * Jump_speed;
			Direction = glm::vec3(0.0f);
			mRigidBody->AddForce(glm::vec3(0, jumpForce, 0));
			mbJumping = true;
		}
	}

	//============================ update movement vector ====================================
	mMovementFor = glm::vec3(mOwner->mTransform.mViewVector.x, 0.0F, mOwner->mTransform.mViewVector.z);
	mMovementRight = glm::vec3(mOwner->mTransform.mRightVector.x, 0.0F, mOwner->mTransform.mRightVector.z);
}

void NetworkPlayer::shoot()
{
	if (mbDead) return;
	///mWeaponComp->ChangeState("Shoot", Weapon::SMTypes::SHOOTING);
}

void NetworkPlayer::HealUpdate()
{
	if (mbDead) return;
	if (mbDamaged)
	{
		damaged_timer += FRC.GetFrameTime();
		if (damaged_timer >= damaged_cooldown)
			mbDamaged = false;
	}
	else
		mHealth->Heal(heal_speed);
}

void NetworkPlayer::CheatCodes()
{
	if (GSM.mConfig.mbCheats)
	{
		if (KeyDown(Key::U) && KeyTriggered(Key::Num1))
			immortal = !immortal;
	}
}
