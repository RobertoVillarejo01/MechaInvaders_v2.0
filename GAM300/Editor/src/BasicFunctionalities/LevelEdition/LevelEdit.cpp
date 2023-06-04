#include <filesystem>
#include "../Editor/src/Editor.h"
#include "Window/Window.h"
#include "System/Scene/SceneSystem.h"


void Editor::EditLevel()
{
	glm::vec2 size = WindowMgr.GetResolution();
	ImGui::SetNextWindowPos(ImVec2(size.x - 300, size.y - 250), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_Once);
	std::string scene_name = std::string("Scene (" + Scene.GetCurrentLevel() + ")");
	if (ImGui::Begin(scene_name.c_str()))
	{
		if (ImGui::Selectable("Create Space"))
			CreateSpace();

		LevelTree();

		ImGui::End();
	}
}

void Editor::CreateSpace()
{
	glm::vec2 size = WindowMgr.GetResolution();
	ImGui::SetNextWindowPos(ImVec2(size.x / 2, size.y / 2), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Once);
	ImGui::Begin("New Space", &new_space, ImGuiWindowFlags_MenuBar);

	static char str0[128] = "";
	ImGui::InputText("Space Name", str0, IM_ARRAYSIZE(str0));

	if (ImGui::Button("Add")) 
		Scene.CreateSpace(str0);

	ImGui::End();
}

void Editor::DisplayMemory()
{
	glm::vec2 size = WindowMgr.GetResolution();
	ImGui::SetNextWindowPos(ImVec2(size.x - 300, 20), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_Once);
	ImGui::Begin("MEMORY", &MemoryOpen);
	MemoryMgr.ShowStats();
	ImGui::End();
}
