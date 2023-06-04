#pragma once
#include <glm/glm.hpp>
#include "Objects/Components.h"


namespace glm
{
	std::ostream& operator<<(std::ostream& os, const glm::vec3& v);
}

//debug
struct State
{
	glm::vec3 prevVelocity;
	glm::vec3 prevPosition;
};

struct Rigidbody : public IComp
{
	void Initialize() override;
	void Update() override;
	void Shutdown() override;

	void AddForce(const glm::vec3& Force);
	void Integrate(float dt);
#ifdef EDITOR
	virtual bool Edit();
#endif

	IComp* Clone();

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	float mMass = 1.0f;
	float mInverseMass = 1.0f;

	glm::vec3 mVelocity = {};
	glm::vec3 mAcceleration = {};

	glm::vec3 linearMomentum = {};

	glm::vec3 mNetForce = {};
	glm::vec3 mGravity = {};
	glm::vec3 mDrag = {};

	static const unsigned type_id = static_cast<const unsigned>(TypeIds::RigidBody);

	bool mIsDynamic = true;
	std::vector<State> mPrevStates;
};
