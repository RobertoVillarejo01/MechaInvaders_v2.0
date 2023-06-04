#pragma once
#include "LogicSystem/Logic.h"

class SoundEmitter;
class CameraComp;
class Player;
class SoundEmitter;

class Weapon : public SMComponent<Weapon>
{
public:
    enum SMTypes {AIMING, SHOOTING, TOTAL_SM };

    void StateMachineInitialize() override;

    // Public Wrapper aroud protected method for testing purposes
    void ChangeBrainSize(size_t _size) { SetBrainSize(_size); }

    void ToJson(nlohmann::json& j) const;
    void FromJson(nlohmann::json& j);

    bool CanReload();
    void FillAmmo() { total_bullets = max_bullets;  }
    //void SpeedUpReloading() { reload_timer /= 2; hide_speed *= 2; }

#ifdef EDITOR
    bool Edit() override;
#endif

    //parameters that define the state of the weapon
    bool reloading = false;
    bool weapon_hided = false;
    bool weapong_hiding = false;
    bool gun_is_full = true;
    bool no_ammo = false;
    bool recoiling = false;
    bool gun_changed = false;
    bool changing_gun = false;
    float init_fov = 0.0f;
    bool infinity_ammo = false;
    bool automatic_weapon = true;

    //parameters that are useful to know about the gun
    int bullet_count = 30;
    int total_bullets = 120;
    int charger_size = 30;
    float gun_damage = 2;
    int max_bullets = 80;

private:
    //Mirilla
    GameObject* mTop = nullptr;
    GameObject* mBot = nullptr;
    GameObject* mLeft = nullptr;
    GameObject* mRight = nullptr;
    float initial_distance = 1.0f;
    float end_distance = 2.0f;
    float aiming_distance = 0.0f;
    float sight_alpha = 1.0f;
    float sight_dt = 0.0f;

    //Gun player, camera etc
    Player* mPlayerInfo = nullptr;
    GameObject* mPlayer     = nullptr;
    GameObject* ShootingPos = nullptr;
    GameObject* AimPos      = nullptr;
    GameObject* HitMarker   = nullptr;
    CameraComp* mCamera     = nullptr;
    SoundEmitter* mEmitter = nullptr;

    //Aiming
    glm::vec3 target = {};
    glm::vec3 init_position = {};
    glm::vec3 aim_position = {};
    float aim_fov = 55.0f;
    float TargetFar = 50;
    float lerp_dt = 0.0f;
    float aim_speed = 5.0f;

    //Shooting
    float time_dt     = 0.0f;
    float CoolDown    = 0.1f;
    float BulletSpeed = 400.0f;

    //Recoiling backwards
    glm::vec3 init_recoilpos = {};
    glm::vec3 final_recoilpos = {};
    float recoil_distance = 0.2f;
    float recoil_cooldown = 0.2f;
    float recoil_back_velocity = 30.0f;
    float recoil_front_velocity = 20.0f;
    float recoil_dt = 0.0f;
    float recoil_gun_angle = 0.0f;

    //Recoiling Upwards
    glm::vec2 recoil_distorsion = {0.2f, 0.5f};
    glm::vec2 recoil_upward_distance = {};
    bool recoiling_backwards = true;

    //Reload Parameters
    float hide_angle = 0.0f;
    float angle_to_hide = -35.0f;
    float hide_speed = 30.0f;
    glm::vec3 initial_hide_offset = {};
    
    float reload_timer = 0.0f;
    float time_to_reload = 1.0f;

    void UpdateSight(float new_distance, float new_alpha = 255.0f);
    void ResetGunValues();

    void AimInit();
    void AimUpdate();
    void AimShutDown();

    void NotAimingInit();
    void NotAimingUpdate();
    void NotAimingShutDown();

    void ReloadInit();
    void ReloadUpdate();
    void ReloadShutDown();

    void ChangeGunInit();
    void ChangeGunUpdate();
    float change_gun_dt = 0.0f;
    float change_gun_timer = 0.5f;

    void HideInit();
    void HideUpdate();
    void HideShutDown();

    void ShootInit();
    void ShootUpdate();

    void IdleInit();
    void IdleUpdate();

    int my_id;
protected:
    virtual void ShotFunction();
};