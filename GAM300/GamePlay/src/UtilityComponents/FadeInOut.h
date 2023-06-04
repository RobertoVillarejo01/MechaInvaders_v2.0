#pragma once
#include "LogicSystem/Logic.h"

class renderable;
class FadeInOut : public ILogic
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

	void SetNextLevel(std::string level) { next_level = level; }
//	void SetDelay(float _delay) { delay = _delay; }
//	void SetFadeSpeed(float _speed) { fade_speed = _speed; }
	float mOutTime = 0.0f;
private:
	float timer = 0.0f;
	float mInTime = 0.0f;
	float mDisplayTime = 0.0f;
	bool faded_in = false;
	bool faded_out = false;
	bool displayed = false;
	bool inMenu = false;

	bool out = false;
	bool just_out = false;
	bool transition = false;
	std::string next_level{};

	renderable* render = nullptr;
	glm::vec4 color = {};
};
