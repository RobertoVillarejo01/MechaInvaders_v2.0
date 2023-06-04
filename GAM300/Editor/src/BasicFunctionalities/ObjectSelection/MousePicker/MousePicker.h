#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "Graphics/Camera/Camera.h"

class GameObject; //FW decl
struct Collider;
class MousePicker
{
public:
	MousePicker(GFX::Camera* _cam)
	{
		mCamera = _cam;
	}

#ifdef EDITOR
	//raycast to all objects
	void RayFromMouse(glm::vec2 mousePos);
	Collider* RayFromMouseCollider(glm::vec2 mousePos);
#endif
	//wrapper
	glm::vec4 ScreenToWorld(glm::vec2& mousePos);
	//Convert from window coordinates to opengl coordinates
	glm::vec3 ScreenToNormalized(glm::vec2& mousePos);
	//I honestly don't know what clip space is
	glm::vec4 NormalizedToClipped(glm::vec3& mouseScreen);
	//Convert to camera
	glm::vec4 ProjToCam(glm::vec4& mouseCam);
	//to world
	glm::vec4 CamToWorld(glm::vec4& mouse3D);

	GFX::Camera* mCamera = nullptr;
};