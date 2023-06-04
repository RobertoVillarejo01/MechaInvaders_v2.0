#pragma once
#include "Objects/Components.h"


enum class shape {AABB, OBB, SPHERICAL, PLANAR, CAPSULE};


struct Collider : public IComp
{
	virtual void Initialize();
	virtual void Update();

#ifdef EDITOR

	virtual bool Edit();
#endif
	virtual void Shutdown();
	virtual IComp* Clone();

	//These are to edit
	glm::vec3 mOffset = {};
	//these are the actual ones that are used to solve collisions
	glm::vec3 mScale = {};
	glm::vec3 mPosition = {};
	glm::mat4 mOrientationMtx;
	shape mShape = shape::AABB;
	bool mbGhost;
protected:
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
};

struct StaticCollider : public Collider
{
	IComp* Clone();
	void Initialize();
	void Update();
	void Shutdown();
	static const unsigned type_id = static_cast<const unsigned>(TypeIds::StaticCollider);
};

struct DynamicCollider : public Collider
{
	IComp* Clone();
	void Initialize() override;
	void Update() override;
	void Shutdown() override;
	static const unsigned type_id = static_cast<const unsigned>(TypeIds::DynamicCollider);
};