///Resource.h
///ibai.abaunza 
///Contains interface classes for the resources

#pragma once

#include <map>
#include <memory>
#include <vector>
#include <iostream>
#include <regex>

#include "Utilities/Singleton.h"
#include "System/TypeInfo/TypeInfo.h"
#include "resource.h"
#include "AudioManager/Audio.h"

class ResourceManager
{
	MAKE_SINGLETON(ResourceManager)

public:

	//put all the importers in the map
	void InitializeImporters();

	// Get the loaded resources or, if not loaded, try to load it
	template<typename T>
	std::shared_ptr<TResource<T>> GetResource(const char* name);

	// Get the vector of the loaded resources of these types
	template<typename T>
	const std::map<std::string, std::shared_ptr<IResource>>& GetResourcesOfType()
	{
		return mResources[typeid(TResource<T>)];
	}

	//load resource
	template<typename T>
	TResource<T>* LoadResourceT(const char* filename);
	
	//load resource class
	std::shared_ptr<IResource> LoadResource(const char* filename)
	{
		std::string extension = GetExtension(filename);
		auto importer = mImporters.find(extension);

		// Update the filename, just in case
		std::string modified_filename(filename);
		auto found = modified_filename.find("\\");
		while (found != std::string::npos)
		{
			modified_filename.replace(found, 1, "/");
			found = modified_filename.find("\\");
		}

		if (importer == mImporters.end())
		{
			std::cerr << "There is no appropiate importer registered for extension: " 
				<< extension << std::endl;
			return nullptr;
		}

		if (extension == "wav" || extension == "mp3")
		{
			gAudioMgr.LoadSound(modified_filename);
			return nullptr;
		}

		auto newResource = importer->second->load(modified_filename.c_str());

		// Sanity check, do not add resource if it is a nullptr
		if (!newResource) return newResource;
		newResource->setName(modified_filename);
		newResource->setModifiedT();

		mResources[TypeInfo(*newResource)][modified_filename] = newResource; //put in the map
		
		return newResource;
	}

	void LoadFromTXT(std::string _file);

	//release memory
	void Release();

	//delete resource
	void RemoveResource(const char* key);

	///show resources on scree
	void ShowGui();

	//load with metafiles
	void LoadFolderMeta(std::string& folderName);

	//check the files
	void HotReload();

#ifdef EDITOR
	//editing
	void Edit();
#endif

private:

	std::string GetExtension(const std::string& filename)
	{
		auto extension = filename.substr(filename.find_last_of(".") + 1);
		if (extension[0] == '/')
			return "";
		else
			return extension;
	}

	//maps for resources and importers
	std::map<TypeInfo, std::map<std::string, std::shared_ptr<IResource>>> mResources;
	std::map<std::string, std::shared_ptr<IresourceImporter>>      mImporters;
};

#define ResourceMgr (ResourceManager::Instance())

template<typename T>
inline std::shared_ptr<TResource<T>> ResourceManager::GetResource(const char* name)
{
	// Check if there are any resources of the expected type
	auto res_vector = mResources.find(TypeInfo(typeid(TResource<T>)));
	if (res_vector != mResources.end())
	{
		// IF there are, try to find the actual resource
		auto res = res_vector->second.find(name);
		if (res != res_vector->second.end())
		{
			// Get actual resource (casted to the specified type)
			auto rclass = std::dynamic_pointer_cast<TResource<T>>(res->second); 
			if (rclass) { return rclass; }
			else //just in case actual resource got deleted for some reason
			{
				return std::dynamic_pointer_cast<TResource<T>>(LoadResource(name));
			}
		}
	}

	// If the resource was not laoded, try to load it
	auto loaded_res = LoadResource(name);
	if (loaded_res)
	{
		return std::dynamic_pointer_cast<TResource<T>>(loaded_res);
	}
	else
	{
		// Error message
		std::cerr << "** Could not get resource: " << name << " **" << std::endl;
		return std::shared_ptr<TResource<T>>(nullptr);
	}
}

template<typename T>
inline TResource<T>* ResourceManager::LoadResourceT(const char* filename)
{

	return NULL;
}
