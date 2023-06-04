#include <filesystem>
#include "../Editor/src/Editor.h"
#include "System/Scene/SceneSystem.h"
#include "Window/Window.h"

void Editor::LevelTree()
{
	glm::vec2 size = WindowMgr.GetResolution();
	ImVec2 guiSize(300, 320);
	ImGui::SetWindowSize(guiSize);
	ImGui::SetNextWindowPos(ImVec2(size.x - guiSize.x, size.y - guiSize.y), ImGuiCond_Once);
	ImGui::Begin("Scene");

	ImGui::Spacing();
	unsigned i = 0;
	for (auto& space : Scene.GetSpaces())
		{
			// The spaces are organized in trees
			ImGui::PushID(space);

			bool visible = space->IsVisible();
			if (ImGui::Checkbox("", &visible))
				space->SetVisibility(visible);
			
			ImGui::SameLine();

			space->Edit();
			
			ImGui::PopID();
			i++;
		}
	ImGui::End();

}

