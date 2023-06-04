#include "Model.h"

#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace GFX {

  Model Model::Quad;
  Model Model::Cube;
  Model Model::Sphere;
  Model Model::Cylinder;
  Model Model::Particle;

  const geometry::aabb& Model::GetAABB() const
  {
    return mAABB; 
  }

  void Model::LoadModel(std::string const& path, bool use_materials)
  {
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
      std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
      return;
    }
    // retrieve the directory path of the filepath
    mDirectory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene);

    // Compute the aabb of the model
    mAABB.min = glm::vec3{ std::numeric_limits<float>::max() };
    mAABB.max = glm::vec3{ -std::numeric_limits<float>::max() };
    for (auto& mesh : mMeshes) {
      if(!mesh) continue;
      for (unsigned j = 0; j < 3; ++j) {
        mAABB.min[j] = glm::min(mAABB.min[j], mesh->mMinVtx[j]);
        mAABB.max[j] = glm::max(mAABB.max[j], mesh->mMaxVtx[j]);
      }
    }

    // If the models does not need materials
    if (use_materials == false)
    {
      for (auto& mesh : mMeshes)
      {
        mesh->mShaderProgram = Shader_t::Basic;
      }
    }
  }

  void Model::ProcessNode(aiNode* node, const aiScene* scene)
  {
    // Rrocess each mesh located at the current node (generate a mesh our engine understands)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
      // The node object only contains indices to index the actual objects in the scene. The scene 
      // contains all the data, node is just to keep stuff organized (like relations between nodes).
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      mMeshes.push_back(std::make_shared<Mesh>(mesh, scene, mDirectory));
    }

    // After we've processed all of the meshes (if any) we then recursively process 
    // each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
      ProcessNode(node->mChildren[i], scene);
  }

}