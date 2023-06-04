#pragma once
#include "LogicSystem/Logic.h"

class Health; //foward declaration
class DamageOverlay : public ILogic
{
public:
	DamageOverlay();
	~DamageOverlay();

	void Initialize() override;
	void Update() override;
	void Shutdown() override;

#ifdef EDITOR
	// Edit function to be override by child class, for ImGui
	bool Edit() override;
#endif

	//Clone function for deep copys in case is necesary
	IComp* Clone() override;

protected:
	void ToJson(nlohmann::json& j) const override;
	void FromJson(nlohmann::json& j) override;

private:
	renderable* damageOverlay = nullptr;
	glm::vec4	damageColor = glm::vec4{};
	float curr_alpha = 0.0f;
	float prev_alpha = 0.0f;
	float max_alpha = 0.7f;

	Health* player_health = nullptr;
	float max_health = 0.0f;

	GameObject* gameOver = nullptr;
};