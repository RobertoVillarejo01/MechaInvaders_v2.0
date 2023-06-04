#include "Space.h"
#include "LogicSystem/LogicSystem.h"
#include "Graphics/ParticleSystem/Particle.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#include "../Editor/src/Editor.h"
#endif // EDITOR


void Space::AddObject(GameObject* object)
{
	if (object == nullptr) return;

	object->mSpace = this;
	alive_objects.push_back(object);

	for (IComp* comp : object->mComps)
		AddComp(comp);
}

GameObject* Space::CreateObject(GameObject* object)
{
	GameObject*	new_object = MemoryMgr.CreateEntity<GameObject>(object);

	new_object->mSpace = this;
	alive_objects.push_back(new_object);
	return new_object;
}

void Space::DestroyObject(GameObject* object)
{
	//check if object null
	if (object == nullptr) return;

	//destroy the components
	while (!object->mComps.empty())
	{
		DestroyComp(object->mComps.back());
		object->mComps.pop_back();
	}

	//destroy the object
	auto it = std::find(alive_objects.begin(), alive_objects.end(), object);
	if (it != alive_objects.end())
	{
		object->Shutdown();
		dead_objects.push_back(*it);
		alive_objects.erase(it, it + 1);
	}
}

void Space::RemoveObject(GameObject* object)
{
	//check if object null
	if (object == nullptr) return;

	//destroy the components
	for (IComp* comp : object->mComps )
		RemoveComp(comp);

	//destroy the object
	auto it = std::find(alive_objects.begin(), alive_objects.end(), object);
	if (it != alive_objects.end())
		alive_objects.erase(it, it + 1);
}

void Space::RemoveComp(IComp* comp)
{
	TypeInfo type{ typeid(*comp) };

	auto& vector = alive_components[type];
	auto it = std::find(vector.begin(), vector.end(), comp);
	if (it != vector.end())
		vector.erase(it, it + 1);
}

void Space::FreeDestroyedObjects()
{
	while (!dead_objects.empty())
	{
		GameObject* temp = dead_objects.back();
		dead_objects.pop_back();

		//CHANGE ONCE THE MEMORY MANAGER IS DONE//
		MemoryMgr.DestroyEntity(temp);
	}

	while (!dead_components.empty())
	{
		IComp* comp = dead_components.back();
		dead_components.pop_back();
		comp->Shutdown();

		//CHANGE ONCE THE MEMORY MANAGER IS DONE//
		//delete temp;
		MemoryMgr.DestroyEntity<IComp>(comp);
	}
}
void Space::ShutDown()
{
	//destroy dead objects
	FreeDestroyedObjects();

	//destroy alive components
	std::for_each(alive_components.begin(), alive_components.end(),
		[&](auto& it)
		{
			for (auto& comp : it.second)
			{
				comp->Shutdown();
			}
		});

	//destroy alive objects
	while (!alive_objects.empty())
	{
		GameObject* temp = alive_objects.back();
		alive_objects.pop_back();

		//CHANGE ONCE THE MEMORY MANAGER IS DONE//
		MemoryMgr.DestroyEntity(temp);
	}

	//destroy alive components
	std::for_each(alive_components.begin(), alive_components.end(),
		[&](auto& it)
		{
			while (!it.second.empty())
			{
				IComp* comp = it.second.back();
				it.second.pop_back();

				//CHANGE ONCE THE MEMORY MANAGER IS DONE//
				//delete temp;
				MemoryMgr.DestroyEntity<IComp>(comp);
			}
		});
}

void Space::Initialize()
{
	for (auto& comp_type : alive_components) {

		int i = 0;
		while (i < comp_type.second.size()) {
			auto& it = comp_type.second[i];
			it->Initialize();
			i++;
		}
	}
}

void Space::Update()
{
	std::for_each(alive_objects.begin(), alive_objects.end(),
		[](GameObject* obj)
		{
			obj->Update();
		});
}

const std::vector<GameObject*>& Space::GetAliveObjects()
{
	return alive_objects;
}

//OBJECT GETTORS
GameObject* Space::FindObject(std::string name)
{
	for (GameObject* obj : alive_objects)
		if (obj->GetName() == name)
			return obj;

	return nullptr;
}

GameObject* Space::FindObject(int id)
{
	for (GameObject* obj : alive_objects)
		if (obj->GetUID() == id)
			return obj;

	return nullptr;
}

std::vector<GameObject*> Space::FindObjects(std::string name)
{
	std::vector<GameObject*> result;
	std::for_each(alive_objects.begin(), alive_objects.end(),
		[&](GameObject* obj)
		{
			if (obj->GetName() == name)
				result.push_back(obj);
		});
	return result;
}


std::vector<GameObject*> Space::FindObjects(Tags tag)
{
	std::vector<GameObject*> result;
	std::for_each(alive_objects.begin(), alive_objects.end(),
		[&](GameObject* obj)
		{
			if (obj->mTag == tag)
				result.push_back(obj);
		});
	return result;
}

//COMPONENTS FEATURES
void Space::AddComp(IComp* comp)
{
	TypeInfo type{ typeid(*comp) };

	if (dynamic_cast<ILogic*>(comp))      
		type = { typeid(ILogic) };
	if (dynamic_cast<IRenderable*>(comp) && !dynamic_cast<ParticleSystem*>(comp))
		type = { typeid(IRenderable) };

	alive_components[type];
	alive_components[type].push_back(comp);
}

void Space::DestroyComp(IComp* comp)
{
	TypeInfo type{ typeid(*comp) };

	if (dynamic_cast<ILogic*>(comp))      
		type = { typeid(ILogic) };
	if (dynamic_cast<IRenderable*>(comp) && !dynamic_cast<ParticleSystem*>(comp))
		type = { typeid(IRenderable) };

	auto& vector = alive_components[type];
	auto it = std::find(vector.begin(), vector.end(), comp);
	if (it != vector.end())
	{
		dead_components.push_back(comp);
		vector.erase(it, it + 1);
	}
}

/*std::map<TypeInfo, std::vector<IComp*>*> Space::GetLogicComps()
{
	std::map<TypeInfo, std::vector<IComp*>*> logics;
	std::for_each(alive_components.begin(), alive_components.end(),
		[&](auto& it)
		{
			if (it.second.size() && dynamic_cast<ILogic*>(it.second.front()))
				logics[it.first] = &it.second;
		});
	return logics;
}*/


nlohmann::json& Space::operator<<(nlohmann::json& j) const
{
	j["Space Name"] << space_name;
	j["Visible"] << mbVisible;

	for (GameObject* go : alive_objects)
	{
		json goJson;
		if(goJson << *go)
			j["GameObjects"].push_back(goJson);
	}
	
	return j;
}

void Space::operator>>(nlohmann::json& j)
{
	j["Space Name"] >> space_name;
	j["Visible"] >> mbVisible;

	if (j.find("GameObjects") != j.end())
	{
		json& gameObjects = *j.find("GameObjects");

		for (auto it = gameObjects.begin(); it != gameObjects.end() && gameObjects.size() != 0; ++it)
		{
			json& compVal = *it;
			GameObject* go = CreateObject();
			compVal >> *go;
		}
	}
}

#ifdef EDITOR
void Space::Edit()
{
	if (ImGui::TreeNode(space_name.data()))
	{
		ImGui::Separator();
		ImGui::Separator();

		if (ImGui::Selectable("Create Object"))
		{
			auto newObj = CreateObject();
			newObj->mTransform.mPosition = EditorMgr.mCamera.GetPosition() + glm::normalize(-EditorMgr.mCamera.GetPosition() + EditorMgr.mCamera.GetTarget());
			EditorMgr.selectedObj = newObj;
		}

		ImGui::Separator();
		ImGui::Separator();

		static ImGuiTextFilter textFilter;

		textFilter.Draw("Search Object");
		if (ImGui::Button("Clear"))
			textFilter.Clear();
		ImGui::Separator();

		// Display each of the objects of the space
		for (unsigned i = 0; i < alive_objects.size(); i++)
		{
			if (textFilter.PassFilter(alive_objects[i]->GetName()))
			{
				ImGui::PushID(alive_objects[i]);

				// Show the name and if seleted set as "Selected Object"
				ImGui::Bullet(); ImGui::SameLine();
				ImVec2 size(ImGui::GetWindowWidth() - 96, 14);
				if (ImGui::Selectable(alive_objects[i]->GetName(), false, ImGuiSelectableFlags_None, size))
				{
					if (EditorMgr.selectedObj && EditorMgr.selectedObj->attaching)
						EditorMgr.selectedObj->CreateChild(alive_objects[i]);
					else
						EditorMgr.selectedObj = alive_objects[i];
				}

				// If clicked, remove the object
				ImGui::SameLine(ImGui::GetWindowWidth() - 35);
				ImGui::PushID(alive_objects[i] + 1);
				if (ImGui::Button("", ImVec2(14, 14)))
				{
					if (EditorMgr.selectedObj == alive_objects[i]) EditorMgr.selectedObj = nullptr;
					DestroyObject(alive_objects[i]);
					ImGui::PopID();
					ImGui::PopID();
					break;
				}

				ImGui::PopID();
				ImGui::PopID();
			}
		}
		ImGui::TreePop();
	}
}
#endif // EDITOR
