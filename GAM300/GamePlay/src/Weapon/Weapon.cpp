#include "Weapon.h"
#include "Serializer/Factory.h"
#include "Utilities/FrameRateController/FrameRateController.h"
#include "Utilities/Input/Input.h"
#include "Health/Health.h"
#include "Player/PlayerCamera.h"
#include "Player/Player.h"
#include "Graphics/Camera/Camera.h"
#include "Enemies/DamageZones.h"
#include "UtilityComponents/FadeOut.h"
#include "AudioManager/Audio.h"
#include "Networking\Networking.h"

#ifdef EDITOR
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

#ifdef EDITOR
bool Weapon::Edit()
{
	bool changed = false;

	ImGui::Checkbox("Automatic Weapon", &automatic_weapon);
	ImGui::DragFloat("Damage", &gun_damage, 0.5f, 0.0f, 300);
	ImGui::DragFloat("TargetFar", &TargetFar, 0.1f, -1000.0f, 1000.0f);
	ImGui::DragFloat("AimSpeed", &aim_speed, 0.1f, 1.0f, 50.0f);
	ImGui::DragFloat("FinalFov", &aim_fov, 0.1f, 1.0f, 180.0f);
	ImGui::DragFloat("CoolDown", &CoolDown, 0.01f, 0.01f, 10.0f);
	
	ImGui::Separator();
	ImGui::Text("BULLETS");
	ImGui::DragInt("Charger Size", &charger_size, 1, 1, 200);
	ImGui::DragInt("Total Bullets", &total_bullets, static_cast<float>(charger_size), charger_size, 1000);
	ImGui::DragFloat("BulletSpeed", &BulletSpeed, 5.0f, 10.0f, 2000.0f);
	max_bullets = total_bullets;

	ImGui::Separator();
	ImGui::Text("RECOIL");
	ImGui::DragFloat("RecoilDistance", &recoil_distance, 0.01f, 0.0f, 2.0f);
	ImGui::DragFloat2("RecoilDistorsion", &recoil_distorsion[0], 0.01f, 0.0f, 10.0f);

	ImGui::DragFloat("Recoil Back Velocity", &recoil_back_velocity, 0.01f, 0.01f, 1000.0f);
	ImGui::DragFloat("Recoil Front Velocity", &recoil_front_velocity, 0.01f, 0.01f, 1000.0f);
	ImGui::DragFloat("Recoil Gun Angle", &recoil_gun_angle, 0.01f, 0.01f, 90.0f);

	ImGui::Separator();
	ImGui::Text("MIRILLA");
	ImGui::DragFloat("Mirilla Iddle Offset", &initial_distance, 0.01f, 0.1f, 50.0f);
	ImGui::DragFloat("Mirilla Moving Offset", &end_distance, 0.01f, 0.1f, 50.0f);

	ImGui::Separator();
	ImGui::Text("RELOAD");
	ImGui::DragFloat("Gun Hide Speed", &hide_speed, 0.01f, 1.0f, 100.0f);
	ImGui::DragFloat("Time Reloading", &time_to_reload, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Total Angle To Reload", &angle_to_hide, 0.01f, -180.0f, 180.0f);
	ImGui::Checkbox("Infinity Ammo", &infinity_ammo);

	return changed;
}
#endif

void Weapon::StateMachineInitialize()
{
	SetBrainSize(SMTypes::TOTAL_SM);
	AddState(&Weapon::AimInit,		 &Weapon::AimUpdate,	   &Weapon::AimShutDown, "Aim", SMTypes::AIMING);
	AddState(&Weapon::NotAimingInit, &Weapon::NotAimingUpdate, &Weapon::NotAimingShutDown, "NotAim", SMTypes::AIMING);
	AddState(&Weapon::ReloadInit, &Weapon::ReloadUpdate, &Weapon::ReloadShutDown, "Reload", SMTypes::AIMING);
	AddState(&Weapon::ChangeGunInit, &Weapon::ChangeGunUpdate, nullptr, "WeaponChange", SMTypes::AIMING);
	AddState(&Weapon::HideInit, &Weapon::HideUpdate, &Weapon::HideShutDown, "Hide", SMTypes::AIMING);

	AddState(&Weapon::ShootInit, &Weapon::ShootUpdate, nullptr, "Shoot", SMTypes::SHOOTING);
	AddState(&Weapon::IdleInit, &Weapon::IdleUpdate, nullptr, "Idle", SMTypes::SHOOTING);

	SetMainState("NotAim", SMTypes::AIMING);
	SetMainState("Idle", SMTypes::SHOOTING);

	mTop = Scene.FindObject("mirillatop");
	mBot = Scene.FindObject("mirillabot");
	mLeft = Scene.FindObject("mirillaleft");
	mRight = Scene.FindObject("mirillaright");

	mPlayer = mOwner->GetParent();
	mPlayerInfo = mPlayer->GetComponentType<Player>();
	mCamera = mPlayer->GetComponentType<CameraComp>();
	mEmitter = mOwner->GetComponentType<SoundEmitter>();
	ShootingPos = mOwner->FindChild("ShootingPos");
	AimPos = mOwner->FindChild("AimPos");
	HitMarker = Scene.FindObject("hitmarker");
	mEmitter = mPlayer->GetComponentType<SoundEmitter>();

	aim_position = (-AimPos->mTransform.mOffset);
	init_position = mOwner->mTransform.mOffset;
	init_recoilpos = init_position;
	final_recoilpos = init_position - glm::vec3(0.0f, 0.0f, -recoil_distance);

	init_fov = mCamera->GetAngle();

	my_id = NetworkingMrg.get_id();

	bullet_count = charger_size;
}

void Weapon::ToJson(nlohmann::json& j) const
{
	j["TargetFar"] << TargetFar;
	j["AimSpeed"] << aim_speed;
	j["CoolDown"] << CoolDown;
	j["BulletSpeed"] << BulletSpeed;
	j["RecoilDistance"] << recoil_distance;
	j["recoil_back_velocity"] << recoil_back_velocity;
	j["recoil_front_velocity"] << recoil_front_velocity;
	j["recoil_gun_angle"] << recoil_gun_angle;
	j["RecoilDistorsion"] << recoil_distorsion;
	j["FinalFov"] << aim_fov;
	j["initial_distance"] << initial_distance;
	j["end_distance"] << end_distance;
	j["charger_size"] << charger_size;
	j["total_bullets"] << total_bullets;
	j["reload_speed"] << hide_speed;
	j["angle_to_reload"] << angle_to_hide;
	j["time_to_reload"] << time_to_reload;
	j["gun_damage"] << gun_damage;
	j["infinity_ammo"] << infinity_ammo;
	j["automatic_weapon"] << automatic_weapon;
}

void Weapon::FromJson(nlohmann::json& j)
{
	if (j.find("TargetFar") != j.end())
		j["TargetFar"] >> TargetFar;
	if (j.find("AimSpeed") != j.end())
		j["AimSpeed"] >> aim_speed;
	if (j.find("CoolDown") != j.end())
		j["CoolDown"] >> CoolDown;
	if (j.find("BulletSpeed") != j.end())
		j["BulletSpeed"] >> BulletSpeed;
	if (j.find("RecoilDistance") != j.end())
		j["RecoilDistance"] >> recoil_distance;
	if (j.find("recoil_back_velocity") != j.end())
		j["recoil_back_velocity"] >> recoil_back_velocity;
	if (j.find("recoil_front_velocity") != j.end())
		j["recoil_front_velocity"] >> recoil_front_velocity;
	if (j.find("recoil_gun_angle") != j.end())
		j["recoil_gun_angle"] >> recoil_gun_angle;
	if (j.find("RecoilDistorsion") != j.end())
		j["RecoilDistorsion"] >> recoil_distorsion;
	if (j.find("FinalFov") != j.end())
		j["FinalFov"] >> aim_fov;
	if (j.find("initial_distance") != j.end())
		j["initial_distance"] >> initial_distance;
	if (j.find("end_distance") != j.end())
		j["end_distance"] >> end_distance;

	if (j.find("charger_size") != j.end())
		j["charger_size"] >> charger_size;
	if (j.find("total_bullets") != j.end())
		j["total_bullets"] >> total_bullets;
	if (j.find("reload_speed") != j.end())
		j["reload_speed"] >> hide_speed;
	if (j.find("angle_to_reload") != j.end())
		j["angle_to_reload"] >> angle_to_hide;
	if (j.find("time_to_reload") != j.end())
		j["time_to_reload"] >> time_to_reload;
	if (j.find("gun_damage") != j.end())
		j["gun_damage"] >> gun_damage;
	if (j.find("infinity_ammo") != j.end())
		j["infinity_ammo"] >> infinity_ammo;
	if (j.find("automatic_weapon") != j.end())
		j["automatic_weapon"] >> automatic_weapon;
}

void Weapon::AimInit()
{
	aiming_distance = mTop->mTransform.mPosition.y;
	mPlayerInfo->mbAiming = true;
}

void Weapon::AimUpdate()
{
	if (lerp_dt < 1)
	{
		lerp_dt += FRC.GetFrameTime() * aim_speed;
		if (lerp_dt > 1)
			lerp_dt = 1.0f;

		mCamera->SetAngle(lerp(init_fov, aim_fov, lerp_dt));
		mOwner->mTransform.mOffset = lerp(init_position, aim_position, lerp_dt);

		init_recoilpos = mOwner->mTransform.mOffset;
		final_recoilpos = init_recoilpos - glm::vec3(0.0f, 0.0f, -recoil_distance);

		float sight_pos = lerp(aiming_distance, 0, lerp_dt);
		float sight_alpha = lerp(1.0f, 0.0f, lerp_dt);
		UpdateSight(sight_pos, sight_alpha);
	}
	
	target = mPlayer->mTransform.mPosition + mPlayer->mTransform.mViewVector * TargetFar;
	mOwner->mTransform.mPosition = mPlayer->mTransform.TransRotMtx() * glm::vec4(mOwner->mTransform.mOffset, 1.0f);
	mOwner->mTransform.LookAt(target);
}

void Weapon::AimShutDown()
{
}

void Weapon::NotAimingInit()
{
}

void Weapon::NotAimingUpdate()
{
	
	//set initial values
	ResetGunValues();


	if (lerp_dt <= 0)
	{
		if (reloading)
		{
			ChangeState("Reload");
			return;
		}
		else if (changing_gun)
		{
			ChangeState("WeaponChange");
			return;
		}
	}

	int sign = mPlayerInfo->mbIdle ? -1 : 1;

	if (lerp_dt > 0)
	{
		lerp_dt -= FRC.GetFrameTime() * aim_speed;
		if (lerp_dt < 0)
		{
			lerp_dt = 0.0f;
			mPlayerInfo->mbAiming = false;
		}
		mCamera->SetAngle(lerp(init_fov, aim_fov, lerp_dt));
		mOwner->mTransform.mOffset = lerp(init_position, aim_position, lerp_dt);

		init_recoilpos = mOwner->mTransform.mOffset;
		final_recoilpos = init_recoilpos - glm::vec3(0.0f, 0.0f, -recoil_distance);

		float init = sign < 0 ? initial_distance : end_distance;
		float sight_pos = lerp(init, 0, lerp_dt);
		float sight_alpha = lerp(1.0f, 0.0f, lerp_dt);
		UpdateSight(sight_pos, sight_alpha);
	}

	else
	{
		if (!weapon_hided)
		{
			glm::vec3 runing_pos = init_position + glm::vec3(0.0f, 0.0f, 1.5f);
			mOwner->mTransform.mOffset = lerp(init_position, runing_pos, mPlayerInfo->lerp_fov_dt);
		}
		//move the weapon
		

		sight_dt += FRC.GetFrameTime() * sign * 10.0f;
		if (sight_dt < 0) sight_dt = 0.0f;
		else if (sight_dt > 1) sight_dt = 1.0f;

		float sight_pos = lerp(initial_distance, end_distance, sight_dt);
		UpdateSight(sight_pos);
	}
}

void Weapon::NotAimingShutDown()
{
}

void Weapon::ShootInit()
{
}

void Weapon::ShootUpdate()
{
	time_dt += FRC.GetFrameTime();
	if (time_dt < CoolDown)
		return;

	time_dt = 0.0f;
	target = mPlayer->mTransform.mPosition + mPlayer->mTransform.mViewVector * TargetFar;
	mOwner->mTransform.LookAt(target);

	if (bullet_count <= 0 && (total_bullets > 0 || infinity_ammo))
	{
		reloading = true;
		ChangeState("NotAim");
		ChangeState("Idle", SMTypes::SHOOTING);
		return;
	}

	if (mEmitter /* && mPlayerInfo->base_player */)
		mEmitter->PlayCue("./../Resources/Audio/BulletSFX.wav", 0.1f, false, false, false); //play sound

	//call to the shot function that can be defined by child clases
	ShotFunction();
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//recoil
	recoil_upward_distance = glm::linearRand(glm::vec2{ -recoil_distorsion.x, 0.0f }, glm::vec2{ recoil_distorsion.x, recoil_distorsion.y });
	recoiling = true;

	//update bullets
	bullet_count--;
	gun_is_full = false;

	//change to idle
	ChangeState("Idle", SMTypes::SHOOTING);
}

void Weapon::ReloadInit()
{
	if (bullet_count >= charger_size)
	{
		gun_is_full = true;
		reloading = false;
	}
		
	else
		reloading = true;
	reload_timer = 0.0f;
}

void Weapon::ReloadUpdate()
{
	//set initial values
	ResetGunValues();

	if (recoiling)return;

	if (gun_is_full && !infinity_ammo)
	{
		ChangeState("NotAim");
		return;
	}

	if(!weapon_hided)
	{
		ChangeState("Hide");
		return;
	}

	reload_timer += FRC.GetFrameTime();

	if (reload_timer >= time_to_reload / mPlayerInfo->reload_vel)
	{
		int bullets_reloaded = total_bullets < charger_size ? total_bullets - bullet_count : charger_size - bullet_count;
		bullet_count = total_bullets < charger_size ? total_bullets : charger_size;
		if (infinity_ammo) bullet_count = charger_size;
		total_bullets -= bullets_reloaded;
		gun_is_full = true;

		ChangeState("Hide");
		reloading = false;
	}
}

void Weapon::ReloadShutDown()
{
}

void Weapon::ChangeGunInit()
{
	changing_gun = true;
	change_gun_dt = 0.0f;
}

void Weapon::ChangeGunUpdate()
{
	//set initial values
	ResetGunValues();

	if (recoiling)return;

	if (!weapon_hided)
	{
		ChangeState("Hide");
		return;
	}

	change_gun_dt += FRC.GetFrameTime();
	if (change_gun_dt > change_gun_timer && !gun_changed)
	{
		gun_changed = true;
		change_gun_dt = 0.0f;
		//mPlayerInfo->ChangeWeapon();

		if (mPlayerInfo->base_player)
		{
			if (NetworkingMrg.AmIServer())
			{
				network_event e = NetworkingMrg.CreateNetEvent(mPlayerInfo->getId(), event_type::weapon_change);
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			else // (!serverItIsI)
			{
				network_event e = NetworkingMrg.CreateNetEvent(mPlayerInfo->getId(), event_type::weapon_change_request);
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);

			}
		}
	}
}


bool Weapon::CanReload()
{
	if (gun_is_full || (total_bullets <= 0 && !infinity_ammo) || changing_gun) return false;
	return true;
}

void Weapon::HideInit()
{
	weapong_hiding = !weapong_hiding;
	weapon_hided = weapong_hiding ? true : weapon_hided;
	initial_hide_offset = weapong_hiding ? mOwner->mTransform.mOffset : initial_hide_offset;

	if(!weapong_hiding)
		mOwner->GetComponentType<renderable>()->SetVisible(true);
}

void Weapon::HideUpdate()
{
	//set initial values
	ResetGunValues();

	if (recoiling)return;

	int downward_hide = weapong_hiding ? 1 : -1;
	float end_angle = weapong_hiding ? angle_to_hide : 0.0f;

	hide_angle -= FRC.GetFrameTime() * hide_speed * mPlayerInfo->reload_vel * downward_hide;

	if (hide_angle >= 0.0f && !weapong_hiding)				hide_angle = 0.0f;
	if (hide_angle <= angle_to_hide && weapong_hiding)		hide_angle = angle_to_hide;

	//rotate the gun starting from the initial values
	auto mtx = glm::rotate(glm::radians(-30.0f), mOwner->mTransform.mViewVector);
	glm::vec3 right_vec = glm::vec3(mtx * glm::vec4(mOwner->mTransform.mRightVector, 0.0f));

	//move the weapon
	glm::vec3 pivot = mOwner->mTransform.mPosition - mOwner->mTransform.mViewVector * glm::vec3(5.0f);
	mOwner->mTransform.RotateAround(pivot, right_vec, hide_angle);
	mOwner->mTransform.mOffset = glm::inverse(mPlayer->mTransform.TransRotMtx()) * glm::vec4(mOwner->mTransform.mPosition, 1.0f);

	//change state
	if (hide_angle == end_angle)
	{
		weapon_hided = weapong_hiding;
		if(weapon_hided || changing_gun)
			mOwner->GetComponentType<renderable>()->SetVisible(false);

		if (gun_changed && changing_gun)
		{
			gun_changed = false;
			changing_gun = false;
			mOwner->GetComponentType<renderable>()->SetVisible(true);
		}

		if(changing_gun)
			ChangeState("WeaponChange");
		else if (reloading)
			ChangeState("Reload");
		else
			ChangeState("NotAim");
	}
}

void Weapon::HideShutDown()
{

}

void Weapon::IdleInit()
{
	recoiling_backwards = true;
}

void Weapon::IdleUpdate()
{
	if (recoiling)
	{
		float rec_speed = recoiling_backwards ? recoil_back_velocity : -recoil_front_velocity;

		recoil_dt += FRC.GetFrameTime() * rec_speed;

		if (recoil_dt >= 1.0f){
			recoil_dt = 1.0f;
			recoiling_backwards = false;
		}
		if (recoil_dt <= 0.0f) {
			recoil_dt = 0.0f;
			recoiling_backwards = true;
		}

		if(!recoiling_backwards)
			mOwner->GetParent()->GetComponentType<PlayerCamera>()->angle += recoil_upward_distance;
		mOwner->mTransform.mOffset = lerp(init_recoilpos, final_recoilpos, recoil_dt);

		if(!mPlayerInfo->mbAiming)
			mOwner->mTransform.RotateAround(mOwner->mTransform.mRightVector, lerp(0.0f, recoil_gun_angle, recoil_dt));

		if (recoiling_backwards && recoil_dt == 0.0f)
		{
			mOwner->mTransform.mOffset = lerp(init_position, aim_position, lerp_dt);
			recoiling = false;
		}
	}
	else
		recoil_dt = 0.0f;

	time_dt += FRC.GetFrameTime();
}

void Weapon::UpdateSight(float new_distance, float new_alpha)
{
	if (mPlayerInfo->getId() == my_id) {
		auto color = mTop->GetComponentType<renderable>()->GetColor();
		color.a = new_alpha;
		mTop->GetComponentType<renderable>()->SetColor(color);
		mBot->GetComponentType<renderable>()->SetColor(color);
		mLeft->GetComponentType<renderable>()->SetColor(color);
		mRight->GetComponentType<renderable>()->SetColor(color);

		mTop->mTransform.mPosition.y = new_distance;
		mBot->mTransform.mPosition.y = -new_distance;
		mLeft->mTransform.mPosition.x = -new_distance;
		mRight->mTransform.mPosition.x = new_distance;
	}
}

void Weapon::ResetGunValues()
{
	target = mPlayer->mTransform.mPosition + mPlayer->mTransform.mViewVector * TargetFar;
	mOwner->mTransform.mPosition = mPlayer->mTransform.TransRotMtx() * glm::vec4(initial_hide_offset, 1.0f);
	mOwner->mTransform.LookAt(target);
}

void Weapon::ShotFunction()
{
	float sight_dist = mTop->mTransform.mPosition.y;
	glm::vec2 distorision_shot = glm::linearRand(glm::vec2{ -sight_dist, -sight_dist }, glm::vec2{ sight_dist, sight_dist });

	glm::vec3 current_target = glm::vec3(target.x + distorision_shot.x, target.y + distorision_shot.y, target.z);
	geometry::ray _ray({ mPlayer->mTransform.mPosition - mPlayer->mTransform.mViewVector }, { glm::normalize(current_target - mPlayer->mTransform.mPosition) });
	glm::vec3 view = glm::normalize(current_target - ShootingPos->mTransform.mPosition);

	GameObject* bullet = mOwner->GetSpace()->CreateObject();
	serializer.LoadArchetype("Bullet1", bullet);

	bullet->mTransform.mPosition = ShootingPos->mTransform.mPosition + view * bullet->mTransform.mScale.z / 2.0f;
	bullet->mTransform.LookAt(current_target);
	bullet->GetComponentType<Rigidbody>()->mVelocity = (view * BulletSpeed);

	/////////////////////////////////////////////////////Check with everything/////////////////////////////////////////////////////////////////////
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

	//store the closest static collision
	static_t = t;
	float enemies_t = FLT_MAX;

	DamageZonesHandler* closest_enemy = nullptr;
	//iterate through the enemies
	auto enemies = mOwner->GetSpace()->FindObjects(Tags::Enemy);
	std::for_each(enemies.begin(), enemies.end(),
		[&](GameObject* obj)
		{
			auto dmg = obj->GetComponentType<DamageZonesHandler>();
			if (dmg)
			{
				float temp_t = dmg->CheckHit(_ray, t);

				if (temp_t < enemies_t && temp_t != -1.0f)
				{
					enemies_t = temp_t;
					closest_enemy = dmg;
				}
			}
		});

	if (closest_enemy)
	{
		closest_enemy->ProcessHit(_ray, gun_damage * mPlayerInfo->damage_bonification, mPlayerInfo);
		if (mPlayerInfo->getId() == my_id)
			HitMarker->GetComponentType<FadeOut>()->Reset();
	}

	//get the smalles t from the enemies and the walls
	t = enemies_t < static_t ? enemies_t : static_t;

	//create particle effect at the collision with the bullet
	if (t >= 0.0f)
	{
		glm::vec3 bullet_collision_pos = _ray.mOrigin + _ray.mDirection * t;
		GameObject* bullet_collision = mOwner->GetSpace()->CreateObject();
		serializer.LoadArchetype("BulletCollision", bullet_collision);
		bullet_collision->mTransform.mPosition = bullet_collision_pos;
	}
}
