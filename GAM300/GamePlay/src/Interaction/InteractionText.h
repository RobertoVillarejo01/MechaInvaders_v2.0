#pragma once
#include "LogicSystem/Logic.h"

class Player;
class InteractionComp;
class TextComponent;

class InteractionText : public ILogic
{
public:
	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	void StopInteracting(bool keepactive = true);
	void DeatachObj(InteractionComp* obj);
	void AttachObj(InteractionComp* obj, std::string str);
	bool CheckInteract();
	void Interact();

	InteractionComp* chosen_object = nullptr;

	Player* mPlayer = nullptr;

private:
	bool show_text = false;
	bool mbInteracting = false;

	float distance = 30.0f;


	TextComponent* mText = nullptr;
	std::unordered_map<InteractionComp*, std::string> objects;
};