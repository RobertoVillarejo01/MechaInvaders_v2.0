#pragma once
#include "../Serializer/Properties/Property.h"

#include "Utilities/Math.h"

class GameObject;

struct Transform : public ISerializable
{
	Transform(glm::vec3 _pos = glm::vec3{ 0.0f }, glm::vec3 _scale = glm::vec3{ 1.0f }, glm::vec3 _offset = glm::vec3{0.0f,0.0f,0.0f},
		glm::vec3 _view = glm::vec3{ 0.0f, 0.0f, -1.0f }, glm::vec3 _up = glm::vec3{ 0.0f, 1.0f, 0.0f },
		glm::vec3 _right = glm::vec3{ 1.0f, 0.0f, 0.0f })
		: mPosition{ _pos }, mScale{ _scale }, mOffset{ _offset }, mViewVector{ _view }, 
		mUpVect{ _up }, mRightVector{ _right } {}


	void RotateAround(glm::vec3 _vector, float _ang_deg);
	void RotateAround(glm::vec3 _pos, glm::vec3 _vector, float _ang_deg);
	void LookAt(glm::vec3 target);

	glm::mat4 ModelToWorld() const;
	glm::mat4 TransRotMtx(GameObject* parent = nullptr) const;
	
	glm::mat4 GetRotMtx() const;
	glm::mat4 ModelToWorldOffset(const glm::mat4& parentPos)const;

	friend void operator<<(nlohmann::json& j, const Transform& _rhs);
	friend void operator>>(nlohmann::json& j, Transform& _rhs);

	glm::vec3 mPosition;
	glm::vec3 mScale;
	
	glm::vec3 mViewVector;
	glm::vec3 mUpVect;
	glm::vec3 mRightVector;

	glm::vec3 mOffset;

#ifdef EDITOR
	glm::vec3 prev_rot{};
	glm::vec3 mOrientation{};
	void UpdateRotationVectors();
	glm::mat4 M2Weuler() const;
	glm::mat4 TransRotEulerMtx() const;
#endif
};
