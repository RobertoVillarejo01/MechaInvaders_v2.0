#pragma once
#include "LogicSystem/Logic.h"

class renderable;
class FadeOut : public ILogic
{
public:
	void Initialize();
	void Update();
	void Reset();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	bool trigger = false;
private:
	bool initialize_triggering = false;
	float timer = 0.0f;
	float delay = 0.0f;
	float fade_speed = 1.0f;

	bool use_particles = false;
	float time_to_stop_particles = 0.5f;
	float particles_timer = 0.0f;

	bool destroy_object = false;
	renderable* render = nullptr;
	glm::vec4 color = {};
};