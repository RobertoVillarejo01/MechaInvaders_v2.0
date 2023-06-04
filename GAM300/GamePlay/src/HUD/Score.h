#pragma once
#include "LogicSystem/Logic.h"

class TextComponent;

class HighestScore : public ILogic
{
public:
	void Initialize();
	void Update();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
private:

	TextComponent* score;
};