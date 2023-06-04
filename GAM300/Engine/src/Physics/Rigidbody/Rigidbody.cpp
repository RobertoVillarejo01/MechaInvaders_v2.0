#include <iostream>
#include "Utilities/FrameRateController/FrameRateController.h"
#include "Rigidbody.h"
#include "System/Scene/SceneSystem.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui_impl_sdl.h"
#include "../Editor/ImGui/imgui_impl_opengl3.h"
#endif // EDITOR

namespace glm
{
	std::ostream& operator<<(std::ostream& os, const glm::vec3& v)
	{
		os << "x: " << v.x;
		os << " y: " << v.y;
		os << " z: " << v.z;
		return os;
	}
}

void Rigidbody::Initialize()
{
	mInverseMass = 1.0f / mMass;
	mDrag = glm::vec3(0.99f);
}

void Rigidbody::Update()
{
	AddForce(mGravity * mMass);
	
	Integrate(static_cast<float>(FRC.GetFrameTime()));

}


void Rigidbody::Shutdown()
{
}

IComp* Rigidbody::Clone() { return mOwner->GetSpace()->CreateComp<Rigidbody>(this); }

void Rigidbody::AddForce(const glm::vec3& Force)
{
	mNetForce += Force; 
}

void Rigidbody::Integrate(float dt)
{
	mAcceleration = mNetForce * mInverseMass;
	mVelocity += mAcceleration * dt;
	mVelocity *= mDrag;

	if (!mOwner)
	{
		std::cerr << "Im integrating but I do not have a owner. File: " << __FILE__
			<< "  Line: " << __LINE__ << std::endl;
	}
	mOwner->mTransform.mPosition += mVelocity * dt;

	mNetForce = glm::vec3(0.0f, 0.0f, 0.0f);
}

#ifdef EDITOR

bool Rigidbody::Edit()
{
	//ImGui::PushID(this);
	//
	//if (ImGui::TreeNodeEx("Edit Rigidbody"))
	//{
		ImGui::DragFloat3("Velocity : ", &mVelocity[0]);
		ImGui::DragFloat3("Gravity : ", &mGravity[0]);
		if(ImGui::DragFloat("Mass", &mMass, 1.0f, 0.5f)) mInverseMass = 1.0f / mMass;
	
	//	ImGui::TreePop();
	//}
	//ImGui::PopID();

	return true;
}

#endif

void Rigidbody::ToJson(nlohmann::json& j) const
{
	j["Velocity"] << mVelocity;
	j["Mass"] << mMass;
	j["Gravity"] << mGravity;
	j["Drag"] << mDrag;
}

void Rigidbody::FromJson(nlohmann::json& j)
{
	j["Velocity"] >> mVelocity;
	j["Mass"] >> mMass;
	j["Gravity"] >> mGravity;
	j["Drag"] >> mDrag;
}
