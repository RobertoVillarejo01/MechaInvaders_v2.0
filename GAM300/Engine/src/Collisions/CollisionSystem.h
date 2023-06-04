#pragma once

#include "Geometry/Geometry.h"
#include "Collisions/Collider.h"
#include "Physics/Rigidbody/Rigidbody.h"
#include "System/Scene/SceneSystem.h"
#include "Events/event.h"

struct ContactInformation
{
	glm::vec3 mContactNormal;
	glm::vec3 mIntersectionPoint;
	float mPenetration;
};

enum class CollisionType
{
	CollisionStarted,
	CollisionPersisted,
	CollisionEnded
};

struct CollisionEvent : public Event
{
public:
	CollisionEvent(Collider* _collider1, Collider* _collider2, CollisionType _type) : coll1{ _collider1 }, coll2{ _collider2 }, mType{_type} {}
	CollisionType mType;
	Collider* coll1 = nullptr;
	Collider* coll2 = nullptr;

	friend bool operator== (const CollisionEvent& lhs, const CollisionEvent& rhs);

private:
	bool operator== (CollisionEvent& rhs);
};


class CollisionSystem
{
	MAKE_SINGLETON(CollisionSystem);

public:

	bool Initialize();
	void Update();

	void CollideBodies();
	bool CheckInLastFrame(const std::pair<Collider*, Collider*>& _pair);
	void AddEvent(CollisionType type, CollisionEvent& event);
	void TriggerEvents();
	bool Collision(Collider* collider1, Collider* collider2, ContactInformation* cI);
	void ResolveContactVelocity(Rigidbody* rb1, Rigidbody* rb2, ContactInformation* cI);
	void ResolveContactPenetration(Rigidbody* rb1, Rigidbody* rb2, ContactInformation* cI);
	void ResolveContactVelocityStatic(Rigidbody* rb, ContactInformation* cI);
	void ResolveContactPenetrationStatic(Rigidbody* rb, ContactInformation* cI);

	bool PointvsAABB(const glm::vec3& pt, const geometry::aabb& AABB);
	bool PointvsOBB(const glm::vec3& pt, const geometry::obb& OBB);
	bool PointvsSphere(const glm::vec3& pt, const geometry::sphere& sp);
	bool PointVsPlane(const glm::vec3& pt, const geometry::plane& p, const float& planeScale);
	bool AABBvsAABB(const geometry::aabb& a, const geometry::aabb& b);
	bool SpherevsSphere(const geometry::sphere& s1, const geometry::sphere& s2);
	bool SpherevsAABB(const geometry::sphere& s, const geometry::aabb& AABB);
	bool SpherevsPlane(const geometry::sphere& s, const geometry::plane& p, const float& planeScale);
	bool AABBvsPlane(const geometry::aabb& AABB, const geometry::plane& p, const float& planeScale);
	bool OBBvsAABB(const geometry::obb& o, const geometry::aabb& a);
	bool CapsulevsCapsule(const geometry::capsule& c1, const geometry::capsule& c2);
	bool CapsulevsOBB(const geometry::capsule& c, const geometry::obb& o);
	bool CapsulevsAABB(const geometry::capsule& c, const geometry::aabb& a);
	bool SpherevsOBB(const geometry::sphere& s, const geometry::obb& o);

	bool SpherevsSphereContact(const geometry::sphere& s1, const geometry::sphere& s2, ContactInformation* contactInfo);
	bool SpherevsPlaneContact(const geometry::sphere& s, const geometry::plane& p, ContactInformation* contactInfo);
	bool SpherevsOBBContact(const geometry::sphere& s, const geometry::obb& o, ContactInformation* contatctInfo);
	bool AABBvsPlaneContact(const geometry::aabb& AABB, const geometry::plane& p, ContactInformation* contactInfo);
	bool SpherevsAABBContact(const geometry::sphere& s, const geometry::aabb& AABB, ContactInformation* contactInfo);
	bool AABBvsAABBContact(const geometry::aabb& a, const geometry::aabb& b, ContactInformation* contactInfo);
	bool CapsulevsCapsuleContact(const geometry::capsule& c1, const geometry::capsule& c2, ContactInformation* contactInfo);
	bool CapsulevsOBBContact(const geometry::capsule& c, const geometry::obb& o, ContactInformation* contatctInfo);
	bool CapsulevsAABBContact(const geometry::capsule& c, const geometry::aabb& a, ContactInformation* contatctInfo);

	bool CollideSpheres(Collider* collider1, Collider* collider2, ContactInformation* contactInfo);
	bool CollidePlanevsSphere(Collider* collider1, Collider* collider2, ContactInformation* contactInfo);
	bool CollidePlanevsAABB(Collider* collider1, Collider* collider2, ContactInformation* contactInfo);
	bool CollideSpherevsAABB(Collider* sphere, Collider* AABB, ContactInformation* contactInfo);
	bool CollideSpherevsOBB(Collider* sphere, Collider* AABB, ContactInformation* contactInfo);
	bool CollideAABBvsAABB(Collider* a, Collider* b, ContactInformation* contactInfo);
	bool CollideCapsules(Collider* a, Collider* b, ContactInformation* contactInfo);
	bool CollideCapsulevsOBB(Collider* a, Collider* b, ContactInformation* contactInfo);
	bool CollideCapsulevsAABB(Collider* a, Collider* b, ContactInformation* contactInfo);

	unsigned mCollisionIterations = 0;
	//TODO: probably will have to change this to 0 < coeff < 1
	const float restitutionCoefficient = 0.0f;

	std::vector<std::pair<Collider*, Collider*>> mLastFrame;
	std::vector<std::pair<Collider*, Collider*>> mCurrFrame;

	std::map<CollisionType, std::vector<CollisionEvent>> mEvents;
};
#define CollisionManager CollisionSystem::Instance()
