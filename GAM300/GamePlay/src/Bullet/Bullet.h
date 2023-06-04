#pragma once

#include "LogicSystem/Logic.h"


class Bullet : public ILogic
{
public:
	void   Initialize() override;
	void   Update() override;

	IComp* Clone() override;

#ifdef EDITOR
	bool   Edit() override;
#endif

	void ToJson(nlohmann::json& j) const override;
	void FromJson(nlohmann::json& j) override;

private:
	float timer = 0.0f;
	float die_cooldown = 1.0f;

	std::vector<GameObject*>   enemies;
};