#include "CollisionSystem.h"

#include <glm/gtx/projection.hpp>
#include <algorithm>

#include "Graphics/DebugDraw/DebugDrawing.h"
#include "Events/EventDispatcher.h"

bool CollisionSystem::Initialize()
{
    mCollisionIterations = 5;
    return true;
}

void CollisionSystem::Update()
{
    CollideBodies();
}

void CollisionSystem::CollideBodies()
{
    auto& spaces = Scene.GetSpaces();
    ContactInformation c;
    for (unsigned i = 0; i < mCollisionIterations; ++i)
    {
        std::for_each(spaces.begin(), spaces.end(),
            [&](auto& it)
            {
                auto& dynamicObjects = it->GetComponentsType<DynamicCollider>();
                auto& staticObjects = it->GetComponentsType<StaticCollider>();
                std::for_each(dynamicObjects.begin(), dynamicObjects.end(), [&](IComp* coll1)
                    {
                        const auto& collider1 = static_cast<DynamicCollider*>(coll1);
                        std::for_each(dynamicObjects.begin(), dynamicObjects.end(), [&](IComp* coll2)
                            {
                                const auto& collider2 = static_cast<DynamicCollider*>(coll2);
                                if (collider1 != collider2)
                                {
                                    std::pair<Collider*, Collider*> pair(collider1, collider2);
                                    if (Collision(collider1, collider2, &c))
                                    {
                                        mCurrFrame.push_back(pair);
                                        //if collided last frame
                                        if (CheckInLastFrame(pair))
                                        {
                                            //collision Persisted
                                            CollisionEvent _event(collider1, collider2, CollisionType::CollisionPersisted);
                                            AddEvent(CollisionType::CollisionPersisted, _event);
                                            //EventDisp.trigger_collision_event(_event);
                                        }
                                        else
                                        {
                                            //collision Started
                                            CollisionEvent _event(collider1, collider2, CollisionType::CollisionStarted);
                                            AddEvent(CollisionType::CollisionStarted, _event);
                                            //EventDisp.trigger_collision_event(_event);
                                        }

                                        if (!collider1->mbGhost && !collider2->mbGhost)
                                        {
                                            if (i == 0)
                                                ResolveContactVelocity(collider1->mOwner->GetComponentType<Rigidbody>(), collider2->mOwner->GetComponentType<Rigidbody>(), &c);
                                            ResolveContactPenetration(collider1->mOwner->GetComponentType<Rigidbody>(), collider2->mOwner->GetComponentType<Rigidbody>(), &c);
                                        }
                                    }
                                    else
                                    {
                                        //if it collided last frame and this one it didn't
                                        if (CheckInLastFrame(pair))
                                        {
                                            //trigger collision ended events
                                            CollisionEvent _event(collider1, collider2, CollisionType::CollisionEnded);
                                            AddEvent(CollisionType::CollisionEnded, _event);
                                            //EventDisp.trigger_collision_event(_event);
                                        }
                                    }
                                }
                            });

                        std::for_each(staticObjects.begin(), staticObjects.end(), [&](IComp* staticColl)
                            {
                                const auto& staticCollider = static_cast<StaticCollider*>(staticColl);
                                std::pair<Collider*, Collider*> pair(collider1, staticCollider);
                                if (Collision(collider1, staticCollider, &c))
                                {
                                    mCurrFrame.push_back(pair);
                                    //if collided last frame
                                    if (CheckInLastFrame(pair))
                                    {
                                        //collision Persisted
                                        CollisionEvent _event(collider1, staticCollider, CollisionType::CollisionPersisted);
                                        AddEvent(CollisionType::CollisionPersisted, _event);
                                        //EventDisp.trigger_collision_event(_event);
                                    }
                                    else
                                    {
                                        //collision Started
                                        CollisionEvent _event(collider1, staticCollider, CollisionType::CollisionStarted);
                                        AddEvent(CollisionType::CollisionStarted, _event);
                                        //EventDisp.trigger_collision_event(_event);
                                    }

                                    if (!collider1->mbGhost && !staticCollider->mbGhost)
                                    {
                                        if (i == 0)
                                            ResolveContactVelocityStatic(collider1->mOwner->GetComponentType<Rigidbody>(), &c);
                                        ResolveContactPenetrationStatic(collider1->mOwner->GetComponentType<Rigidbody>(), &c);
                                    }
                                }
                                else
                                {
                                    //if it collided last frame and this one it didn't
                                    if (CheckInLastFrame(pair))
                                    {
                                        //trigger collision ended events
                                        CollisionEvent _event(collider1, staticCollider, CollisionType::CollisionEnded);
                                        AddEvent(CollisionType::CollisionEnded, _event);
                                        //EventDisp.trigger_collision_event(_event);
                                    }
                                }

                            });
                    });
            });
    }

    TriggerEvents();

    mLastFrame.clear();
    mLastFrame = mCurrFrame;
    mCurrFrame.clear();

}

bool CollisionSystem::CheckInLastFrame(const std::pair<Collider*, Collider*>& _pair)
{
    for(auto it : mLastFrame)
    {
        bool ac = it.first == _pair.first;
        bool bd = it.second == _pair.second;

        bool ad = it.first == _pair.second;
        bool bc = it.second == _pair.first;
        
        if ((ac && bd) || (ad && bc))
            return true;
    }

    return false;
}

void CollisionSystem::AddEvent(CollisionType type, CollisionEvent& event)
{
    auto& vec = mEvents[type];

    if (vec.empty())
        vec.push_back(event);
    else
    {
        auto found = std::find(vec.begin(), vec.end(), event);

        if (found == vec.end())
            vec.push_back(event);
    }
}

void CollisionSystem::TriggerEvents()
{
    for (auto it : mEvents)
    {
        for(auto it2 : it.second)
            EventDisp.trigger_collision_event(it2);
    }

    mEvents.clear();
}

bool CollisionSystem::Collision(Collider* collider1, Collider* collider2, ContactInformation* cI)
{
    switch (collider1->mShape)
    {
    case shape::AABB:
        switch (collider2->mShape)
        {
        case shape::AABB:
           return CollideAABBvsAABB(collider1, collider2, cI);
            break;
        case shape::SPHERICAL:
            return CollideSpherevsAABB(collider2, collider1, cI);
            break;
        case shape::PLANAR:
            return CollidePlanevsAABB(collider1, collider2, cI);
            break;
        case shape::CAPSULE:
            return CollideCapsulevsAABB(collider2, collider1, cI);
            break;
        default:
            break;
        }
        break;
    case shape::OBB:
        switch (collider2->mShape)
        {
        case shape::AABB:
            return CollideAABBvsAABB(collider1, collider2, cI);
            break;
        case shape::SPHERICAL:
            return CollideSpherevsOBB(collider2, collider1, cI);
            break;
        case shape::PLANAR:
            return CollidePlanevsAABB(collider1, collider2, cI);
            break;
        case shape::CAPSULE:
            return CollideCapsulevsOBB(collider2, collider1, cI);
            break;
        default:
            break;
        }
        break;
    case shape::CAPSULE:
        switch (collider2->mShape)
        {
        case shape::AABB:
            return CollideCapsulevsAABB(collider1, collider2, cI);
            break;
        case shape::CAPSULE:
            return CollideCapsules(collider1, collider2, cI);
        case shape::OBB:
                return CollideCapsulevsOBB(collider1, collider2, cI);
        default:
            break;
        }
        break;
    case shape::SPHERICAL:
        switch (collider2->mShape)
        {
        case shape::AABB:
           return CollideSpherevsAABB(collider1, collider2, cI);
            break;
        case shape::SPHERICAL:
           return CollideSpheres(collider1, collider2, cI);
            break;
        case shape::PLANAR:
            return CollidePlanevsSphere(collider1, collider2, cI);
            break;
        case shape::OBB:
            return CollideSpherevsOBB(collider1, collider2, cI);
        default:
            break;
        }
        break;
    }
    return false;
}

void CollisionSystem::ResolveContactVelocity(Rigidbody* rb1, Rigidbody* rb2, ContactInformation* cI)
{
    glm::vec3 relativeVelocity = rb2->mVelocity - rb1->mVelocity;

    float separation = glm::dot(relativeVelocity, cI->mContactNormal);
    if (separation > 0.0f) return;

    float separationIncrement = -separation *  restitutionCoefficient;
    separationIncrement -= separation;

    float massInfluence1 = rb1->mInverseMass / (rb1->mInverseMass + rb2->mInverseMass);
    float massInfluence2 = rb2->mInverseMass / (rb1->mInverseMass + rb2->mInverseMass);

    rb1->mVelocity -= cI->mContactNormal * separationIncrement * massInfluence1 * massInfluence2;
    rb2->mVelocity += cI->mContactNormal * separationIncrement * massInfluence1 * massInfluence2;
   //auto vao = rb1->mVelocity;
   //auto vbo = rb2->mVelocity;
   //rb1->mVelocity = 2 * rb1->mMass / (rb1->mMass + rb2->mMass) * vao + (rb2->mMass - rb1->mMass) / (rb1->mMass + rb2->mMass) * vbo;
   //rb2->mVelocity = 2 * rb2->mMass / (rb1->mMass + rb2->mMass) * vbo + (rb1->mMass - rb2->mMass) / (rb1->mMass + rb2->mMass) * vao;
}

void CollisionSystem::ResolveContactPenetration(Rigidbody* rb1, Rigidbody* rb2, ContactInformation* cI)
{
    float massInfluence1 = rb1->mInverseMass / (rb1->mInverseMass + rb2->mInverseMass);
    float massInfluence2 = rb2->mInverseMass / (rb1->mInverseMass + rb2->mInverseMass);

    rb1->mOwner->mTransform.mPosition -= cI->mContactNormal * cI->mPenetration * massInfluence1 * massInfluence2;
    rb2->mOwner->mTransform.mPosition += cI->mContactNormal * cI->mPenetration * massInfluence1 * massInfluence2;
}

void CollisionSystem::ResolveContactVelocityStatic(Rigidbody* rb, ContactInformation* cI)
{
    glm::vec3 relativeVelocity = -rb->mVelocity;
    float separation = glm::dot(relativeVelocity, cI->mContactNormal);
    
    if (separation > 0) return;
    float separationIncrement = separation * (1.0f + restitutionCoefficient);
    rb->mVelocity += cI->mContactNormal * separationIncrement;
    rb->mVelocity *= rb->mDrag;
}

void CollisionSystem::ResolveContactPenetrationStatic(Rigidbody* rb, ContactInformation* cI)
{
    rb->mOwner->mTransform.mPosition -= cI->mContactNormal * cI->mPenetration;
}

#pragma region Collision Detection
bool CollisionSystem::PointvsAABB(const glm::vec3& pt, const geometry::aabb& AABB)
{
    return (pt.x >= AABB.min.x && pt.y >= AABB.min.y && pt.z >= AABB.min.z &&
            pt.x <= AABB.max.x && pt.y <= AABB.max.y && pt.z <= AABB.max.z);
}

bool CollisionSystem::PointvsOBB(const glm::vec3& pt, const geometry::obb& OBB)
{
    glm::vec3 vector = pt - OBB.position;
    for (int i = 0; i < 3; ++i)
    {
        glm::vec3 axis = OBB.orientation[i * 3];
        float distance = glm::dot(vector, axis);
        if (distance > OBB.halfSize[i] || distance < -OBB.halfSize[i])
            return false;
    }
    return true;
}

bool CollisionSystem::PointvsSphere(const glm::vec3& pt, const geometry::sphere& sp)
{
    return (glm::length2(pt - sp.mCenter) <= (sp.mRadius * sp.mRadius));
}

bool CollisionSystem::PointVsPlane(const glm::vec3& pt, const geometry::plane& p, const float& planeScale)
{
    glm::vec3 normal = glm::normalize(p.mNormal);

    glm::vec3 closestPoint = pt - glm::dot(pt - p.mPoint, normal) * normal;
    float distanceSq = glm::length2(closestPoint - pt);
    if (distanceSq <= (planeScale * planeScale)) return true;

    float dot = glm::dot(pt - p.mPoint, p.mNormal);
    if (dot <= cEpsilon && dot >= -cEpsilon) return true;

    return false;
}

bool CollisionSystem::AABBvsAABB(const geometry::aabb& a, const geometry::aabb& b)
{
    return (a.min.x <= b.max.x && a.min.y <= b.max.y && a.min.z <= b.max.z &&
            a.max.x >= b.min.x && a.max.y >= b.min.y && a.max.z >= b.min.z);
}

bool CollisionSystem::SpherevsSphere(const geometry::sphere& s1, const geometry::sphere& s2)
{
    float distSq = glm::length2(s2.mCenter - s1.mCenter);
    float sumRadSq = (s1.mRadius + s2.mRadius) * (s1.mRadius + s2.mRadius);
    return distSq <= sumRadSq;  
}

bool CollisionSystem::SpherevsAABB(const geometry::sphere& s, const geometry::aabb& AABB)
{
    // get box closest point to sphere center by clamping
    float x = std::max(AABB.min.x, std::min(s.mCenter.x, AABB.max.x));
    float y = std::max(AABB.min.y, std::min(s.mCenter.y, AABB.max.y));
    float z = std::max(AABB.min.z, std::min(s.mCenter.z, AABB.max.z));
    glm::vec3 closestPoint = glm::vec3(x, y, z);

    // this is the same as isPointInsideSphere
    float distanceSq = glm::length2(closestPoint - s.mCenter);

    return distanceSq <= (s.mRadius * s.mRadius);
}

bool CollisionSystem::SpherevsPlane(const geometry::sphere& s, const geometry::plane& p, 
  const float& plane_thickness)
{
    return PointVsPlane(s.mCenter, p, s.mRadius + plane_thickness);
}

bool CollisionSystem::AABBvsPlane(const geometry::aabb& AABB, const geometry::plane& p, const float& plane_thickness)
{
    //we take the individual classifications for each of the AABB vertices
    geometry::classification_t classifications[8] =
    {
        classify_plane_point(p,  AABB.min, plane_thickness),
        classify_plane_point(p, {AABB.max.x, AABB.min.y, AABB.min.z}, plane_thickness),
        classify_plane_point(p, {AABB.min.x, AABB.max.y, AABB.min.z}, plane_thickness),
        classify_plane_point(p, {AABB.min.x, AABB.min.y, AABB.max.z}, plane_thickness),
        classify_plane_point(p, {AABB.min.x, AABB.max.y, AABB.max.z}, plane_thickness),
        classify_plane_point(p, {AABB.max.x, AABB.max.y, AABB.min.z}, plane_thickness),
        classify_plane_point(p, {AABB.max.x, AABB.min.y, AABB.max.z}, plane_thickness),
        classify_plane_point(p, AABB.max, plane_thickness),
    };

    unsigned insides = 0, outsides = 0, overlapping = 0;
    //here, we count the different amount of classifications of each type we have
    for (unsigned i = 0; i < 8; ++i)
    {
        if (classifications[i] == geometry::classification_t::overlapping) overlapping += 1;
        else if (classifications[i] == geometry::classification_t::inside) insides += 1;
        else if (classifications[i] == geometry::classification_t::outside) outsides += 1;
    }

    //all of the vertices overlap
    if (overlapping == 8) return true;
    //all of the vertices are inside
    else if (insides == 8 || outsides == 8) return false;

    //some of the vertices are inside, others outside, others overlapping
    return true;
}


bool CollisionSystem::OBBvsAABB(const geometry::obb& o, const geometry::aabb& a)
{
    auto orient = o.orientation;
    std::array<glm::vec3, 15> testingAxis =
    {
        //aabb's axis
        glm::vec3{1, 0, 0}, glm::vec3{0, 2, 0}, glm::vec3{0, 0, 1},
        //obb's axis
        glm::vec3{orient[0][0], orient[0][1], orient[0][2]},
        glm::vec3{orient[1][0], orient[1][1], orient[1][2]},
        glm::vec3{orient[2][0], orient[2][1], orient[2][2]},
    };
    // there is a total of 15 possible axis to test. The ones already added are local to aabb and obb,
    //and we will used those to calculate the other possible axis by computing every cross product between them.
    for (int i = 0; i < 3; ++i)
    {
        //we get three axis per iteration, hence the * 3
        testingAxis[6 + i * 3] = glm::cross(testingAxis[i], testingAxis[3]);
        testingAxis[6 + i * 3 + 1] = glm::cross(testingAxis[i], testingAxis[4]);
        testingAxis[6 + i * 3 + 2] = glm::cross(testingAxis[i], testingAxis[5]);
    }
    for (size_t i = 0; i < testingAxis.size(); ++i)
        if (!geometry::OverlapOnAxis(a, o, testingAxis[i]))
            return false;
    return true;

}
bool CollisionSystem::CapsulevsCapsule(const geometry::capsule& c1, const geometry::capsule& c2)
{
    geometry::segment s1 = { c1.position - glm::vec3(0, c1.height, 0) / 2.0f, c1.position + glm::vec3(0, c1.height, 0) / 2.0f };
    geometry::segment s2 = { c2.position - glm::vec3(0, c2.height, 0) / 2.0f, c2.position + glm::vec3(0, c2.height, 0) / 2.0f };
    auto closest = geometry::ClosestSegmentSegment(s1, s2);
    float distS1 = glm::length2(closest.p2 - closest.p1);
    float sumRad = c1.radius + c2.radius;
    return distS1 <= (sumRad * sumRad);
}

bool CollisionSystem::CapsulevsOBB(const geometry::capsule& c, const geometry::obb& o)
{
    auto top = o.position + o.halfSize;
    auto bot = o.position - o.halfSize;
    if (c.position.y < bot.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y + c.height / 2.0f, c.position.z), c.radius };
        return SpherevsOBB(s, o);
    }
    else if (c.position.y > top.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y - c.height / 2.0f, c.position.z), c.radius };
        return SpherevsOBB(s, o);
    }

    return SpherevsOBB({ c.position, c.radius }, o);
}

bool CollisionSystem::CapsulevsAABB(const geometry::capsule& c, const geometry::aabb& a)
{
    auto top = a.max;
    auto bot = a.min;

    if (c.position.y < bot.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y + c.height / 2.0f, c.position.z), c.radius };
        return SpherevsAABB(s, a);
    }
    else if (c.position.y > top.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y - c.height / 2.0f, c.position.z), c.radius };
        return SpherevsAABB(s, a);
    }
    return false;
}
bool CollisionSystem::SpherevsOBB(const geometry::sphere& s, const geometry::obb& o)
{
    auto closest = geometry::ClosestPointOBB(o, s.mCenter);
   
    float distSq = glm::length2(s.mCenter - closest);
   
    return distSq <= (s.mRadius * s.mRadius);
}
#pragma endregion

#pragma region Contact
bool CollisionSystem::SpherevsSphereContact(const geometry::sphere& s1, 
  const geometry::sphere& s2, ContactInformation* contactInfo)
{
    if (SpherevsSphere(s1, s2))
    {
        if (contactInfo)
        {
            glm::vec3 vector = s2.mCenter - s1.mCenter;
            glm::vec3 normal = glm::normalize(vector);
            glm::vec3 intersectionPoint = s1.mCenter + normal * s1.mRadius;
            float penetration = s1.mRadius + s2.mRadius - glm::length(vector);
            contactInfo->mContactNormal = normal;
            contactInfo->mIntersectionPoint = intersectionPoint;
            contactInfo->mPenetration = penetration;
        }

        return true;
    }
    return false;
}

bool CollisionSystem::SpherevsPlaneContact(const geometry::sphere& s, 
  const geometry::plane& p, ContactInformation* contactInfo)
{
    Debug::DrawPlane(p, { 1.0f, 1.0f, 1.0f, 1.0f });

    if (SpherevsPlane(s, p, p.mScale))
    {
        if (contactInfo)
        {
            glm::vec3 actualPlanePoint = p.mPoint + glm::vec3(p.mScale);
            glm::vec3 closestPoint = s.mCenter - glm::dot(s.mCenter - actualPlanePoint, glm::normalize(p.mNormal)) * glm::normalize(p.mNormal);
            glm::vec3 vector = closestPoint - s.mCenter;
            glm::vec3 normal = glm::normalize(vector);
            float penetration = s.mRadius - glm::length(vector);

            contactInfo->mContactNormal = p.mNormal;
            contactInfo->mIntersectionPoint = closestPoint;
            contactInfo->mPenetration = penetration;
        }
        return true;
    }
    return false;
}

bool CollisionSystem::SpherevsOBBContact(const geometry::sphere& s, const geometry::obb& o, ContactInformation* contatctInfo)
{
    if (SpherevsOBB(s, o))
    {
        if (contatctInfo)
        {
            auto closest = geometry::ClosestPointOBB(o, s.mCenter);
            glm::vec3 vec = closest - s.mCenter;
            glm::vec3 normal = glm::normalize(vec);
            float penetration = s.mRadius - glm::length(vec);
            contatctInfo->mContactNormal = normal;
            contatctInfo->mPenetration = penetration;
        }
        return true;
    }
    return false;
}

bool CollisionSystem::AABBvsPlaneContact(const geometry::aabb& AABB, const geometry::plane& p, ContactInformation* contactInfo)
{
    if (AABBvsPlane(AABB, p, p.mScale))
    {
        if (contactInfo)
        {
            glm::vec3 normal = glm::normalize(p.mNormal);

        }

        return true;
    }
    return false;
}

bool CollisionSystem::SpherevsAABBContact(const geometry::sphere& s, const geometry::aabb& AABB, ContactInformation* contactInfo)
{
    if (SpherevsAABB(s, AABB))
    {
        if (contactInfo)
        {
            float x = std::max(AABB.min.x, std::min(s.mCenter.x, AABB.max.x));
            float y = std::max(AABB.min.y, std::min(s.mCenter.y, AABB.max.y));
            float z = std::max(AABB.min.z, std::min(s.mCenter.z, AABB.max.z));
            glm::vec3 closestPoint = glm::vec3(x, y, z);
            glm::vec3 vector = closestPoint - s.mCenter;
            glm::vec3 normal{};
            if (glm::length2(vector) > 0.0f) normal = glm::normalize(vector);
            else normal = glm::vec3(1.0f, 0.0f, 0.0f);
            float penetration = s.mRadius - glm::length(vector);
           
            contactInfo->mContactNormal = normal;
            contactInfo->mIntersectionPoint = closestPoint;
            contactInfo->mPenetration = penetration;
        }
        return true;
    }
    return false;
}

bool CollisionSystem::AABBvsAABBContact(const geometry::aabb& a, const geometry::aabb& b, ContactInformation* contactInfo)
{
    if (AABBvsAABB(a, b))
    {
        if (contactInfo)
        {
            glm::vec3 aPos = (a.min + a.max) / 2.0f;
            glm::vec3 bPos = (b.min + b.max) / 2.0f;
            glm::vec3 vector = bPos - aPos;
            glm::vec3 aSize = { glm::length(a.max.x - a.min.x), glm::length(a.max.y - a.min.y), glm::length(a.max.z - a.min.z) };
            glm::vec3 bSize = { glm::length(b.max.x - b.min.x), glm::length(b.max.y - b.min.y), glm::length(b.max.z - b.min.z) };
            glm::vec3 penetrationVector = (aSize + bSize) / 2.0f - glm::vec3(std::fabs(vector.x), std::fabs(vector.y), std::fabs(vector.z));
            glm::vec3 normal;

            if (penetrationVector.x < penetrationVector.y && penetrationVector.x < penetrationVector.z)
            {
                normal = { 1.0f, 0.0f, 0.0f };
                contactInfo->mPenetration = penetrationVector.x;
            }
            else if (penetrationVector.y < penetrationVector.x && penetrationVector.y < penetrationVector.z)
            {
                normal = { 0.0f, 1.0f, 0.0f };
                contactInfo->mPenetration = penetrationVector.y;
            }
            else
            {
                normal = { 0.0f, 0.0f, 1.0f };
                contactInfo->mPenetration = penetrationVector.z;
            }

            if (glm::dot(vector, normal) < 0.0f) normal *= -1;

            contactInfo->mContactNormal = normal;
        }
        return true;
    }
    return false;
}

bool CollisionSystem::CapsulevsCapsuleContact(const geometry::capsule& c1, const geometry::capsule& c2, ContactInformation* contactInfo)
{
    geometry::segment s1 = { c1.position - glm::vec3(0, c1.height, 0) / 2.0f, c1.position + glm::vec3(0, c1.height, 0) / 2.0f };
    geometry::segment s2 = { c2.position - glm::vec3(0, c2.height, 0) / 2.0f, c2.position + glm::vec3(0, c2.height, 0) / 2.0f };
    auto closest = geometry::ClosestSegmentSegment(s1, s2);
    float dot = glm::dot(closest.p2 - closest.p1, s1.p2 - s1.p1);
    if (dot >= -FLT_EPSILON && dot <= FLT_EPSILON)
    {
        if (CapsulevsCapsule(c1, c2))
        {
            if (contactInfo)
            {
                glm::vec3 vector = closest.p2 - closest.p1;
                glm::vec3 normal = glm::normalize(vector);
                float penetration = c1.radius + c2.radius - glm::length(vector);
                contactInfo->mContactNormal = normal;
                contactInfo->mPenetration = penetration;
            }
            return true;
        }
    }
    else
    {
        if (c1.position.y > c2.position.y)
            return SpherevsSphereContact({ c1.position - glm::vec3(0, c1.height, 0) / 2.0f, c1.radius },
                                         { c2.position + glm::vec3(0, c2.height, 0) / 2.0f, c2.radius },
                                         contactInfo);
        else return SpherevsSphereContact({ c1.position + glm::vec3(0, c1.height, 0) / 2.0f, c1.radius },
                                          { c2.position - glm::vec3(0, c2.height, 0) / 2.0f, c2.radius },
                                          contactInfo);
    }
    return false;
}

bool CollisionSystem::CapsulevsOBBContact(const geometry::capsule& c, const geometry::obb& o, ContactInformation* contatctInfo)
{
    auto top = o.position + o.halfSize;
    auto bot = o.position - o.halfSize;

    if (c.position.y < bot.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y + c.height / 2.0f, c.position.z), c.radius };
        return SpherevsOBBContact(s, o, contatctInfo);

    }
    else if (c.position.y > top.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y - c.height / 2.0f, c.position.z), c.radius };
        return SpherevsOBBContact(s, o, contatctInfo);
    }
    return SpherevsOBBContact({ c.position, c.radius }, o, contatctInfo);
}

bool CollisionSystem::CapsulevsAABBContact(const geometry::capsule& c, const geometry::aabb& a, ContactInformation* contatctInfo)
{
    auto posAABB = (a.max + a.min) / 2.0f;
    auto scaleAABB = a.max - a.min;
    auto top = posAABB + scaleAABB / 2.0f;
    auto bot = posAABB - scaleAABB / 2.0f;

    if (c.position.y < bot.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y + c.height / 2.0f, c.position.z), c.radius };
        return SpherevsAABBContact(s, a, contatctInfo);

    }
    else if (c.position.y > top.y)
    {
        geometry::sphere s = { glm::vec3(c.position.x, c.position.y - c.height / 2.0f, c.position.z), c.radius };
        return SpherevsAABBContact(s, a, contatctInfo);
    }
    return SpherevsAABBContact({ c.position, c.radius }, a, contatctInfo);
}
#pragma endregion

#pragma region Collision Wrappers
bool CollisionSystem::CollideSpheres(Collider* collider1, Collider* collider2, ContactInformation* contactInfo)
{
    glm::vec3 sphere1Center = collider1->mOwner->mTransform.mPosition + collider1->mOffset;
    glm::vec3 sphere2Center = collider2->mOwner->mTransform.mPosition + collider2->mOffset;
    glm::vec3 scale1 = collider1->mScale;
    glm::vec3 scale2 = collider2->mScale;
    return SpherevsSphereContact({ sphere1Center, scale1.x }, {sphere2Center, scale2.x }, contactInfo);
}

bool CollisionSystem::CollidePlanevsSphere(Collider* collider1, Collider* collider2, ContactInformation* contactInfo)
{
    glm::vec3 sphereCenter = collider1->mOwner->mTransform.mPosition + collider1->mOffset;
    glm::vec3 planeCenter = collider2->mOwner->mTransform.mPosition + collider2->mOffset;
    glm::vec3 scale1 = collider1->mScale;

    return SpherevsPlaneContact({ sphereCenter, scale1.x },
    {planeCenter, collider2->mOwner->mTransform.mViewVector}, contactInfo);
}

bool CollisionSystem::CollidePlanevsAABB(Collider* collider1, Collider* collider2, ContactInformation* contactInfo)
{
    return false;
}

bool CollisionSystem::CollideSpherevsAABB(Collider* sphere, Collider* AABB, ContactInformation* contactInfo)
{
    glm::vec3 max = AABB->mOwner->mTransform.mPosition + AABB->mOffset + (AABB->mScale) / 2.0f;
    glm::vec3 min = AABB->mOwner->mTransform.mPosition + AABB->mOffset - (AABB->mScale) / 2.0f;
    glm::vec3 sphereCenter = sphere->mOwner->mTransform.mPosition + sphere->mOffset;
    glm::vec3 scale1 = sphere->mScale;

    return SpherevsAABBContact({ sphereCenter, scale1.x }, { min, max }, contactInfo);
}

bool CollisionSystem::CollideSpherevsOBB(Collider* sphere, Collider* OBB, ContactInformation* contactInfo)
{
    glm::vec3 sphereCenter = sphere->mOwner->mTransform.mPosition + sphere->mOffset;
    geometry::obb o = {OBB->mOwner->mTransform.mPosition + OBB->mOffset, OBB->mScale / 2.0f, OBB->mOrientationMtx};
    return SpherevsOBBContact({sphereCenter, sphere->mScale.x}, o, contactInfo);
}

bool CollisionSystem::CollideAABBvsAABB(Collider* a, Collider* b, ContactInformation* contactInfo)
{
    glm::vec3 minA = a->mOwner->mTransform.mPosition + a->mOffset - (a->mScale) / 2.0f;
    glm::vec3 maxA = a->mOwner->mTransform.mPosition + a->mOffset + (a->mScale) / 2.0f;
    glm::vec3 minB = b->mOwner->mTransform.mPosition + b->mOffset - (b->mScale) / 2.0f;
    glm::vec3 maxB = b->mOwner->mTransform.mPosition + b->mOffset + (b->mScale) / 2.0f;
    return AABBvsAABBContact({ minA, maxA }, { minB, maxB }, contactInfo);
}

bool CollisionSystem::CollideCapsules(Collider* a, Collider* b, ContactInformation* contactInfo)
{
    auto pos1 = a->mOwner->mTransform.mPosition + a->mOffset;
    auto pos2 = b->mOwner->mTransform.mPosition + b->mOffset;

    return CapsulevsCapsuleContact({a->mScale.x, a->mScale.y, pos1}, {b->mScale.x, b->mScale.y, pos2}, contactInfo);
}

bool CollisionSystem::CollideCapsulevsOBB(Collider* cap, Collider* OBB, ContactInformation* contactInfo)
{
    auto pos1 = cap->mOwner->mTransform.mPosition + cap->mOffset;
    geometry::obb o = { OBB->mOwner->mTransform.mPosition + OBB->mOffset, OBB->mScale / 2.0f, OBB->mOrientationMtx };
    return CapsulevsOBBContact({ cap->mScale.x, cap->mScale.y, pos1 }, o, contactInfo);
}

bool CollisionSystem::CollideCapsulevsAABB(Collider* cap, Collider* aabb, ContactInformation* contactInfo)
{
    auto pos1 = cap->mOwner->mTransform.mPosition + cap->mOffset;
    glm::vec3 minA = aabb->mOwner->mTransform.mPosition + aabb->mOffset - (aabb->mScale) / 2.0f;
    glm::vec3 maxA = aabb->mOwner->mTransform.mPosition + aabb->mOffset + (aabb->mScale) / 2.0f;
    return CapsulevsAABBContact({ cap->mScale.x, cap->mScale.y, pos1 }, { minA, maxA }, contactInfo);
}
#pragma endregion

bool CollisionEvent::operator==(CollisionEvent& rhs)
{
    if (mType != rhs.mType)
        return false;
    bool ac = coll1 == rhs.coll1;
    bool bd = coll2 == rhs.coll2;
    bool ad = coll1 == rhs.coll2;
    bool bc = coll2 == rhs.coll1;
    if ((ac && bd) || (ad && bc))
        return true;
    return false;
}
bool operator==(const CollisionEvent& lhs, const CollisionEvent& rhs)
{
    if (lhs.mType != rhs.mType)
        return false;
    bool ac = lhs.coll1 == rhs.coll1;
    bool bd = lhs.coll2 == rhs.coll2;
    bool ad = lhs.coll1 == rhs.coll2;
    bool bc = lhs.coll2 == rhs.coll1;
    if ((ac && bd) || (ad && bc))
        return true;
    return false;
}
