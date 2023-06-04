#pragma once

#include "LogicSystem/Logic.h"

class move_input;
class FixingTask;
class CameraComp;
class PlayerCamera;
class Health;
class Weapon;
class HUDBar;
class SoundEmitter;

class NetworkPlayer : public SMComponent<NetworkPlayer>
{
public:
    enum SMTypes { COMBAT_SM, VOICES_SM, HEAL_SM, CHEATS_SM, TOTAL_SM };

    void StateMachineInitialize() override;
    void StateMachineShutDown() override;

    // Public Wrapper aroud protected method for testing purposes
    void ChangeBrainSize(size_t _size) { SetBrainSize(_size); }

    void ToJson(nlohmann::json& j) const;
    void FromJson(nlohmann::json& j);

    void setId(int player_id) { id = player_id; }
    int getId() { return id; }

    void OnCollisionStarted(const Collider* coll);
    void OnCollisionPersisted(const Collider* coll);
    void OnCollisionEnded(const Collider* coll);
    bool mbWalking = false;
    bool mbIdle = false;
    bool mbJumping = false;
    bool mbSprinting = false;
    bool mbShooting = false;

    void moveUpdate(move_input& p);
    void shoot();

    bool mbDead = false;
    bool mbDamaged = false;

    bool base_player = false;
    glm::vec3 hit_direction = {};

private:
    GameObject* camera = nullptr;
    GameObject* mWeapon = nullptr;

    //componets
    Rigidbody* mRigidBody = nullptr;
    Health* mHealth = nullptr;
    SoundEmitter* mEmitter = nullptr;


    glm::vec3 mMovementFor{};
    glm::vec3 mMovementRight{};
    glm::vec3 Direction{};
    glm::vec3 dir_before_jump = {};

    glm::vec3 current_jump_speed = {};
    float MoveSpeed = 1.0F;
    float Air_speed = 0.1f;
    float Jump_speed = 0.7f;
    float RunSpeed = 3.0F;
    float mGravitySlope = 0.0F;

    float sprintCooldown = 1.0F;
    float sprintLimit = 1.0F;

    float cooldown = 0.7F;
    float sprintTime = 0.0F;
   
    bool mbOnSlope = false;

    glm::vec3 PlayerGravity{};
    float Timer = 1.2f;
    float jumptimer = 0.0f;
    float jumpForce = 0.F;
    float fallForce = 0.F;

    float interaction_timer = 0.0f;

    //tasks
    FixingTask* fixtask = nullptr;

#ifdef EDITOR
    bool Edit();
#endif // EDITOR

    //--------------MOVE-----------
    void MoveInit();
    void MoveUpdate();

    //-------------SHOT------------

    //------------DAMAGED-----------
    void DamagedInit();
    void DamagedUpdate();
    void DamagedEnd();
    float stun_dt = 0.0f;
    float stun_time = 0.2f;
    float stun_displacement = 10.0f;
    glm::vec3 stun_new_pos = {};


    //------------DIE-----------
    void DieInit();
    void DieUpdate();
    GameObject* gameOver = nullptr;
    std::vector<DynamicCollider*> playercolls;
    glm::vec3 init_pos = {};
    float time_dead = 0.0f;
    float time_to_die = 5.0f;


    //--------------REVIVE-------------
    void ReviveInit();
    void ReviveUpdate();
    glm::vec3 collOffset = {};

    //-------------AUTO HEAL----------
    void HealUpdate();
    float heal_speed = 0.1f;
    float damaged_timer = 0.0f;
    float damaged_cooldown = 5.0f;

    //-------------CHEATS----------
    void CheatCodes();
    bool immortal = false;
    // Player Voices
    void Silent() { }

    int id;
    bool firtcoll = true;
};