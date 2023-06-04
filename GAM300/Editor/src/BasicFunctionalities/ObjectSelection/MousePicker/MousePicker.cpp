#include "MousePicker.h"
#include "Window/Window.h"
#include "../Editor/src/Editor.h"
#include <glm/gtx/transform.hpp>
#include "System/Scene/SceneSystem.h"
#include "Geometry/Geometry.h"

#ifdef EDITOR
void MousePicker::RayFromMouse(glm::vec2 mousePos)
{
	auto spaces = Scene.GetSpaces();
	if (spaces.empty())
		return;

	if (!ImGui::IsAnyWindowHovered())
		EditorMgr.mConfig.mCandidateObjects.clear();

	geometry::aabb abbSelection;
	geometry::obb obbSelection;
	geometry::ray r;
	r.mDirection = glm::vec3(ScreenToWorld(mousePos));
	r.mOrigin = EditorMgr.mCamera.GetPosition();
	for (auto space : spaces)
	{
		//check if visible
		if (!space->IsVisible()) continue;

		auto objects = space->GetAliveObjects();

		for (auto it : objects)
		{
			// If the object has a renderable component, it likely has a mesh

			auto* it_render = it->GetComponentType<renderable>();
			obbSelection.position = it->mTransform.mPosition;
			obbSelection.orientation = it->mTransform.GetRotMtx();
			if (!it_render)
				obbSelection.halfSize = it->mTransform.mScale / 2.0f;
			else
			{
				abbSelection = it_render->GetAABB();
				obbSelection.halfSize = it->mTransform.mScale * (abbSelection.max - abbSelection.min) / 2.0f;
				//abbSelection.min = _obj->mTransform.mScale * abbSelection.min + _obj->mTransform.mPosition;
				//abbSelection.max = _obj->mTransform.mScale * abbSelection.max + _obj->mTransform.mPosition;
			}

			auto res = geometry::intersection_ray_obb(r, obbSelection);

			if (res > cEpsilon && !ImGui::IsAnyWindowHovered())
				EditorMgr.mConfig.mCandidateObjects.push_back(it);
		}
	}
}

Collider* MousePicker::RayFromMouseCollider(glm::vec2 mousePos)
{
	geometry::ray r;
	r.mDirection = glm::vec3(ScreenToWorld(mousePos));
	r.mOrigin = EditorMgr.mCamera.GetPosition();
	float resFinal = std::numeric_limits<float>::max();
	Collider* picked = nullptr;
	std::vector<IComp*>& colliders = Scene.GetComponentsType<StaticCollider>();
	for (unsigned i = 0; i < colliders.size(); i++)
	{
		StaticCollider* collider = dynamic_cast<StaticCollider*>(colliders[i]);
		switch (collider->mShape)
		{
			case shape::AABB:
			case shape::OBB:
			{
				geometry::obb obb;
				obb.halfSize = collider->mScale / 2.0f;
				obb.position = collider->mOwner->mTransform.mPosition + collider->mOffset;
				obb.orientation = collider->mOrientationMtx;
				auto res = geometry::intersection_ray_obb(r, obb);

				if (res < resFinal && res > 0.0f)
				{
					resFinal = res;
					picked = collider;
				}
				break;
			}
			case shape::SPHERICAL:
			{
				geometry::sphere sphere;
				sphere.mCenter = collider->mOwner->mTransform.mPosition + collider->mOffset;
				sphere.mRadius = collider->mScale.x;
				auto res = geometry::intersection_ray_sphere(r, sphere);

				if (res < resFinal && res > 0.0f)
				{
					resFinal = res;
					picked = collider;
				}
				break;
			}
		}
	}

	std::vector<IComp*>& dyn_colliders = Scene.GetComponentsType<DynamicCollider>();
	for (unsigned i = 0; i < dyn_colliders.size(); i++)
	{
		DynamicCollider* collider = dynamic_cast<DynamicCollider*>(dyn_colliders[i]);
		switch (collider->mShape)
		{
			case shape::AABB:
			case shape::OBB:
			{
				geometry::obb obb;
				obb.halfSize = collider->mScale / 2.0f;
				obb.position = collider->mOwner->mTransform.mPosition + collider->mOffset;
				obb.orientation = collider->mOrientationMtx;
				auto res = geometry::intersection_ray_obb(r, obb);

				if (res < resFinal && res > 0.0f)
				{
					resFinal = res;
					picked = collider;
				}
				break;
			}
			case shape::SPHERICAL:
			{
				geometry::sphere sphere;
				sphere.mCenter = collider->mOwner->mTransform.mPosition + collider->mOffset;
				sphere.mRadius = collider->mScale.x;
				auto res = geometry::intersection_ray_sphere(r, sphere);

				if (res < resFinal && res > 0.0f)
				{
					resFinal = res;
					picked = collider;
				}
				break;
			}
		}
	}
	return picked;
}
#endif

glm::vec4 MousePicker::ScreenToWorld(glm::vec2& mousePos)
{
	glm::vec3 normalized = ScreenToNormalized(mousePos);
	glm::vec4 clipped = NormalizedToClipped(normalized);
	glm::vec4 cam = ProjToCam(clipped);
	glm::vec4 world = CamToWorld(cam);

	return world;
}

glm::vec3 MousePicker::ScreenToNormalized(glm::vec2& mousePos)
{
	glm::vec3 normalized;

	float windowW = static_cast<float>(WindowMgr.GetResolution().x);
	float windowH = static_cast<float>(WindowMgr.GetResolution().y);

	normalized.x = (2.0f * mousePos.x) / windowW - 1.0f;
	normalized.y = 1.0f - (2.0f * mousePos.y) / windowH;
	normalized.z = 1.0f;

	return normalized;
}

glm::vec4 MousePicker::NormalizedToClipped(glm::vec3& mouseScreen)
{
	glm::vec4 ray_clip = glm::vec4(mouseScreen.x, mouseScreen.y, -1.0f, 1.0f);

	return ray_clip;
}

glm::vec4 MousePicker::ProjToCam(glm::vec4& mouseClipped)
{
	glm::mat4 proj = glm::perspective(glm::radians(60.0f), static_cast<float>(WindowMgr.GetResolution().x) /
									  static_cast<float>(WindowMgr.GetResolution().y), 0.01f, 1000.0f);
	glm::vec4 ray_eye = glm::inverse(proj) * mouseClipped;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

	return ray_eye;
}

glm::vec4 MousePicker::CamToWorld(glm::vec4& mouse3D)
{
	glm::mat4 mat = glm::inverse(mCamera->GetW2Cam());
	glm::vec4 ray_world = mat * mouse3D;
	ray_world = glm::normalize(ray_world);

	return ray_world;
}


