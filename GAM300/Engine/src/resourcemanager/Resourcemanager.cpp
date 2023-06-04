#include <fstream>
#include "Resourcemanager.h"
#include "resource.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#include "../Editor/ImGui/imgui_impl_sdl.h"
#include "../Editor/ImGui/imgui_impl_opengl3.h"
#endif // EDITOR

void ResourceManager::InitializeImporters()
{
	// Register the different loaders
	mImporters.insert({ "dummy", std::make_shared<DummyImporter>() });
	mImporters.insert({ "",		 std::make_shared<FolderImporter>() });

	mImporters.insert({ "obj",   std::make_shared<ModelImporter>() });
	mImporters.insert({ "fbx",   std::make_shared<ModelImporter>() });

	mImporters.insert({ "ttf",   std::make_shared<FontImporter>() });

	mImporters.insert({ "cube",  std::make_shared<CubemapImporter>() });
	mImporters.insert({ "jpg",   std::make_shared<TextureImporter>() });
	mImporters.insert({ "png",   std::make_shared<TextureImporter>() });

	mImporters.insert({ "wav", nullptr });
	mImporters.insert({ "mp3", nullptr });
}

void ResourceManager::LoadFromTXT(std::string _file)
{
	std::string path;
	std::ifstream myfile;

	if (_file.rfind("/") != std::string::npos)
		path = _file.substr(0, _file.rfind("/") + 1);

	myfile.open(_file);

	if (myfile.is_open())
	{
		std::string temp;
		while (getline(myfile, temp))	{
			LoadResource((path + temp).data());
		}

		myfile.close();
	}
}

inline void ResourceManager::Release()
{
}

inline void ResourceManager::RemoveResource(const char* key)
{
}

inline void ResourceManager::ShowGui()
{
}

inline void ResourceManager::LoadFolderMeta(std::string& folderName)
{
}

void ResourceManager::HotReload()
{
	for (auto& res_type : mResources)
	{
		for (auto it = res_type.second.begin(); it != res_type.second.end(); it++)
		{
			//we found that file has changed
			FileStatus currentTime;
			stat(it->first.c_str(), &currentTime);
			if (it->second->getModifiedT().st_mtime != currentTime.st_mtime)
			{
				std::string nameCopy = it->first;
				std::string extension = GetExtension(it->first);
				auto importer = mImporters.find(extension);

				auto newResource = importer->second->load(it->first.c_str(), true);
				it->second->setModifiedT();
				it->second->setName(nameCopy.c_str());
			}
		}
	}
}

#ifdef EDITOR

void ResourceManager::Edit()
{
	for (auto& res_type : mResources)
	{
		//auto name_begin = res_type.first.get_name().rfind(' ') + 1;
		//auto name_endin = res_type.first.get_name().find('>') - name_begin;
		//std::string clean_name = res_type.first.get_name().substr(name_begin, name_endin);

		if (ImGui::TreeNode(res_type.first.get_name().c_str()))
		{
			for (auto& it : res_type.second)
				ImGui::Text(it.first.c_str());
			ImGui::TreePop();
		}
	}
}

#endif

