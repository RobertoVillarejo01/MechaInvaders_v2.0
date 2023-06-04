#pragma once
#include "LogicSystem/Logic.h"

class Player;

class DamageZonesHandler : public ILogic
{
public:
enum class zones { HEAD, BODY, LEFT_ARM, RIGHT_ARM };
#ifdef EDITOR
	bool Edit();
	void EditZones(GameObject* child);
#endif

	float CheckHit(const geometry::ray& _ray, float smallest_t);
	float ProcessHit(const geometry::ray& _ray, float damage, Player* player);
	void Initialize();
	void Update();
	void Shutdown();
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
	

	geometry::aabb total_aabb{};
	glm::vec3 hitDir{};
	IComp* Clone() override;
	std::map<zones, GameObject*> mDamageZones;

};