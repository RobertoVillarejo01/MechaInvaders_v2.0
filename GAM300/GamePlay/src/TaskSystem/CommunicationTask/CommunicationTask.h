#pragma once

#include "../Engine/src/LogicSystem/StateMachine.h"
#include <vector>

class TaskInfo;
class SoundEmitter;

class CommunicationTask : public SMComponent<CommunicationTask>
{
public:
	enum SMTypes { INTERACTION_SM, COLOR_SM, TOTAL_SM };

	void StateMachineInitialize() override;

	void ChangeBrainSize(size_t _size) { SetBrainSize(_size); }

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	void ResetTask();

#ifdef EDITOR
	bool Edit();
#endif

	void ResetAll();
	void AllCorrect();

private:
	TaskInfo* info = nullptr;
	GameObject* number = nullptr;
	IRenderable* number_rend = nullptr;
	SoundEmitter* emitter = nullptr;

	glm::vec4 num_offset;
	bool num_rot90 = false;
	bool num_rot180 = false;

	//---------IDLE-----------
	void IdleInit();
	void IdleUpdate();

	//-------ACTIVATED-------
	void ActivatedInit();
	void ActivatedUpdate();
	void ActivatedShutDown();

	//---------COLOR---------
	void ColorUpdate();

};