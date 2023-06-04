#include "Player.h"
#include "Weapon/Weapon.h"
#include "Graphics/Camera/Camera.h"
#include "Physics/Rigidbody/Rigidbody.h"
#include "Serializer/Factory.h"
#include "Graphics/Renderable/Renderable.h"
#include "Health/Health.h"
#include "Geometry/Geometry.h"
#include "PlayerCamera.h"
#include "Utilities/Utils.h"
#include "GameStateManager/GameStateManager.h"
#include "TaskSystem/TaskSystem.h"
#include "TaskSystem/TaskInfo.h"
#include "TaskSystem/FixingTask/FixingTask.h"
#include "HUD/HUDBar.h"
#include "AudioManager/Audio.h"
#include "Interaction/InteractionText.h"
#include "Interaction/InteractionComp.h"
#include "../VendingMachine/VendingMachine.h"
#include "Graphics/TextRendering/TextRender.h"

#include "Networking\Networking.h"

GameObject* test = nullptr;
void Player::StateMachineInitialize()
{
	mOwner->mTag = Tags::Player;

	camera = mOwner->FindChild("Camera");
	mWeapon = mOwner->FindChild("Pistol");

	mRigidBody = mOwner->GetComponentType<Rigidbody>();
	playercolls = mOwner->GetComponentsType<DynamicCollider>();
	mCameraComp	= mOwner->GetComponentType<CameraComp>();
	mPlayerCameraComp = mOwner->GetComponentType<PlayerCamera>();
	mHealth	= mOwner->GetComponentType<Health>();
	mWeaponComp	= mWeapon->GetComponentType<Weapon>();
	mEmitter = mOwner->GetComponentType<SoundEmitter>();
	InteractComp = mOwner->GetComponentType<InteractionComp>();

	GameObject* BarHUD = Scene.FindObject("HUDBar");
	if(BarHUD)
		hud_bar = BarHUD->GetComponentType<HUDBar>();

	GameObject* text = mOwner->GetSpace()->FindObject("InteractionText");
	if (text) {
		interaction_text = text->GetComponentType<InteractionText>();
		interaction_text->mPlayer = this;
	}

	
	GameObject* revivetext = Scene.FindObject("ReviveMe");
	if (revivetext)
	{
		revive_text = revivetext->GetComponentType<TextComponent>();
		revive_text->SetVisible(false);
	}

	//get all 3 waypoints (there's 3 because communication task requires 3 of them)
	

	test = Scene.FindObject("DoubleShotVendingMachine");

	subscribe_collision_event();
	SetBrainSize(SMTypes::TOTAL_SM);

	//AddState(&Player::IdleInit, &Player::IdleUpdate, nullptr, "Idle", SMTypes::COMBAT_SM);
	AddState(&Player::MoveInit, &Player::MoveUpdate, nullptr, "Move", SMTypes::COMBAT_SM);
	SetMainState("Move", 0);
	AddState(&Player::DamagedInit, &Player::DamagedUpdate, &Player::DamagedEnd, "Damaged", SMTypes::COMBAT_SM);
	AddState(&Player::InteractingInit, &Player::InteractingUpdate, &Player::InteractingShutDown, "Interacting", SMTypes::COMBAT_SM);
	AddState(&Player::DieInit, &Player::DieUpdate, nullptr, "Die", SMTypes::COMBAT_SM);
	AddState(&Player::ReviveInit, &Player::ReviveUpdate, nullptr, "Revive", SMTypes::COMBAT_SM);
	AddState(&Player::AimInit, &Player::AimUpdate, nullptr, "Aim", SMTypes::AIMING_SM);
	AddState(&Player::Silent, nullptr, nullptr, "Silent", SMTypes::VOICES_SM);
	AddState(nullptr, &Player::HealUpdate, nullptr, "Heal", SMTypes::HEAL_SM);
	AddState(nullptr, &Player::CheatCodes, nullptr, "Cheats", SMTypes::CHEATS_SM);

	serverItIsI = NetworkingMrg.AmIServer();

	mbDead = false;
	base_player = true;

	//guarrada para conseguir mi id (Expected format: Player#)
	std::string name = mOwner->GetName();
	if (name.find("Player") != std::string::npos && name.length() >= 7) {
		name.erase(0, 6); // "Player" lenght == 6
		id = stoi(name);
	}
	else {
		std::cerr << "Issues getting the id of the player for State Machines" << std::endl;
		id = 0;
	}

	if (id == NetworkingMrg.get_id())
	{
		base_player = true;
		mCameraComp->render = base_player;
		mPlayerCameraComp->base_player = base_player;
	}
	else
	{
		base_player = false;
		mCameraComp->render = base_player;
		mPlayerCameraComp->base_player = base_player;
	}

	if (mRigidBody) {
		PlayerGravity = glm::vec3(0.0F, -200.0f, 0.0F);
		mRigidBody->mGravity = glm::vec3(0.0F, -9.8f, 0.0F);
		firtcoll = true;
	}

	init_fov = mCameraComp->GetAngle();

	icons.clear();

	//GET THE ICONS FROM THE HUD
	if (base_player)
	{
		//auto iconsTag = mOwner->GetSpace()->FindObjects(Tags::Icon);

		auto& spaces = Scene.GetSpaces();
		
		if (Scene.FindObject("Icon1", spaces[1]))
			icons.push_back(Scene.FindObject("Icon1", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon2", spaces[1]))
			icons.push_back(Scene.FindObject("Icon2", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon3", spaces[1]))
			icons.push_back(Scene.FindObject("Icon3", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon4", spaces[1]))
			icons.push_back(Scene.FindObject("Icon4", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon5", spaces[1]))
			icons.push_back(Scene.FindObject("Icon5", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon6", spaces[1]))
			icons.push_back(Scene.FindObject("Icon6", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon7", spaces[1]))
			icons.push_back(Scene.FindObject("Icon7", spaces[1])->GetComponentType<IconLogic>());
		if (Scene.FindObject("Icon8", spaces[1]))
			icons.push_back(Scene.FindObject("Icon8", spaces[1])->GetComponentType<IconLogic>());

		for (int i = 0; i < 3; ++i)
		{
			std::string current = "waypoint";
			current += std::to_string(i + 1);
			mWaypoints[i].first = Scene.FindObject(current.c_str());
			if (mWaypoints[i].first)
				mWaypoints[i].second = mWaypoints[i].first->GetComponentType<renderable>();
		}
	}
}

void Player::StateMachineShutDown()
{
	if (gameOver != nullptr)
	{
		Scene.DestroyObject(gameOver);
		gameOver = nullptr;
	}
}

void Player::ToJson(nlohmann::json& j) const
{
	j["MoveSpeed"] << MoveSpeed;
	j["SprintSpeed"] << RunSpeed;
	j["SlopeGravity"] << mGravitySlope;

	j["SprintCooldown"] << sprintCooldown;
	j["SprintLimit"] << sprintLimit;
	j["JumpForce"] << jumpForce;
	j["StunTime"] << stun_time;
	j["StunDisplacement"] << stun_displacement;

	j["JumpSpeed"] << Jump_speed;
	j["AirSpeed"] << Air_speed;

	j["Run_Fov"] << run_fov;

	j["heal_speed"] << heal_speed;
	j["damaged_cooldown"] << damaged_cooldown;
}

void Player::FromJson(nlohmann::json& j)
{
	if (j.find("MoveSpeed") != j.end())
		j["MoveSpeed"] >> MoveSpeed;

	if (j.find("SprintSpeed") != j.end())
		j["SprintSpeed"] >> RunSpeed;

	if (j.find("SlopeGravity") != j.end())
		j["SlopeGravity"] >> mGravitySlope;

	if (j.find("SprintCooldown") != j.end())
		j["SprintCooldown"] >> sprintCooldown;

	if (j.find("SprintLimit") != j.end())
		j["SprintLimit"] >> sprintLimit;

	if (j.find("JumpForce") != j.end())
		j["JumpForce"] >> jumpForce;

	if (j.find("StunTime") != j.end())
		j["StunTime"] >> stun_time;

	if (j.find("StunDisplacement") != j.end())
		j["StunDisplacement"] >> stun_displacement;

	if (j.find("JumpSpeed") != j.end())
		j["JumpSpeed"] >> Jump_speed;

	if (j.find("AirSpeed") != j.end())
		j["AirSpeed"] >> Air_speed;

	if (j.find("Run_Fov") != j.end())
		j["Run_Fov"] >> run_fov;

	if (j.find("heal_speed") != j.end())
		j["heal_speed"] >> heal_speed;

	if (j.find("damaged_cooldown") != j.end())
		j["damaged_cooldown"] >> damaged_cooldown;
}

void Player::OnCollisionStarted(const Collider* coll)
{
	if (firtcoll)
	{
		mRigidBody->mGravity = PlayerGravity;
		firtcoll = false;
	}

	//fly enemy projectile
	if (coll->GetOwner()->mTag == Tags::EnemyBullet)
	{
		ChangeState("Damaged", Player::SMTypes::COMBAT_SM);
	}

	if (!mRigidBody)
		return;

	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			on_ramp = true;
	}
}

void Player::OnCollisionPersisted(const Collider* coll)
{
	if (coll->mShape == shape::OBB)
	{
		if (coll->mOwner->mTag == Tags::Ramp)
			on_ramp = true;
	}
}

void Player::OnCollisionEnded(const Collider* coll)
{
	if (!mRigidBody)
		return;

	//if (coll->mShape == shape::OBB)
	//{
	//	if (coll->mOwner->mTag == Tags::Ramp)
	//		mRigidBody->mDrag = glm::vec3(0.99f);
	//}
}

#ifdef EDITOR

bool Player::Edit()
{
	ImGui::Text("MOVEMENT");
	ImGui::DragFloat("MoveSpeed", &MoveSpeed, 0.02F);
	ImGui::DragFloat("Slope Gravity", &mGravitySlope);

	ImGui::Separator();
	ImGui::Text("SPRINT");
	ImGui::DragFloat("SprintSpeed", &RunSpeed, 0.02F);
	ImGui::DragFloat("SprintCooldown", &sprintCooldown, 0.02F);
	ImGui::DragFloat("SprintLimit", &sprintLimit, 0.02F);
	ImGui::DragFloat("Run Fov", &run_fov, 0.1f);

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

void Player::MoveInit()
{
}
void Player::ComputeWaypointDirection(std::pair<GameObject*, renderable*> waypoint, GameObject* current_task)
{

	const float dist = 120.0f;
	float distsq = dist * dist;
	auto vec = current_task->mTransform.mPosition - mOwner->mTransform.mPosition;
	float current_dist = glm::length2(vec);

	if (current_dist <= distsq)
	{
		waypoint.second->SetVisible(false);
		return;
	}

	waypoint.second->SetVisible(true);

	auto obj_pos = current_task->mTransform.mPosition;

	auto proj = mCameraComp->GetW2Proj() * glm::vec4(obj_pos, 1.0f);

	vec = glm::normalize(obj_pos - mOwner->mTransform.mPosition);

	auto view = glm::normalize(mOwner->mTransform.mViewVector);
	auto right = glm::normalize(mOwner->mTransform.mRightVector);

	float dotfw = (glm::dot(view, vec));
	float anglefw = glm::acos(dotfw);

	float dotr = (glm::dot(right, vec));
	float angler = glm::acos(dotr);

	if ((angler - glm::half_pi<float>()) <= glm::epsilon<float>()) anglefw *= -1.0f;

	glm::quat quaternion = glm::angleAxis(anglefw, glm::vec3(0, 0, 1));

	auto mat = glm::toMat4(quaternion);
	glm::vec3 rotated = glm::normalize(mat * glm::vec4(0, 1, 0, 0));

	glm::vec2 result = lerp(glm::vec2(waypoint.first->mTransform.mPosition.x, waypoint.first->mTransform.mPosition.y), glm::vec2(rotated.x, rotated.y) * 200.0f, 0.3f);

	waypoint.first->mTransform.mPosition.x = result.x;
	waypoint.first->mTransform.mPosition.y = result.y;

	rotated *= 2000;
	rotated.z = waypoint.first->mTransform.mPosition.z;
	waypoint.first->mTransform.LookAt(rotated);
}

void Player::Waypoint()
{
	if (InLobby) return;

	auto task_vec = TaskSys.activetasks;
	if (task_vec.empty())
	{
		for(int i = 0; i < 3; ++i)
		mWaypoints[i].second->SetVisible(false);
		return;
	}
	auto current_task = task_vec[0];
	if (!current_task) return;

	if (current_task->tag == TaskTag::Communication)
	{
		for (int i = 0; i < 3; ++i)
		{
			if(!task_vec[i]->correct)
				ComputeWaypointDirection(mWaypoints[i], task_vec[i]->mOwner);
		}
	}
	else
	{
		mWaypoints[1].second->SetVisible(false);
		mWaypoints[2].second->SetVisible(false);
		ComputeWaypointDirection(mWaypoints[0], current_task->mOwner);
	}

}

void Player::MoveUpdate()
{
	if(on_ramp)
		mRigidBody->mDrag = glm::vec3(0.0F);
	else
		mRigidBody->mDrag = glm::vec3(0.99f);
	on_ramp = false;

	if (base_player)
	{
		updateInput();

		Waypoint();
	}
	//update the jump
	if (mbJumping)
	{
		mbIdle = false;

		if (Direction != glm::vec3(0.0f))
			current_jump_speed = dir_before_jump * Jump_speed + Direction * Air_speed;
		if (mbSprinting)
			current_jump_speed *= 1.4f;
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
		if (sprintTime <= cooldown)
			sprintTime += FRC.GetFrameTime();
		else
		{
			mbSprinting = false;
			cooldown = 0.0f;
		}
			
	}
	else
	{
		mbWalking = true;
		sprintTime = 0.0F;
		if(cooldown < sprintLimit)
			cooldown += FRC.GetFrameTime();
	}

	if (!mbAiming)
	{
		int sign = mbSprinting ? 1 : -1;

		lerp_fov_dt += FRC.GetFrameTime() * fov_speed * sign;
		if (lerp_fov_dt < 0)	lerp_fov_dt = 0.0f;
		if (lerp_fov_dt > 1)	lerp_fov_dt = 1.0f;
		mCameraComp->SetAngle(lerp(init_fov, run_fov, lerp_fov_dt));
	}
	else
	{
		lerp_fov_dt = 0.0f;
	}
		
}

//------------AIMING-----------
void Player::AimInit()
{

}

void Player::AimUpdate()
{
	mCameraComp->SetPosition(mOwner->mTransform.mPosition);
}

//------------DAMAGED-----------
void Player::DamagedInit()
{
	if (mbDead) return;

	stun_dt = 0.0f;
	if(!immortal)
		mHealth->GetDamaged(20.0f);
	hit_direction.y = 0.0f;

	if (hit_direction != glm::vec3(0.0f))
		hit_direction = glm::normalize(hit_direction);


	if (mHealth->getCurrentHealth() > 0.0f)
		mRigidBody->AddForce(hit_direction * stun_displacement);

	//stun_new_pos = mOwner->mTransform.mPosition + hit_direction * stun_displacement;
	mbDamaged = true;
	damaged_timer = 0.0f;
	mRigidBody->mDrag = glm::vec3(0.80f); 
	if(mEmitter && base_player)
		mEmitter->PlayCue("./../Resources/Audio/damage.mp3", 1.0f, false, false, false);
}

void Player::DamagedUpdate()
{
	stun_dt += FRC.GetFrameTime();

	//mOwner->mTransform.mPosition = lerp(mOwner->mTransform.mPosition, stun_new_pos, 0.25f);

	//float dist = glm::length2((mOwner->mTransform.mPosition - stun_new_pos));

	if (mHealth->getCurrentHealth() <= 0.0f)
		ChangeState("Die");
		
		
	else if(stun_dt >= stun_time)
		ChangeState("Move");
}

void Player::DamagedEnd()
{
	mRigidBody->mDrag = glm::vec3(0.99f);
}

void Player::DieInit()
{
	LosePowerUps();

	mbDead = true;
	time_dead = 0.0f;

	//assuming that the first collider is the one in the feet
	collOffset = playercolls[0]->mOffset;
	init_pos = mOwner->mTransform.mPosition;
	playercolls[0]->mOffset = glm::vec3(0.0f);
	playercolls[1]->mbGhost = true;

	InteractComp->Activate(true);

	if (serverItIsI) {
		network_event e = NetworkingMrg.CreateNetEvent(id, event_type::player_dead);
		NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
	}

	//play dead souind
	if (base_player)
	{
		mEmitter->PlayCue("./../Resources/Audio/player_death.wav", 1.0f, false, false, true);
		revive_text->SetText("Waiting For Revive");
		revive_text->SetVisible(true);
	}

}

void Player::DieUpdate()
{
	time_dead += FRC.GetFrameTime();

	if (InteractComp->mbInteracting)
	{
		time_reviving += FRC.GetFrameTime();
		InteractComp->SetBar(time_reviving / timer_to_revive);

		if (time_reviving >= timer_to_revive)
		{
			playercolls[0]->mOffset = collOffset;
			playercolls[1]->mbGhost = false;
			mOwner->mTransform.mPosition = init_pos;

			InteractComp->StopInteracting(false);
			if (base_player)
				revive_text->SetVisible(false);

			network_event e = NetworkingMrg.CreateNetEvent(id, event_type::respawn);
			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}
	}
	else
		time_reviving = 0.0f;
}

//Base Player preemptively apply their movement input locallyand will send said input data to the server.
//---------Interacting---------
void Player::InteractingInit()
{
	interacting = true;
	interaction_timer = 0.0f;
	ChosenObject = interaction_text->chosen_object;

	if (!ChosenObject || ChosenObject->mType == InteractionComp::Type::WeaponMachine && (mWeaponComp->changing_gun || mWeaponComp->reloading))
	{
		ChangeState("Move");
		return;
	}
	if (!mWeaponComp->weapong_hiding && ChosenObject && ChosenObject->mType != InteractionComp::Type::WeaponMachine)
		mWeaponComp->ChangeState("Hide", Weapon::SMTypes::AIMING);
	interaction_text->Interact();
}

void Player::InteractingUpdate()
{
	if ((!interacting || KeyUp(Key::E)))
		ChangeState("Move");
}

void Player::InteractingShutDown()
{
	if (base_player)
		hud_bar->Reset();
	interacting = false;
	if (mWeaponComp->weapong_hiding && ChosenObject && ChosenObject->mType != InteractionComp::Type::WeaponMachine)
	{
		mWeaponComp->ChangeState("Hide", Weapon::SMTypes::AIMING);
		mWeaponComp->ReInitState("Hide", Weapon::SMTypes::AIMING);
	}
	interaction_text->StopInteracting();
	ChosenObject = nullptr;
}

void Player::ReviveInit()
{
	mbDead = false;
	mHealth->RestoreHealthMax();

	//assuming that the first collider is the one in the feet
	playercolls[0]->mOffset = collOffset;
	playercolls[1]->mbGhost = false;

	mOwner->mTransform.mPosition.y = init_pos.y;
}

void Player::ReviveUpdate()
{
	ChangeState("Move");
}

void Player::updateInput()
{
	if (mbDead)
		return;

	bool reloading = mWeaponComp->reloading;
	bool weapon_hided = mWeaponComp->weapon_hided;
	bool can_reload = mWeaponComp->CanReload();
	bool recoiling = mWeaponComp->recoiling;
	bool changing_gun = mWeaponComp->changing_gun;
	bool bullets_left = mWeaponComp->bullet_count > 0;

	move_input input;
	input.info = static_cast<move_input::player_info>(move_input::empty);

	int temp = static_cast<int>(input.info);

	if (MouseWheel() && SecondWeapon != nullptr && !mWeaponComp->changing_gun)
	{
		if(mbSprinting)
			cooldown = 0.0f;
		mbSprinting = false;
		mWeaponComp->ChangeState("NotAim", Weapon::SMTypes::AIMING);
		mWeaponComp->changing_gun = true;
	}

	//============================ move forward ==============================================
	if (KeyDown(Key::W)) {
		temp |= static_cast<int>(move_input::forward);
	}

	//============================ move backforward ==========================================
	if (KeyDown(Key::S)) {
		temp |= static_cast<int>(move_input::back);
	}

	//============================ move to the left ==========================================
	if (KeyDown(Key::A)) {
		temp |= static_cast<int>(move_input::left);
	}

	//============================ move to the right =========================================
	if (KeyDown(Key::D)) {
		temp |= static_cast<int>(move_input::right);
	}

	//============================ jump ======================================================
	if (KeyTriggered(Key::Space))
			temp |= static_cast<int>(move_input::space);

	//============================ Task ======================================================
	if (interaction_text && interaction_text->CheckInteract())
	{
		if(!interaction_text->chosen_object->mbTriggerAction && KeyDown(Key::E))
			temp |= static_cast<int>(move_input::interact);
		else if(interaction_text->chosen_object->mbTriggerAction && KeyTriggered(Key::E))
			temp |= static_cast<int>(move_input::interact);
	}
		
	//============================ sprint ====================================================
	if (KeyTriggered(Key::LShift) && !changing_gun)
		temp |= static_cast<int>(move_input::sprint);
	if (mbSprinting)
	{
		if (KeyReleased(Key::LShift) || KeyReleased(Key::W))
		{
			cooldown = 0.0f;
			mbSprinting = false;
		}
	}
	else
	{
		mbWalking = true;
		sprintTime = 0.0F;
	}

	//============================ shoot =====================================================
	if (!reloading && !recoiling && (bullets_left || can_reload)) 
	{
		if ((MouseDown(MouseKey::LEFT) && mWeaponComp->automatic_weapon) || (MouseTriggered(MouseKey::LEFT) && !mWeaponComp->automatic_weapon))
		{
			temp |= static_cast<int>(move_input::shoot);
			mbShooting = true;
		}
	}
	else
		mbShooting = false;

	input.angle = mPlayerCameraComp->angle;

	//============================ Aim & Reload ==============================================
	if (!reloading && base_player && !weapon_hided && !changing_gun)
	{
		if (MouseDown(MouseKey::RIGHT))
		{
			if (mbSprinting)
				cooldown = 0.0f;
			mbSprinting = false;
			mbAiming = true;
			mWeaponComp->ChangeState("Aim", Weapon::SMTypes::AIMING);
		}
		else {
			mbAiming = false;
			mWeaponComp->ChangeState("NotAim", Weapon::SMTypes::AIMING);
		}
	}

	if (KeyDown(Key::R) && !reloading && base_player && !weapon_hided && can_reload)
	{
		if (mbSprinting)
			cooldown = 0.0f;
		mbSprinting = false;
		temp |= static_cast<int>(move_input::reload);
	}

	//============================ send data to server =======================================
	input.info = static_cast<move_input::player_info>(temp);

	if (input.info != move_input::empty) {
		input.id = id;
		moveUpdate(input);
		input.pos = mOwner->mTransform.mPosition;
		NetworkingMrg.sendPacketToServer(Flag::e_move_input, input);

		mbIdle = false;
	}
	else if(!mbJumping)
		mbIdle = true;
}

void Player::moveUpdate(move_input& p)
{
 	bool reloading = mWeaponComp->reloading;
	bool weapon_hided = mWeaponComp->weapon_hided;
	bool can_reload = mWeaponComp->CanReload();

	if (mbDead)
		return;

	Direction = {};
	float speed = MoveSpeed;
	//============================ move forward ==============================================
	if (p.info & move_input::forward)
	{
		Direction += mMovementFor;
		if ((p.info & move_input::sprint) && !mbSprinting && !mbAiming && !mbShooting && !reloading) {
			if (sprintTime <= cooldown)
			{
				sprintTime += FRC.GetFrameTime();
				mbSprinting = true;
				mbWalking = false;
			}
				
		}
		if(mbSprinting) speed = RunSpeed;
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
	mPlayerCameraComp->angle = p.angle; 
	mPlayerCameraComp->UpdateVectors();

	if (p.info & move_input::shoot && !reloading && !weapon_hided)
	{
		if (mbSprinting)
			cooldown = 0.0f;
		mbSprinting = false;
		shoot();

		if (base_player) {
			network_event e = NetworkingMrg.CreateNetEvent(p.id, event_type::shoot);
			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}
	}

	//============================ Task ======================================================
	if (p.info & move_input::interact)
	{
		interactEvent();

		if (base_player) {
			network_event e = NetworkingMrg.CreateNetEvent(p.id, event_type::interact);
			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}
	}

	//============================ reload ====================================================
	if (p.info & move_input::reload) {
		if (mbSprinting)
			cooldown = 0.0f;
		mbSprinting = false;
		mWeaponComp->ChangeState("NotAim", Weapon::SMTypes::AIMING);
		mWeaponComp->reloading = true;
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

		//KILL IF THEY FALL
	if (mOwner->mTransform.mPosition.y < -2000.0f)
		ChangeState("Die");
}

void Player::shoot()
{
	if (mbDead) return;
	mWeaponComp->ChangeState("Shoot", Weapon::SMTypes::SHOOTING);
}

void Player::interactEvent()
{
	ChangeState("Interacting", SMTypes::COMBAT_SM);
	//if (TaskSys.actualtask && TaskSys.actualtask->IsInteractible())
	//{
	//	float distance = glm::length(mOwner->mTransform.mPosition - TaskSys.actualtask->mOwner->mTransform.mPosition);
	//	if (distance < TaskSys.actualtask->interaction_range)
	//		ChangeState("Interacting", SMTypes::COMBAT_SM);//joder
	//}
}

void Player::GameOverEvent()
{
	if (!gameOver)
	{
		gameOver = Scene.CreateObject(gameOver, Scene.GetSpace("HUD"));
		serializer.LoadArchetype("GameOver", gameOver);
		gameOver->mTransform.mPosition = glm::vec3(0.0f, 0.0f, -48.0f);
	}
}

void Player::HealUpdate()
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

void Player::CheatCodes()
{
	if (GSM.mConfig.mbCheats)
	{
		if (KeyDown(Key::U) && KeyTriggered(Key::Num1))
			immortal = !immortal;

		if (KeyDown(Key::U) && KeyTriggered(Key::Num2))
			money += 666;

		if (KeyDown(Key::U) && KeyTriggered(Key::Num3))
			fly = !fly;

		if (fly)
			mRigidBody->mGravity = glm::vec3(0.0f);
		if (!fly)
			mRigidBody->mGravity = glm::vec3(0.0f, -200.0f, 0.0f);
	}
}

void Player::ChangeWeapon()
{
	if (SecondWeapon == nullptr && new_weapon == nullptr) return;
	
	if (SecondWeapon != nullptr)
	{
		if (new_weapon != nullptr)
		{
			mOwner->GetSpace()->DestroyObject(mWeapon);
			mWeapon = new_weapon;
		}
		else
		{
			auto* temp_weapon = mWeapon;
			mWeapon = SecondWeapon;
			SecondWeapon = temp_weapon;
		}
	}
	else if(new_weapon != nullptr)
	{
		auto* temp_weapon = mWeapon;
		mWeapon = new_weapon;
		SecondWeapon = temp_weapon;
	}

	mWeapon->GetComponentType<renderable>()->SetVisible(true);
	mWeaponComp = mWeapon->GetComponentType<Weapon>();
	mWeaponComp->changing_gun = true;
	mWeaponComp->gun_changed = true;
	mWeaponComp->ChangeState("Hide");
	new_weapon = nullptr;
}

void Player::buy_weapon(std::string weapon_name, unsigned weapon_cost)
{
	GameObject* newWeapon = mOwner->GetSpace()->CreateObject();
	mOwner->CreateChild(newWeapon);
	serializer.LoadArchetype(weapon_name.c_str(), newWeapon);
	newWeapon->GetComponentType<Weapon>()->changing_gun = true;
	newWeapon->GetComponentType<Weapon>()->gun_changed = true;
	newWeapon->GetComponentType<Weapon>()->ChangeState("Hide");

	if (!base_player)
		newWeapon->mTransform.mScale = glm::vec3(0.04f);

	new_weapon = newWeapon;
	money -= (unsigned)weapon_cost;
	mWeaponComp->ChangeState("WeaponChange");
}

void Player::LosePowerUps()
{
	for (auto i : icons)
		dynamic_cast<IconLogic*>(i)->ChangeIcon(VendingMachine::VM_type::AMMO);
	//RESET HEALTH
	mHealth->DecreaseMaxHealth(float(hp_increase * health_pu));
	//RESET AGILITY
	reload_vel = 1;
	//RESET DAMAGE
	damage_bonification = 1;
	//RESET FAST REVIVE
	fast_revive = false;

	//RESET STATS
	health_pu = 0;
	agility_pu = 0;
	doubleshot_pu = 0;
	total_pu = 0;
}