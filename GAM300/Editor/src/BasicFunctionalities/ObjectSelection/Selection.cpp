#include "../Editor/src/Editor.h"
#include "Utilities/Input/Input.h"
#include "System/Scene/SceneSystem.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "../ObjectSelection/MousePicker/MousePicker.h"
#include "../Editor/ImGui/ImGuizmo.h"
#include "Serializer/Factory.h"


void Editor::MultipleSelection(const glm::vec2& initialPos, const glm::vec2& finalPos)
{
	glm::vec2 size = finalPos - initialPos;
	glm::vec2 center = initialPos - size / 2.0f;
	geometry::aabb selectionRect{ glm::vec3(center.x - size.x / 2.0f, center.y - size.y / 2.0f, center.x - size.x / 2.0f),
								  glm::vec3(center.x + size.x / 2.0f, center.y + size.y / 2.0f, center.y + size.y / 2.0f) };

	for (auto& space : Scene.GetSpaces())
	{
		for (auto& obj : space->GetAliveObjects())
		{
			glm::vec2 posScreen;
			auto pos = mCamera.GetProj() * mCamera.GetW2Cam() * glm::vec4(obj->mTransform.mPosition, 1.0f);
			posScreen.x = pos.x;
			posScreen.y = pos.y;
			glm::vec2 min = { selectionRect.min.x, selectionRect.min.y };
			glm::vec2 max = { selectionRect.max.x, selectionRect.max.y };
			//if (geometry::intersection_point_aabb_2D(posScreen, min, max))
			//	mSelectedObjects.push_back(obj);

		}
	}
}

void Editor::SelectObjects()
{
	if (MouseDown(MouseKey::LEFT) && !mDraggingMouse)
	{
		IsDragging = true;
		if (!startingMousePos) startingMousePos = new glm::vec2(InputManager.WindowMousePos());
		*endMousePos = (glm::vec2(InputManager.WindowMousePos()));
		mMultipleEdition = true;
		geometry::aabb selectionRect = { glm::vec3(*endMousePos, startingMousePos->x), glm::vec3(*startingMousePos, startingMousePos->x) };
		Debug::DrawAABB(selectionRect, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));

		MultipleSelection(*startingMousePos, *endMousePos);
	}

	if (MouseReleased(MouseKey::LEFT) && startingMousePos)
	{
		delete startingMousePos;
		startingMousePos = nullptr;
		IsDragging = false;
	}
}

void Editor::ItemList(const std::vector<GameObject*>& _objects)
{
	if (!mConfig.mShowItemList) return;

	ImGui::Begin("Select objects");
	ImGui::SetWindowPos(ImVec2(mConfig.mCurrentMousePos.x, mConfig.mCurrentMousePos.y));
	for (auto o : _objects)
	{
		if (ImGui::IsItemHovered())
			RenderPreviewObject(o, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
		if (ImGui::MenuItem(o->GetName()))
		{
			if (selectedObj && selectedObj->attaching)
				selectedObj->CreateChild(o);
			else
			{
				selectedObj = o;
				mConfig.mShowItemList = false;
			}
			break;
		}
	}
	ImGui::End();
}

static std::vector<GameObject*> temp;
void Editor::PickObj()
{
	//if (ImGuizmo::IsUsing() || ImGuizmo::IsOver()) return;
	MousePicker m{ &EditorMgr.mCamera };
	glm::vec2 mousePos = InputManager.RawMousePos();
	m.RayFromMouse(mousePos);
	auto& objects = mConfig.mCandidateObjects;
	if (objects.empty())
	{
		temp.clear();
		mConfig.mShowItemList = false;
		if(!ImGui::IsAnyWindowHovered() && MouseTriggered(MouseKey::LEFT) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
			selectedObj = nullptr;
		return;
	}
	auto obj = objects.front();
	
	RenderPreviewObject(obj, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

	if (MouseTriggered(MouseKey::LEFT) && !ImGui::IsAnyWindowHovered() && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
	{
		if (selectedObj && selectedObj->attaching)
			selectedObj->CreateChild(obj);
		else
		{
			selectedObj = obj;
			mConfig.mShowItemList = false;
		}	
	}

	if (selectedObj && mCamera.GetMode() == EditorCamera::eCamType::Spherical)
		mCamera.SetSphereTarget(selectedObj->mTransform.mPosition);
	if (!KeyDown(Key::LAlt))
	{
		if (MouseTriggered(MouseKey::MID) && !ImGui::IsAnyWindowHovered())// && !KeyDown(Key::LAlt) );
		{
			mConfig.mShowItemList = true;
			mConfig.mCurrentMousePos = mousePos;
			temp = mConfig.mCandidateObjects;
		}
	}

	ItemList(temp);
}

void Editor::RenderPreviewObject(GameObject* _obj, const glm::vec4& _color)
{
	geometry::aabb abbSelection;
	geometry::obb obbSelection;

	//TODO
	//if obj is in a visible space else , get another obj until in spacegeometry::obb obbSelection;
	if (_obj && _obj->GetSpace())
	{
		// If the object has a renderable component, it likely has a mesh
		auto* it_render = _obj->GetComponentType<renderable>();
		obbSelection.position = _obj->mTransform.mPosition;
		obbSelection.orientation = _obj->mTransform.GetRotMtx();
		if (!it_render)
		{
			obbSelection.halfSize = _obj->mTransform.mScale / 2.0f;
		}
		else 
		{
			abbSelection = it_render->GetAABB();
			obbSelection.halfSize = _obj->mTransform.mScale * (abbSelection.max - abbSelection.min) / 2.0f;
			//abbSelection.min = _obj->mTransform.mScale * abbSelection.min + _obj->mTransform.mPosition;
			//abbSelection.max = _obj->mTransform.mScale * abbSelection.max + _obj->mTransform.mPosition;
		}

		Debug::DrawOBB(obbSelection, _color);
	}
}

void Editor::PickCollider()
{
	MousePicker m{ &EditorMgr.mCamera };
	auto collider = m.RayFromMouseCollider(InputManager.RawMousePos());
	if (collider)
	{
		switch (collider->mShape)
		{
			case shape::AABB:
			case shape::OBB:
			{
				geometry::obb obb;
				obb.halfSize = collider->mScale / 2.0f;
				obb.position = collider->mOwner->mTransform.mPosition + collider->mOffset;
				obb.orientation = collider->mOrientationMtx;
				Debug::DrawOBB(obb, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				break;
			}
			case shape::SPHERICAL:
			{
				geometry::sphere sphere;
				sphere.mCenter = collider->mOwner->mTransform.mPosition + collider->mOffset;
				sphere.mRadius = collider->mScale.x;
				Debug::DrawSphere(sphere, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				break;
			}
		}
		if (MouseTriggered(MouseKey::LEFT) && !ImGui::IsAnyWindowHovered() && !ImGuizmo::IsOver())
			selectedObj = collider->mOwner;
	}
}


void Editor::EditObj()
{
	if (!selectedObj)
		return;
	RenderPreviewObject(selectedObj, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	std::string name = "Edit GameObject";

	if (ImGui::Begin("Edit GameObject"))
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(320, 450), ImGuiCond_Once);

		ImGui::PushID(selectedObj->GetUID());
		selectedObj->Edit();
		ImGui::PopID();
	}

	ImGui::End();
}

void Editor::Copy(GameObject* obj)
{
	serializer.Clipboard(obj);
}

GameObject* Editor::Paste()
{
	GameObject* obj = Scene.CreateObject();
	serializer.LoadClipboard(obj);

	std::string name = obj->GetName();
	name += "_copy";
	obj->SetName(name.data());

	return obj;
}
