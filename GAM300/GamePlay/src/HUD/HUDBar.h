#pragma once
#include "LogicSystem/Logic.h"

class renderable;
class HUDBar : public ILogic
{
public:
	void Initialize();
	void Update();

	void SetBar(float percentage);
	void Reset();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

private:
	float current_var_val = 0.0f;
	float total_var_val = 100.0f;

	glm::vec3 InitialPos{};
	glm::vec3 InitialScale{};


	renderable* render = nullptr;
};