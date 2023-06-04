#include "PlayerCamera.h"

#include "Graphics/Camera/Camera.h"
#include "Utilities/Input/Input.h"
#include "Serializer/Factory.h"
#include "Window/Window.h"
#include "GameStateManager/MenuManager/MenuManager.h"
#include "GameStateManager/GameStateManager.h"

#define WIN_LEAN_AND_MEAN
#include <Windows.h>


#ifdef EDITOR
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR


/****************** Camera Component ******************/
void PlayerCamera::Initialize()
{
	mCamera = mOwner->GetComponentType<CameraComp>();
	SetCursorPos(0, 0);
}

IComp* PlayerCamera::Clone()
{
	return Scene.CreateComp<PlayerCamera>(mOwner->GetSpace(), this);
}

void PlayerCamera::UpdateVectors()
{
	// update Front, Right and Up Vectors using the updated Euler angles
	glm::vec3 target;
	glm::vec3 cam_pos = mOwner->mTransform.mPosition;

	//computing the new view vector based on rotations
	target.x = cam_pos.x + sin(glm::radians(angle.y)) * cos(glm::radians(angle.x));
	target.y = cam_pos.y + cos(glm::radians(angle.y));
	target.z = cam_pos.z + sin(glm::radians(angle.y)) * sin(glm::radians(angle.x));

	mCamera->LookAt(target);
}

void PlayerCamera::GetRotation()
{
	int x;
	int y;
	
	SDL_GetRelativeMouseState(&x, &y);
	//SetCursorPos(x, y)

	//variables to store the mouse position
	glm::vec2 current_mouese_pos = InputManager.WindowMousePos();

	//computing the offset on x and y
	float xoffset = x * GSM.mConfig.mSensitivity;
	float yoffset = -y * GSM.mConfig.mSensitivity;

	//modify the rotation values
	angle.x += xoffset;
	angle.y += yoffset;

	//rotations of less tha 90 degrees to avoid gimbal locks
	if (angle.y > -5)  angle.y = -5;
	if (angle.y < -170) angle.y = -170;

}

void PlayerCamera::ToJson(nlohmann::json& j) const
{
	j["mbStaticCamera"] << mbStaticCamera;
}

void PlayerCamera::FromJson(nlohmann::json& j)
{
	j["mbStaticCamera"] >> mbStaticCamera;
}

void PlayerCamera::Update()
{
	if (mbStaticCamera)
		return;
	if (!MenuMgr.InMenu())
	{
		if (base_player)
		{
			GetRotation();
			UpdateVectors();
		}
	}
}

#ifdef EDITOR

bool PlayerCamera::Edit()
{
	bool changed = false;
	Debug::DrawSphere({ {mCamera->GetPosition()}, {(mOwner->mTransform.mScale.x + mOwner->mTransform.mScale.y + mOwner->mTransform.mScale.z) / 6.0f} }, { 0.0f, 0.0f, 1.0f, 1.0f });

	ImGui::Checkbox("Static Camera", &mbStaticCamera);
	if (ImGui::Button("Reset To Owner"))
	{
		mCamera->SetPosition(mOwner->mTransform.mPosition);
		mCamera->SetTarget(mCamera->GetPosition() + mOwner->mTransform.mViewVector);
	}

	return changed;
}

#endif