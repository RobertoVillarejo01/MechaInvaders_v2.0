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
class InteractionText;
class InteractionComp;
class IconLogic;
class TextComponent;

class Player : public SMComponent<Player>
{
public:
    enum SMTypes { COMBAT_SM, AIMING_SM, VOICES_SM, HEAL_SM, CHEATS_SM, TOTAL_SM };

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
    void interactEvent();
    void GameOverEvent();
    void ChangeWeapon();

    void buy_weapon(std::string weapon_name, unsigned weapon_cost);

    void ComputeWaypointDirection(std::pair<GameObject*, renderable*> waypoint, GameObject* current_task);
    void Waypoint();

	bool interacting = false;
    bool mbDead = false;
    bool mbAiming = false;
    bool mbDamaged = false;

    //----------VENDING MACHINES--------------
    void LosePowerUps();
    std::vector<IComp*> icons;
    unsigned int health_pu = 0;
    unsigned int agility_pu = 0;
    unsigned int doubleshot_pu = 0;
    unsigned int total_pu = 0;
    unsigned int hp_increase = 30;
    unsigned int damage_bonification = 1;
    unsigned int reload_vel = 1;
    bool fast_revive = false;
    //----------------------------------------

    bool base_player = false;
    bool serverItIsI = false;
    bool InLobby = false;

    glm::vec3 hit_direction = {};

    unsigned int money = 0;
    Weapon* mWeaponComp = nullptr;
    Health* mHealth = nullptr;
    float MoveSpeed = 1.0F;
    float RunSpeed  = 3.0F;
    float lerp_fov_dt = 0.0f;
    float sprintLimit = 1.0F;

    GameObject* new_weapon = nullptr;
private:
    bool on_ramp = false;
    GameObject* camera = nullptr;
    GameObject* mWeapon = nullptr;
    GameObject* SecondWeapon = nullptr;
    GameObject* waypoint = nullptr;
    std::array<std::pair<GameObject*, renderable*>, 3> mWaypoints{};

    //componets
    Rigidbody* mRigidBody = nullptr;
    CameraComp* mCameraComp = nullptr;
    PlayerCamera* mPlayerCameraComp = nullptr;
    SoundEmitter* mEmitter = nullptr;
    InteractionText* interaction_text = nullptr;
    InteractionComp* InteractComp = nullptr;
    InteractionComp* ChosenObject = nullptr;
    TextComponent* revive_text = nullptr;

    glm::vec3 mMovementFor{};
    glm::vec3 mMovementRight{};
    glm::vec3 Direction{};
    glm::vec3 dir_before_jump = {};
    
    //PROP_VAL(float, MoveSpeed, 1.0F);
    //PROP_VAL(float, RunSpeed, 3.0F);
    glm::vec3 current_jump_speed = {};
    float Air_speed = 0.1f;
    float Jump_speed = 0.7f;
    float mGravitySlope = 0.0F;

    //PROP_VAL(float, sprintCooldown, 1.0F);
    //PROP_VAL(float, sprintLimit, 1.0F);
    float sprintCooldown = 1.0F;
    float sprintTime = 0.0F;


    //PROP_VAL(float, cooldown, 0.7F);
    //PROP_VAL(float, sprintTime, 0.0F);
    //PROP_VAL(bool, mbSprinting, false);
    float cooldown = 0.7F;
    float init_fov = 0.0f;
    float run_fov = 70.0f;
    float fov_speed = 5.0f;
    bool mbOnSlope = false;

    glm::vec3 PlayerGravity{};
    float Timer        = 1.2f;
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

	//---------Interacting---------
	void InteractingInit();
	void InteractingUpdate();
	void InteractingShutDown();
    HUDBar* hud_bar = nullptr;

    //-------------SHOT------------
    //------------AIMING-----------
    void AimInit();
    void AimUpdate();

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
    float time_reviving = 0.0f;
    float timer_to_revive = 3.0f;

    //-------------AUTO HEAL----------
    void HealUpdate();
    float heal_speed = 0.1f;
    float damaged_timer = 0.0f;
    float damaged_cooldown = 5.0f;

    //-------------CHEATS----------
    void CheatCodes();
    bool immortal = false;
    bool fly = false;

    // Player Voices
    void Silent() { }
    void updateInput();

    int id;
    bool firtcoll = true;
};