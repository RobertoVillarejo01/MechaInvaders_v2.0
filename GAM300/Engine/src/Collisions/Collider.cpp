#include "Collider.h"
#include "System/Scene/SceneSystem.h"
#include "Physics/Rigidbody/Rigidbody.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "CollisionSystem.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#include "../Editor/ImGui/imgui_impl_sdl.h"
#include "../Editor/ImGui/imgui_impl_opengl3.h"
#include "../Editor/ImGui/ImGuizmo.h"
#include "../Editor/src/Editor.h"
#endif // EDITOR

void Collider::Initialize()
{
	
}

void Collider::Update()
{
}


#ifdef EDITOR

bool Collider::Edit()
{
	   
		glm::vec4 white(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec3 & ownerPos = mOwner->mTransform.mPosition;
		glm::vec3& ownerScale = mOwner->mTransform.mScale;


		if (ImGui::TreeNode("Edit"))
		{
			if (ImGui::Button("Set scale to owner's"))    mScale = ownerScale;
			if (ImGui::Button("Set position to owner's")) mOffset = glm::vec3(0, 0, 0);
			switch (mShape)
			{
				case shape::AABB:
				{
					//if (glm::epsilonEqual<glm::vec3>(mOwner->mTransform.mOrientation, glm::vec3(0, 0, 0), glm::vec3(cEpsilon)))
					//if(glm::length(mOwner->mTransform.mOrientation) >= -FLT_EPSILON && glm::length(mOwner->mTransform.mOrientation) <= FLT_EPSILON)
					//{
					geometry::aabb box;
					ImGui::InputFloat3("Box Collider Scale", &mScale[0]);
					box.max = ownerPos + mScale / 2.0f + mOffset;
					box.min = ownerPos - mScale / 2.0f + mOffset;
					Debug::DrawAABB(box, white);

					break;
				}

				case shape::OBB:
				{
					geometry::obb b;
					b.position = ownerPos + mOffset;
					b.halfSize = mScale / 2.0f;
					b.orientation = mOwner->mTransform.GetRotMtx();
					Debug::DrawOBB(b, white);
					break;
				}
				
				case shape::SPHERICAL:
				{
					geometry::sphere sp;
					ImGui::SliderFloat("Sphere Collider Radius", &mScale.x, 0.0f, 100.0f, "%.1f", 1.0f);
					sp.mCenter = ownerPos + mOffset;
					sp.mRadius = mScale.x;
					Debug::DrawSphere(sp, white);
					break;
				}

				case shape::PLANAR:
				{
					geometry::plane p;
					p.mNormal = mOwner->mTransform.mViewVector;

					Debug::DrawPlane(p, white);
					break;
				}
				case shape::CAPSULE:
				{
					ImGui::SliderFloat("Capsule Collider Radius", &mScale.x, 0.0f, 45.0f, "%.1f", 1.0f);
					mScale.z = mScale.x;
					ImGui::SliderFloat("Capsule Collider Height", &mScale.y, 0.0f, 45.0f, "%.1f", 1.0f);
					geometry::cylinder c;
					c.mRadius = mScale.x;
					c.mCenter = ownerPos + mOffset;
					c.mHeight = mScale.y;
					Debug::DrawCylinder(c, white);
					break;
				}
			}
			if (!mOwner->IsEditing)
			{

				static ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE_COLLIDER;
				static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::LOCAL;

				if (ImGui::RadioButton("Translate collider", mCurrentGizmoOperation == ImGuizmo::TRANSLATE_COLLIDER))
					mCurrentGizmoOperation = ImGuizmo::TRANSLATE_COLLIDER;
				ImGui::SameLine();
				if (ImGui::RadioButton("Scale collider", mCurrentGizmoOperation == ImGuizmo::SCALE_COLLIDER))
					mCurrentGizmoOperation = ImGuizmo::SCALE_COLLIDER;

				if (mCurrentGizmoOperation != ImGuizmo::SCALE_COLLIDER)
				{
					if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
						mCurrentGizmoMode = ImGuizmo::LOCAL;
					ImGui::SameLine();
					if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
						mCurrentGizmoMode = ImGuizmo::WORLD;
				}
				static bool useSnap = false;
				ImGui::Checkbox("Snap", &useSnap);
				glm::vec3 snap;
				//switch (mCurrentGizmoOperation)
				//{
				//case ImGuizmo::TRANSLATE:
				//	snap = snapTranslation;
				//	ImGui::InputFloat3("Snap", &snap.x);
				//	break;
				//case ImGuizmo::ROTATE:
				//	snap = snapRotation;
				//	ImGui::InputFloat("Angle Snap", &snap.x);
				//	break;
				//case ImGuizmo::SCALE:
				//	snap = snapScale;
				//	ImGui::InputFloat("Scale Snap", &snap.x);
				//	break;
				//}
				ImGuiIO& io = ImGui::GetIO();
				ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

				float matrixTranslation[3], matrixRotation[3], matrixScale[3];
				auto mModel2World = glm::translate(mOffset + ownerPos) * glm::scale(mScale);
				ImGuizmo::Manipulate(&EditorMgr.mCamera.GetW2Cam()[0][0], &EditorMgr.mCamera.GetProj()[0][0], mCurrentGizmoOperation, mCurrentGizmoMode, &mModel2World[0][0], nullptr, useSnap ? &snap.x : nullptr);
				ImGuizmo::DecomposeMatrixToComponents(&mModel2World[0][0], matrixTranslation, matrixRotation, matrixScale);
				matrixRotation[0] = 0.0f;
				matrixRotation[1] = 0.0f;
				matrixRotation[2] = 0.0f;

				for (int i = 0; i < 3; i++)
				{
					if (matrixScale[i] < std::numeric_limits<float>::epsilon())
						matrixScale[i] = 0.00001f;
				}
				mOffset = glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]) - ownerPos;
				mScale = glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]);
				ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &mModel2World[0][0]);
			}

			ImGui::InputFloat3("Collider position", &mOffset[0]);
			ImGui::TreePop();
		}
		//ImGui::PopID();
		std::string sh;
		switch (mShape)
		{
		case shape::AABB:
			sh = "AABB";
			break;
		case shape::OBB:
			sh = "OBB";
			break;
		case shape::SPHERICAL:
			sh = "Sphere";
			break;

		case shape::CAPSULE:
			sh = "Capsule";
			break;
		}
		if (ImGui::TreeNodeEx("Select Shape"))
		{
			std::string current = "Current: " + sh;
			ImGui::Text(current.c_str());
			if (ImGui::Button("AABB Collider")) { mScale = mOwner->mTransform.mScale; mShape = shape::AABB; }
			if (ImGui::Button("OBB Collider")) { mScale = mOwner->mTransform.mScale; mShape = shape::OBB; }
			if (ImGui::Button("Sphere Collider")) { mScale = mOwner->mTransform.mScale; mShape = shape::SPHERICAL; }
			if (ImGui::Button("Capsule Collider")) { mScale = mOwner->mTransform.mScale; mShape = shape::CAPSULE; }
			ImGui::TreePop();
		}
		

		ImGui::Checkbox("Ghost", &mbGhost);


	return false;
}

#endif

void Collider::Shutdown()
{
}

void Collider::ToJson(nlohmann::json& j) const
{
	j["Collider Offset"] << mOffset;
	j["Collider Scale"] << mScale;
	switch (mShape)
	{
	case shape::AABB:
		j["Collider Shape"] << std::string("Cubic");
		break;
	case shape::SPHERICAL:
		j["Collider Shape"] << std::string("Spherical");
		break;
	case shape::PLANAR:
		j["Collider Shape"] << std::string("Planar");
		break;
	case shape::CAPSULE:
		j["Collider Shape"] << std::string("Capsule");
		break;
	case shape::OBB:
		j["Collider Shape"] << std::string("OBB");
		break;
	}
	j["Ghost"] << mbGhost;
}

void Collider::FromJson(nlohmann::json& j)
{
	j["Collider Offset"] >> mOffset;
	j["Collider Scale"] >> mScale;
	std::string shapeType = j["Collider Shape"].get<std::string>();
	if (shapeType == "Cubic") mShape = shape::AABB;
	else if (shapeType == "Spherical") mShape = shape::SPHERICAL;
	else if (shapeType == "Planar") mShape = shape::PLANAR;
	else if (shapeType == "Capsule") mShape = shape::CAPSULE;
	else if (shapeType == "OBB") mShape = shape::OBB;
	j["Ghost"] >> mbGhost;

	if (j.find("Ghost") != j.end())
		j["Ghost"] >> mbGhost;
	else
		mbGhost = false;
}

IComp* Collider::Clone() 
{
	auto newCollider = mOwner->GetSpace()->CreateComp<Collider>(this);
	newCollider->mScale = mScale;
	newCollider->mShape = mShape;
	newCollider->mOffset = mOffset;
	newCollider->mOrientationMtx = mOrientationMtx;
	return  newCollider;
}

void StaticCollider::Initialize()
{
	mOrientationMtx = mOwner->mTransform.GetRotMtx();
	//mScale = mOwner->mTransform.mScale;
}

void StaticCollider::Update()
{
	mOrientationMtx = mOwner->mTransform.GetRotMtx();
}

void StaticCollider::Shutdown()
{
}
IComp* StaticCollider::Clone() { return mOwner->GetSpace()->CreateComp<StaticCollider>(this); }

void DynamicCollider::Initialize()
{
	mOrientationMtx = mOwner->mTransform.GetRotMtx();
	//mScale = mOwner->mTransform.mScale;
	auto rb = mOwner->GetComponentType<Rigidbody>();
	if (!rb) rb = static_cast<Rigidbody*>(mOwner->AddComp(Scene.CreateComp<Rigidbody>()));
	rb->Initialize();
}

void DynamicCollider::Update()
{
	mOrientationMtx = mOwner->mTransform.GetRotMtx();
}

void DynamicCollider::Shutdown()
{
}

IComp* DynamicCollider::Clone() { return mOwner->GetSpace()->CreateComp<DynamicCollider>(this); }