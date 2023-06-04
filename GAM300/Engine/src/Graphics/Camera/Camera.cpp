#include "Camera.h"
#include "Graphics/Lighting/Light.h"

#include <glm/gtx/transform.hpp>

namespace GFX {

	void Camera::Update()
	{
		mW2Cam = glm::lookAt(mPosition, mTarget, mUpVec);
		mW2Proj = mCam2Proj * mW2Cam;
	}

	const glm::mat4& Camera::GetProj() const
	{
		return mCam2Proj;
	}
	const glm::mat4& Camera::GetW2Proj() const
	{
		return mW2Proj;
	}
	const glm::mat4& Camera::GetW2Cam() const
	{
		return mW2Cam;
	}
	const glm::mat4& Camera::GetCam2Proj() const
	{
		return mCam2Proj;
	}
	const glm::vec3& Camera::GetPosition() const
	{
		return mPosition;
	}
	const glm::vec3& Camera::GetTarget() const
	{
		return mTarget;
	}

	void Camera::SetProjection(float angle, glm::ivec2 viewport_size, float near, float far)
	{
		mNear = near;
		mFar = far;
		mAngle = angle;
		mViewportSize = viewport_size;

		mCam2Proj = glm::perspective(glm::radians(angle),
			(float)viewport_size.x / (float)viewport_size.y, near, far);
	}
	void Camera::SetPosition(glm::vec3 position)
	{
		mPosition = position;
	}
	void Camera::SetTarget(glm::vec3 target)
	{
		mTarget = target;
	}
	void Camera::SetUpVec(glm::vec3 up)
	{
		mUpVec = up;
	}
	void Camera::SetNear(float near)
	{
		mNear = near;
		mCam2Proj = glm::perspective(glm::radians(mAngle),
			(float)mViewportSize.x / (float)mViewportSize.y, mNear, mFar);
	}
	void Camera::SetFar(float far)
	{
		mFar = far;
		mCam2Proj = glm::perspective(glm::radians(mAngle),
			(float)mViewportSize.x / (float)mViewportSize.y, mNear, mFar);
	}
	void Camera::SetAngle(float angle)
	{
		mAngle = angle;
		mCam2Proj = glm::perspective(glm::radians(mAngle),
			(float)mViewportSize.x / (float)mViewportSize.y, mNear, mFar);
	}
	void Camera::SetViewSize(glm::ivec2 viewsize)
	{
		mViewportSize = viewsize;
		mCam2Proj = glm::perspective(glm::radians(mAngle),
			(float)mViewportSize.x / (float)mViewportSize.y, mNear, mFar);
	}


	// We do not need to do anything fancy here, the matrices are already set in the Set light
	void LightCamera::Update() {}
	void LightCamera::SetLight(LightComponent* _light)
	{
		assert(_light != nullptr);

		mW2Cam = _light->GetWorld2Cam();
		mCam2Proj = _light->GetCam2Proj();
		mW2Proj = mCam2Proj * mW2Cam;
	}

}
