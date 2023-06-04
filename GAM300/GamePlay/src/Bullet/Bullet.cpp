#include "Bullet.h"
#include "Engine.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void Bullet::Initialize()
{
	timer = 0;
}

void Bullet::Update()
{
	timer += FRC.GetFrameTime();
	//enemies = mOwner->GetSpace()->GetObjectsByTag(Tags::Enemy);
	//
	//std::for_each(enemies.begin(), enemies.end(),
	//	[&](GameObject* obj)
	//	{
	//		auto collider = obj->GetComponentType< Collider>();
	//		if (collider)
	//		{
	//			geometry::ray _ray({ mOwner->mTransform.mPosition - mOwner->mTransform.mViewVector * (mOwner->mTransform.mPosition.z / 2) }, {mOwner->mTransform.mViewVector});
	//			glm::vec3 half_scale = { collider->mScaleOffset.x / 2 ,collider->mScaleOffset.y / 2, collider->mScaleOffset.z / 2 };
	//
	//			geometry::aabb bv({ collider->mOffset + obj->mTransform.mPosition - half_scale }, { collider->mOffset + obj->mTransform.mPosition + half_scale });
	//
	//			float t = geometry::intersection_ray_aabb(_ray, bv);
	//			if (t > 0)
	//			{
	//				//mOwner->GetSpace()->DestroyObject(mOwner);
	//				mOwner->GetSpace()->DestroyObject(obj);
	//			}
	//		}
	//	});

	if (timer >= die_cooldown)
		mOwner->GetSpace()->DestroyObject(mOwner);
}

IComp* Bullet::Clone() { return Scene.CreateComp<Bullet>(mOwner->GetSpace(), this); }

#ifdef EDITOR
bool Bullet::Edit()
{
	bool changed = false;

	return changed;
}
#endif // EDITOR


void Bullet::ToJson(nlohmann::json& j) const
{
}

void Bullet::FromJson(nlohmann::json& j)
{
}