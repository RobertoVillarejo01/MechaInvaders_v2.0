#include "MemoryMgr.h"
#include "System/Space/Space.h"
#include "System/Scene/SceneSystem.h"
#include "Serializer/Factory.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#include "../Editor/src/Editor.h"
#endif // EDITOR


void MemoryManager::Initialize()
{
}

void MemoryManager::ShutDown()
{
	//destroy alive components
	std::for_each(memory.begin(), memory.end(),
		[&](auto& it)
		{
			delete it.second;
		});
	memory.clear();
}

MemoryManager::~MemoryManager()
{
}

#ifdef EDITOR

void MemoryManager::ShowStats()
{
	unsigned id = 0;
	std::size_t total_entities = 0u, total_entities_allocated = 0u, total_memory_in_use = 0u, total_memory_allocated = 0u;

	//get total stats
	std::for_each(memory.begin(), memory.end(),
		[&](auto it)
		{
			TypeInfo type = it.first;
			size_t entity_size = memory[type]->GetTypeSize();
			total_entities += memory[type]->GetMemryInUse();
			total_entities_allocated += memory[type]->GetMemoryAllocated();
			total_memory_in_use += memory[type]->GetMemryInUse() * entity_size;
			total_memory_allocated += memory[type]->GetMemoryAllocated() * entity_size;
		});

	if (ImGui::Checkbox("Use Custom Memory", &use_custom_memory))
	{
		use_custom_memory = !use_custom_memory;
		serializer.SaveLevel("Lighting");
		Scene.ShutDown();
		EditorMgr.selectedObj = nullptr;
		use_custom_memory = !use_custom_memory;
		serializer.LoadLevel(Scene.GetCurrentLevel().c_str());
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Text("TOTAL MEMORY");
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	if (!use_custom_memory)
	{
		std::string obj_in_use =    std::string("Total Allocations    : ") + std::to_string(Total_allocations);
		std::string obj_allocated = std::string("Total Deallocations  : ") + std::to_string(Total_deallocations);
		ImGui::Text(obj_in_use.c_str());
		ImGui::Text(obj_allocated.c_str());
		return;
	}

	std::string obj_in_use = std::string("Entities in use    : ") + std::to_string(total_entities);
	std::string obj_allocated = std::string("Entities allocated : ") + std::to_string(total_entities_allocated);
	ImGui::Text(obj_in_use.c_str());
	ImGui::Text(obj_allocated.c_str());

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	std::string bytes(" bytes");
	std::string mem_in_use = std::string("Memory in use      : ") + std::to_string(total_memory_in_use) + bytes;
	std::string mem_allocated = std::string("Memory allocated   : ") + std::to_string(total_memory_allocated) + bytes;
	ImGui::Text(mem_in_use.c_str());
	ImGui::Text(mem_allocated.c_str());

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("ALL ENTITIES"))
	{
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		std::for_each(memory.begin(), memory.end(),
			[&](auto it)
			{
				TypeInfo type = it.first;
				ImGui::PushID(id++);

				std::string comp_name = type.get_name();
				comp_name = comp_name.substr(comp_name.find(' ') + 1);
				comp_name[0] = toupper(comp_name[0]);

				size_t entity_size = memory[type]->GetTypeSize();
				total_entities += memory[type]->GetMemryInUse();
				total_entities_allocated += memory[type]->GetMemoryAllocated();
				total_memory_in_use += memory[type]->GetMemryInUse() * entity_size;
				total_memory_allocated += memory[type]->GetMemoryAllocated() * entity_size;

				if (ImGui::TreeNode(comp_name.c_str()))
				{
					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					std::string obj_in_use = std::string("Entities in use    : ") + std::to_string(memory[type]->GetMemryInUse());
					std::string obj_allocated = std::string("Entities allocated : ") + std::to_string(memory[type]->GetMemoryAllocated());
					ImGui::Text(obj_in_use.c_str());
					ImGui::Text(obj_allocated.c_str());

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					std::string bytes(" bytes");
					std::string mem_in_use = std::string("Memory in use      : ") + std::to_string(memory[type]->GetMemryInUse() * entity_size) + bytes;
					std::string mem_allocated = std::string("Memory allocated   : ") + std::to_string(memory[type]->GetMemoryAllocated() * entity_size) + bytes;
					ImGui::Text(mem_in_use.c_str());
					ImGui::Text(mem_allocated.c_str());

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					std::string obj_size = std::string("Entity Size        : ") + std::to_string(entity_size) + bytes;
					std::string page_size = std::string("Page Size          : ") + std::to_string(memory[type]->GetPageSize() * entity_size) + bytes;
					std::string entities_per_page = std::string("Entities per Page  : ") + std::to_string(memory[type]->GetPageSize());
					ImGui::Text(obj_size.c_str());
					ImGui::Text(page_size.c_str());
					ImGui::Text(entities_per_page.c_str());

					ImGui::TreePop();
				}

				ImGui::PopID();
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Separator();
				ImGui::Spacing();
			});
		ImGui::TreePop();
	}
	
}
#endif // EDITOR
