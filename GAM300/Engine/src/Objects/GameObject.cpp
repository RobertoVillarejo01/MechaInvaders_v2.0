#pragma once
#include "Components.h"
#include "System/Space/Space.h"
#include "System/Scene/SceneSystem.h"
#include "Serializer/Factory.h"
#include "Utilities/Input/Input.h"
#include "Window/Window.h"
#include "System/Memory/MemoryMgr.h"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

#ifdef EDITOR

#include "../Editor/ImGui/imgui.h"
#include "../Editor/ImGui/imgui_impl_sdl.h"
#include "../Editor/ImGui/imgui_impl_opengl3.h"
#include "../Editor/ImGui/ImGuizmo.h"
#include "../Editor/src/Editor.h"
#include "Undo/Undo.h"

#endif // EDITOR


constexpr unsigned MAX_NAME_SIZE = 50;

#ifdef EDITOR

void GameObject::Edit()
{
	char name_temp[MAX_NAME_SIZE];
	strcpy(name_temp, mName.c_str());

	ImGui::PushItemWidth(160);
	ImGui::InputText("Name", name_temp, MAX_NAME_SIZE);

	mName = name_temp;

	const char* Tag[] = { "None", "GameObjects", "Player", "Enemy", "EnemyBullet", "Spawner", "SpawnPoint", "Task", "Ramp", "Door", "VendingMachine", "Icon" };
	if (ImGui::BeginCombo(" Tags", Tag[static_cast<int>(mTag)]))
	{
		if (ImGui::Selectable(" - None - "))
			mTag = Tags::None;
		if (ImGui::Selectable(" - GameObjects - "))
			mTag = Tags::GameObjects;
		if (ImGui::Selectable(" - Player - "))
			mTag = Tags::Player;
		if (ImGui::Selectable(" - Enemy - "))
			mTag = Tags::Enemy;
		if (ImGui::Selectable(" - EnemyBullet - "))
			mTag = Tags::EnemyBullet;
		if (ImGui::Selectable(" - Spawner - "))
			mTag = Tags::Spawner;
		if (ImGui::Selectable(" - SpawnPoint - "))
			mTag = Tags::SpawnPoint;
		if (ImGui::Selectable(" - Task - "))
			mTag = Tags::Task;
		if (ImGui::Selectable(" - Ramp - "))
			mTag = Tags::Ramp;
		if (ImGui::Selectable(" - Door - "))
			mTag = Tags::Door;
		if (ImGui::Selectable(" - VendingMachine - "))
			mTag = Tags::VendingMachine;
		if (ImGui::Selectable(" - Icon - "))
			mTag = Tags::Icon;

		ImGui::EndCombo();
	}

	ImGui::Separator();
	ImGui::Separator();
	EditAddComp();
	ImGui::SameLine();
	if (ImGui::Button("Archetype", { 100, 20 }))
	{
		serializer.SaveArchetype(mName.c_str(), this);
	}
	ImGui::Separator();
	ImGui::Separator();

	if (ImGui::TreeNode("Transform"))
	{
		ImGui::Separator();
		EditTransform();
		ImGui::TreePop();
	}
	

	ImGui::Separator();
	EditCompList();
	ImGui::Separator();

	ImGui::Spacing();
	
	if (edit_child && selected_child && EditorMgr.selectedObj == this)
	{
		ImGui::SetNextWindowPos(ImVec2(350, 30), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(320, 350), ImGuiCond_Once);
		if (ImGui::Begin("Edit Child", &edit_child))
		{
			ImGui::PushID(selected_child->GetUID());
			selected_child->Edit();
			ImGui::PopID();
		}
		ImGui::End();
	}

	if (ImGui::TreeNode("Childs"))
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		if (ImGui::Selectable("Create Child")) CreateChild();
		ImGui::Checkbox("Attach Child", &attaching);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		for (GameObject* child : mChilds)
		{
			ImGui::PushID(child);
			if (ImGui::Button("Edit"))
			{
				edit_child = true;
				selected_child = child;
				if (mParent)
					EditorMgr.selectedObj = this;
			}			

			char name_child[MAX_NAME_SIZE];
			strcpy(name_child, child->mName.c_str());

			ImGui::SameLine();
			ImGui::PushItemWidth(110);
			ImGui::InputText("Name", name_child, MAX_NAME_SIZE);
			child->mName = name_child;

			ImGui::SameLine();
			if (ImGui::Button("erase"))
			{
				child->mParent = nullptr;
				auto it = std::find(mChilds.begin(), mChilds.end(), child);
				mChilds.erase(it);
				ImGui::PopID();
				break;
			}

			ImGui::SameLine(ImGui::GetWindowWidth() - 35);
			if (ImGui::Button("", ImVec2(14, 14))) {
				child->Destroy();
				ImGui::PopID();
				break;
			}

			ImGui::PopID();
		}
		ImGui::TreePop();
	}
}

void GameObject::EditCompList()
{
	for (auto& it : mComps)
	{
		std::string comp_name = TypeInfo(typeid(*it)).get_name();
		comp_name = comp_name.substr(comp_name.find(' ') + 1);
		comp_name[0] = toupper(comp_name[0]);

		ImGui::PushID(it);
		if (ImGui::TreeNode(comp_name.data()))
		{
			it->Edit();
			ImGui::TreePop();
		}
		else
		{
			ImGui::SameLine(ImGui::GetWindowWidth() - 35);
			if (ImGui::Button("", ImVec2(14, 14))) {
				this->RemoveComp(it);
				ImGui::PopID();
				break;
			}
		}
		ImGui::PopID();
		ImGui::Separator();
	}
}

void GameObject::EditTransform()
{
	mTransform.prev_rot = mTransform.mOrientation;
	static glm::vec3 snapTranslation = glm::vec3(1.0f);
	static glm::vec3 snapRotation = glm::vec3(1.0f);
	static glm::vec3 snapScale = glm::vec3(1.0f);

	ImGui::PushID(this);
	static ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::LOCAL;

	if (InputManager.KeyIsTriggered(Key::W))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (InputManager.KeyIsTriggered(Key::E))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (InputManager.KeyIsTriggered(Key::R))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;

	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
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
	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		snap = snapTranslation;
		ImGui::InputFloat3("Snap", &snapTranslation.x);
		break;
	case ImGuizmo::ROTATE:
		snap = snapRotation;
		ImGui::InputFloat("Angle Snap", &snapRotation.x);
		break;
	case ImGuizmo::SCALE:
		snap = snapScale;
		ImGui::InputFloat("Scale Snap", &snapScale.x);
		break;
	}
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	if (ImGui::Button("ResetPosition"))
	{
		mTransform.mPosition = { 0.0f, 0.0f, 0.0f };
	}
	if (ImGui::Button("ResetVectors"))
	{
		mTransform.mUpVect = { 0.0f, 1.0f, 0.0f };
		mTransform.mViewVector = { 0.0f, 0.0f, -1.0f };
		mTransform.mRightVector = { 1.0f, 0.0f, 0.0f };
	}


	float matrixTranslation[3], matrixRotation[3], matrixScale[3];

	//auto translate = glm::translate(glm::vec3(0.0f));
	auto translate = glm::translate(mTransform.mPosition);
	auto scale = glm::scale(mTransform.mScale);
	auto rotate = glm::mat4(1.0f);

	auto mModel2World = mParent && mbUseParentPosition ? mTransform.ModelToWorldOffset(mParent->mTransform.TransRotMtx()) : translate * rotate * scale;
	auto w2c = EditorMgr.mCamera.GetW2Cam();
	auto c2p = EditorMgr.mCamera.GetProj();

	ImGuizmo::Manipulate(&w2c[0][0], &c2p[0][0], mCurrentGizmoOperation, mCurrentGizmoMode, 
		&mModel2World[0][0], nullptr, useSnap ? &snap.x : nullptr);
	ImGuizmo::DecomposeMatrixToComponents(&mModel2World[0][0], matrixTranslation,
		matrixRotation, matrixScale);
	
	if (ImGui::DragFloat3("Position", matrixTranslation, 0.1f, -100000.0f, 100000.0f))
	{
		if (mGuiIsUsed)
		{
			mGuiIsUsed = false;
			storeChange = true;
		}
	}
	else mGuiIsUsed = true;

	if(ImGui::DragFloat3("Scale", matrixScale, 0.1f, 0.001f, 100000.0f)) 
	{
		if (mGuiIsUsed)
		{
			mGuiIsUsed = false;
			storeChange = true;
		}
	}
	else mGuiIsUsed = true;

	if(ImGui::DragFloat3("Rotation", &mTransform.mOrientation[0], 0.1f))
	{
		mTransform.RotateAround({ 1, 0, 0 }, mTransform.mOrientation[0] - mTransform.prev_rot[0]);
		mTransform.RotateAround({ 0, 1, 0 }, mTransform.mOrientation[1] - mTransform.prev_rot[1]);
		mTransform.RotateAround({ 0, 0, 1 }, mTransform.mOrientation[2] - mTransform.prev_rot[2]);
		if (mGuiIsUsed)
		{
			mGuiIsUsed = false;
			storeChange = true;
		}
	}
	else mGuiIsUsed = true;

	if (ImGuizmo::IsUsing())
	{
		if (mGuizmoIsUsed)
		{
			mGuizmoIsUsed = false;
			storeChange = true;
		}
	}
	else mGuizmoIsUsed = true;
	if (storeChange)
	{
		mChangesDone += 1;
		Undo::Instance().StoreChange(this);
		storeChange = false;
	}
	//security check for the scale so it doesnt get to negative or 0
	for (int i = 0; i < 3; i++)
	{
		if (matrixScale[i] < FLT_EPSILON)
			matrixScale[i] = 0.00001f;
	}

	mTransform.RotateAround({ 1, 0, 0 }, matrixRotation[0]);
	mTransform.RotateAround({ 0, 1, 0 }, matrixRotation[1]);
	mTransform.RotateAround({ 0, 0, 1 }, matrixRotation[2]);

	if (mParent && mbUseParentPosition)
		mTransform.mOffset = glm::inverse(mParent->mTransform.TransRotMtx()) * glm::vec4(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2], 1.0f);
	
	else  
		mTransform.mPosition = glm::vec3(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);

	mTransform.mScale = glm::vec3(matrixScale[0], matrixScale[1], matrixScale[2]);

	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &mModel2World[0][0]);

	ImGui::Checkbox("Use Parent Direction", &mbUseParentDirection);
	ImGui::Checkbox("Use Parent Position", &mbUseParentPosition);

	ImGui::PopID();
	IsEditing = ImGuizmo::IsOver();
}

void GameObject::EditAddComp()
{
	if (ImGui::BeginCombo("", "Add Component"))
	{
		for (auto& it : factory.getRegistered())
		{
			std::string comp_name = it;
			comp_name = comp_name.substr(comp_name.find(' ') + 1);
			comp_name[0] = toupper(comp_name[0]);

			ImGui::Bullet(); ImGui::SameLine();
			if (ImGui::Selectable(comp_name.c_str()))
			{
				IComp* c = factory.Create(it.c_str(), mSpace);
				c->mOwner = this;
				c->Initialize();
				AddComp(c);
			}

		}

		ImGui::EndCombo();
	}
}

#endif

GameObject::GameObject() : IBase()
{
	mbEnabled = false;
	mTag = Tags::GameObjects;
	mName = "GameObject";
}

GameObject::GameObject(GameObject* object)
{
	mbEnabled = object->mbEnabled;
	mTag = object->mTag;
	mSpace = object->mSpace;

	std::for_each(object->mComps.begin(), object->mComps.end(),
		[&](IComp* comp)
		{
			mComps.push_back(comp->Clone());
		});
}

const GameObject* GameObject::operator=(GameObject* _go)
{
	GameObject* object = mSpace->CreateObject(_go);
	object->mbEnabled = _go->mbEnabled;
	object->mTag = _go->mTag;
	object->mSpace = _go->mSpace;

	std::for_each(_go->mComps.begin(), _go->mComps.end(),
		[&](IComp* comp)
		{
			object->mComps.push_back(comp->Clone());
		});
	return object;
}

GameObject::~GameObject()
{
}

// ----------------------------------------------------------------------------
#pragma region// STATE METHODS

void GameObject::SetEnabled(bool enabled) // Call Set Enabled on all components
{
	// call base method
	mbEnabled = enabled;

	// delegate to components
	for (IComp* comp : mComps)
		comp->SetEnabled(enabled);
}
void GameObject::Initialize()
{	
	mbEnabled = false;
	mTag = Tags::GameObjects;
	mName = "GameObject";
}

void GameObject::Update()
{
	//update childs
	std::for_each(mChilds.begin(), mChilds.end(),
		[&](GameObject* obj)
		{
			if(obj->mbUseParentPosition)
				obj->mTransform.mPosition = mTransform.TransRotMtx() * glm::vec4(obj->mTransform.mOffset, 1.0f);

			if (obj->mbUseParentDirection)
			{
				obj->mTransform.mViewVector = mTransform.mViewVector;
				obj->mTransform.mUpVect = mTransform.mUpVect;
				obj->mTransform.mRightVector = mTransform.mRightVector;
			}
			//obj->mTransform.mViewVector = mTransform.TransRotMtx() * glm::vec4(obj->mTransform.mViewChild, 0.0f);
			//obj->mTransform.mUpVect = mTransform.TransRotMtx() * glm::vec4(obj->mTransform.mUpChild, 0.0f);
			//obj->mTransform.mRightVector = mTransform.TransRotMtx() * glm::vec4(obj->mTransform.mRightChild, 0.0f);
		});
	//mTransform.UpdateRotationVectors();
}

void GameObject::Shutdown()
{
	if (mParent)
	{
		auto it = std::find(mParent->mChilds.begin(), mParent->mChilds.end(), this);
		if(it != mParent->mChilds.end())
			mParent->mChilds.erase(it, it + 1);

#ifdef EDITOR
		if (mParent->selected_child == this)
			mParent->selected_child = nullptr;
#endif // EDITOR

	}

	for (GameObject* obj : mChilds)
	{
		obj->Shutdown();
		obj->Destroy();
	}
}

#pragma endregion


#pragma region// COMPONENT MANAGEMENT

// Find Component
size_t GameObject::GetCompCount() const
{
	return mComps.size();
}

void GameObject::InitComp()
{
	for (GameObject* child : mChilds)
		child->InitComp();

	for (auto it : mComps)
		it->Initialize();
}
// Add/Remove by address
IComp* GameObject::AddComp(IComp* pComp)
{
	if (!pComp) return nullptr;

	pComp->mOwner = this;
	for (IComp * comp : mComps) 
	{
		Collider* is_collider = dynamic_cast<Collider*>(pComp);
		if (is_collider == nullptr)
		{
			if (comp->are_the_same_type(*pComp))
			{
				mSpace->DestroyComp(pComp);
				return nullptr;
			}
		}
	}
	mComps.push_back(pComp);
	return pComp;
}

void GameObject::RemoveComp(IComp* pComp)
{
	if (!pComp)
		return;

	auto it = std::find(mComps.begin(), mComps.end(), pComp);
	if (it != mComps.end())
	{
		pComp->Shutdown();
		mComps.erase(it);
		mSpace->DestroyComp(pComp);
	}
}
#pragma endregion

void GameObject::Destroy()
{
	mSpace->DestroyObject(this);
}

void GameObject::ChangeSpace(Space* space)
{
	if (space == nullptr)
		return;

	Scene.ChangeObjSpace(this, space);
}

GameObject* GameObject::CreateChild(GameObject* _obj)
{
	GameObject* new_child = _obj;

	if (new_child == nullptr)
	{
		new_child = mSpace->CreateObject();
		new_child->SetName("Child");
	}
	else
	{
		auto it = std::find(mChilds.begin(), mChilds.end(), new_child);
		if (it != mChilds.end())
			return nullptr;
		if (!new_child->mParent)
			new_child->mTransform.mOffset = new_child->mTransform.mPosition - mTransform.mPosition;
			
	}
	
	new_child->mParent = this;
	mChilds.push_back(new_child);

	return new_child;
}

void GameObject::DestroyChild(GameObject* _obj)
{
	if (_obj == nullptr) return;

	for (GameObject* childs : _obj->mChilds)
		_obj->DestroyChild(childs);

	auto it = std::find(mChilds.begin(), mChilds.end(), _obj);
	if (it == mChilds.end()) return;
	mChilds.erase(it, it + 1);

	_obj->mSpace->DestroyObject(_obj);
}

GameObject* GameObject::FindChild(std::string child_name)
{
	for (GameObject* obj : mChilds)
	{
		if (obj->GetName() == child_name)
			return obj;
	}
	return nullptr;
}

void GameObject::SetVisibility(bool _mbVisible)
{

}

bool operator<<(nlohmann::json& j, const GameObject& _rhs)
{
	if (_rhs.GetParent()) return false;

	j["Transform"] << _rhs.mTransform;
	j["mName"] << _rhs.mName;
	j["Tag"] << static_cast<int>(_rhs.mTag);
	j["UseParentDirection"] << _rhs.mbUseParentDirection;
	j["UseParentPos"] << _rhs.mbUseParentPosition;

	std::vector<IComp*> components =_rhs.GetComps();
	
	for (auto&c : components)
	{
		json temp;
		temp << *c;
		j["Components"].push_back(temp);
	}

	for (GameObject* child : _rhs.GetChilds())
	{
		json temp;
		GameObject* parent = child->mParent;
		child->mParent = nullptr;
		temp << *child;
		child->mParent = parent;
		j["Childs"].push_back(temp);
	}

	return true;
}

void operator>>(nlohmann::json& j, GameObject& _rhs)
{
	if (j.find("Transform") != j.end())
		j["Transform"] >> _rhs.mTransform;
	if (j.find("mName") != j.end())
		j["mName"] >> _rhs.mName;

	int tag = 0;
	if (j.find("Tag") != j.end())
		j["Tag"] >> tag; 
	_rhs.mTag = static_cast<Tags>(tag);

	if (j.find("UseParentDirection") != j.end())
		j["UseParentDirection"] >> _rhs.mbUseParentDirection;
	if (j.find("UseParentPos") != j.end())
		j["UseParentPos"] >> _rhs.mbUseParentPosition;

	if (j.find("Components") != j.end()) {
	
		json& components = *j.find("Components");
	
		for (auto it = components.begin(); it != components.end(); ++it) {
	
			json& compVal = *it;
			std::string str = compVal["Type"].get<std::string>();
			IComp* c = factory.Create(str.c_str(), _rhs.mSpace);
			c->mOwner = &_rhs;
			compVal >> *c;
			_rhs.AddComp(c);
		}
	}

	if (j.find("Childs") != j.end()) {

		json& childs = *j.find("Childs");

		for (auto it = childs.begin(); it != childs.end() && childs.size() != 0; ++it)
		{
			json& compVal = *it;
			GameObject* go = _rhs.GetSpace()->CreateObject();
			compVal >> *go;
			_rhs.mChilds.push_back(go);
			go->mParent = &_rhs;
		}
	}
}
