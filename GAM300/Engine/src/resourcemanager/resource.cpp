#include <chrono>
#include <iomanip>
#include <fstream>
#include <filesystem>

#include "Resourcemanager.h"
#include "resource.h"

#pragma region // HOT RELOAD //

FileStatus IResource::getModifiedT()
{
    return mStatus;
}

long long IResource::currentModT()
{
    return mLastModifiedT;
}

void IResource::setModifiedT()
{
    stat(mName.c_str(), &mStatus);
}

void IResource::setName(std::string _Name)
{
    mName = _Name;
}

std::string IResource::getName() const
{
    return mName;
}

#pragma endregion

#pragma region // MESHES //

#include "Graphics/Model/Model.h"

std::shared_ptr<IResource> ModelImporter::load(const char* filename, bool reload)
{
	if (!reload)
	{
		auto res = std::make_shared<TResource<GFX::Model>>();
		res->SetResource(new GFX::Model{ filename });
		return res;
	}
	else
	{
		auto res = ResourceMgr.GetResource<GFX::Model>(filename);
		res->Release();
		res->SetResource(new GFX::Model{ filename });
		return res;
	}
}

#pragma endregion

#pragma region // TEXTURES //

#define STB_IMAGE_IMPLEMENTATION
#include "Graphics/Mesh/Mesh.h"
#include "stbi/stb_image.h"
#include <GL/glew.h>
#include <GL/GL.h>

std::shared_ptr<IResource> TextureImporter::load(const char* filename, bool reload)
{
	// Get the resource
	std::shared_ptr<TResource<GFX::Texture2D>> res;

	// If we are reloading, just reuse the same TResource pointer as before, otherwise create 
	// a new one
	if (!reload)
	{
		res = std::make_shared<TResource<GFX::Texture2D>>();
		res->SetResource(new GFX::Texture2D{});
	}
	else
	{
		res = ResourceMgr.GetResource<GFX::Texture2D>(filename);
		res->Release();
		res->SetResource(new GFX::Texture2D{});
	}

	//stbi_set_flip_vertically_on_load(true);

  int width, height, nrComponents;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(filename, &width, &height, &nrComponents, 0);
  stbi_set_flip_vertically_on_load(false);
  if (data)
  {
    GLenum format = GL_RGBA;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

		res->get()->Bind();
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
  }
  else
  {
    stbi_image_free(data);
		return nullptr;
  }

	// Return the resource
	return res;
}

std::shared_ptr<IResource> CubemapImporter::load(const char* filename, bool reload)
{
	// Get the resource
	std::shared_ptr<TResource<GFX::Cubemap>> res;

	// If we are reloading, just reuse the same TResource pointer as before, otherwise create 
	// a new one
	if (!reload)
	{
		res = std::make_shared<TResource<GFX::Cubemap>>();
		res->SetResource(new GFX::Cubemap{});
	}
	else
	{
		res = ResourceMgr.GetResource<GFX::Cubemap>(filename);
		res->Release();
		res->SetResource(new GFX::Cubemap{});
	}

	// Open file and read the texture
	std::ifstream input_file(filename);

	// Sanity check
	if (!input_file.is_open()) {
		std::cerr << "Could not open file: " << filename << " When loading cubemap" << std::endl;
		return nullptr;
	}
	unsigned texture_count;
	input_file >> texture_count;

	// Get the path to the file
	std::string file_str(filename);
	auto path = file_str.substr(0, file_str.find_last_of("/") + 1);

	std::vector<std::string> textures;
	textures.resize(texture_count);
	for (unsigned i = 0; i < texture_count; ++i) {
		input_file >> textures[i];
		textures[i] = path + textures[i];
	}

	// Bind the cubemap and populate it
	res->get()->Bind();
	int width, height, nrComponents;
	for (unsigned i = 0; i < texture_count; ++i)
	{
		unsigned char* data = stbi_load(textures[i].data(), &width, &height, &nrComponents, 0);
		if (data)
		{
			GLenum format = GL_RGBA;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			res->get()->Bind();
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, 
				format, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			stbi_image_free(data);
			return nullptr;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// Return the resource
	return res;
}

#pragma endregion

#pragma region // FOLDERS //

std::shared_ptr<IResource> FolderImporter::load(const char* filename, bool reload)
{
	// Check if the folder exist
	if (!std::filesystem::exists(filename)) return nullptr;

	// Load everything inside
	for (auto i : std::filesystem::directory_iterator(filename)) {
		ResourceMgr.LoadResource((i.path()).generic_string().data());
	}

	return nullptr;
}

#pragma endregion

#pragma region // FONT RENDERING //
#include "Graphics/TextRendering/TextRender.h"

std::shared_ptr<IResource> FontImporter::load(const char* filename, bool reload)
{
	if (!reload)
	{
		auto res = std::make_shared<TResource<GFX::Font>>();
		res->SetResource(new GFX::Font{});
		res->get()->LoadFont(filename);
		return res;
	}
	else
	{
		auto res = ResourceMgr.GetResource<GFX::Font>(filename);
		res->Release();
		res->SetResource(new GFX::Font{});
		res->get()->LoadFont(filename);
		return res;
	}
}
#pragma endregion
