#include "Camera.h"
#include "Utilities/Input/Input.h"
#include "Physics/Rigidbody/Rigidbody.h"
#include "Serializer/Factory.h"
#include "Window/Window.h"
#include "Graphics/Texture/Texture.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "resourcemanager/Resourcemanager.h"
#include "Utilities/FrameRateController/FrameRateController.h"
#include "GameStateManager/MenuManager/MenuManager.h"

#define WIN_LEAN_AND_MEAN
#include <Windows.h>


#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR


/****************** Camera Component ******************/

void CameraComp::Initialize()
{
	// Camera properties
	if (mbDefaultValues)
	{
		SetPosition(mOwner->mTransform.mPosition);
		SetTarget(mPosition + mOwner->mTransform.mViewVector);
		
		angle = glm::vec2(0, -90);
	}

	// Set as debug drawing camera (can only be one per session)
	if (mbDrawDebug)
		Debug::DebugRenderer::Instance().SetCamera(this);

	// Information dependent on Viewport size
	RegenerateBuffers();
}

IComp* CameraComp::Clone()
{
	return Scene.CreateComp<CameraComp>(mOwner->GetSpace(), this);
}

void CameraComp::LookAt(glm::vec3 target)
{
	mOwner->mTransform.LookAt(target);
	SetTarget(target);
	SetUpVec(mOwner->mTransform.mUpVect);
	if (render)
		Camera::Update();
}

void CameraComp::ToJson(nlohmann::json& j) const
{
	j["mPosition"] << mPosition;
	j["mTarget"] << mTarget;
	j["mbStaticCamera"] << mbStaticCamera;
	j["mbDefaultValues"] << mbDefaultValues;

	j["mLightActionRadius"] << mLightActionRadius;
	j["mShadowLightRadius"] << mShadowLightRadius;

	j["mNear"] << mNear;
	j["mFar"] << mFar;
	j["mAngle"] << mAngle;

	j["mbDrawDebug"] << mbDrawDebug;

	if (mCubemap)
		j["mCubeMap"] << mCubemap->getName();
}

void CameraComp::FromJson(nlohmann::json& j)
{
	j["mPosition"] >> mPosition;
	j["mTarget"] >> mTarget;
	j["mbStaticCamera"] >> mbStaticCamera;
	j["mbDefaultValues"] >> mbDefaultValues;
	
	if (j.find("mLightActionRadius") != j.end()) {
		j["mLightActionRadius"] >> mLightActionRadius;
		j["mShadowLightRadius"] >> mShadowLightRadius;
	}

	if (j.find("mNear") != j.end()) {
		j["mNear"] >> mNear;
		j["mFar"] >> mFar;
		j["mAngle"] >> mAngle;
	}

	if (j.find("mCubeMap") != j.end()) {
		std::string cube_name;
		j["mCubeMap"] >> cube_name;
		mCubemap = ResourceMgr.GetResource<GFX::Cubemap>(cube_name.data());
	}

	if (j.find("mbDrawDebug") != j.end()) 
		j["mbDrawDebug"] >> mbDrawDebug;
}

void CameraComp::Update()
{
	if (mbStaticCamera) 
	{
		Camera::Update();
		return;
	}
	Camera::Update();
}

void CameraComp::RegenerateBuffers()
{
	auto& size = WindowMgr.GetResolution();
	SetProjection(mAngle, size, mNear, mFar);
	mGBuffer.Initialize(size);
	mFrameBuffer.GenerateColorBuffer(size, 2);
}

bool CameraComp::IsObjectVisible(const glm::vec3& position, glm::vec3& _pos_projection)
{
	auto p = mW2Proj * glm::vec4(position, 1.0f);
	_pos_projection = p;
	auto halfVP = glm::vec2(mViewportSize.x / 2.0f, mViewportSize.y / 2.0f);

	bool res = _pos_projection.x < halfVP.x&& _pos_projection.x > -halfVP.x &&
		_pos_projection.y < halfVP.y&& _pos_projection.y > -halfVP.y &&
		_pos_projection.z > 0.0f;
	return res;
}

#ifdef EDITOR

bool CameraComp::Edit()
{
	bool changed = false;

	// Get current cubemap's name
	std::string cube_name("** NONE **");
	if (mCubemap) {
		cube_name = mCubemap->getName();
		cube_name = cube_name.substr(cube_name.find_last_of('/') + 1);
	}

	// Edit near and far planes
	bool cam1 = ImGui::DragFloat("Near", &mNear, 0.001f, 0.001f);
	bool cam2 = ImGui::DragFloat("Far", &mFar, 0.1f, mNear);
	bool cam3 = ImGui::DragFloat("Angle", &mAngle, 0.1f);
	
	if (cam1 || cam2 || cam3)
		SetProjection(mAngle, WindowMgr.GetResolution(), mNear, mFar);

	// Light action radius
	ImGui::Separator();
	ImGui::DragFloat("LightRadius", &mLightActionRadius);
	ImGui::DragFloat("ShadowRadius", &mShadowLightRadius);

	// Select the cubemap
	ImGui::Separator();
	if (ImGui::BeginCombo("Cubemap", cube_name.data()))
	{
		// Default case
		if (ImGui::Selectable("** NONE **", nullptr == mCubemap)) {
			mCubemap = nullptr;
		}

		auto& cubemaps = ResourceMgr.GetResourcesOfType<GFX::Cubemap>();
		for (auto& map : cubemaps)
		{
			GFX::CubemapRes casted_map = std::reinterpret_pointer_cast<TResource<GFX::Cubemap>>(map.second);
			std::string filename = map.first.substr(map.first.find_last_of('/') + 1);
			if (ImGui::Selectable(filename.data(), casted_map == mCubemap))
				mCubemap = casted_map;
		}
		ImGui::EndCombo();
	}
	ImGui::Separator();
	ImGui::Checkbox("DrawDebug", &mbDrawDebug);

	return changed;
}

#endif