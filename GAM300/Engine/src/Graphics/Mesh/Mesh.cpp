#include <GL/glew.h>
#include <GL/gl.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Graphics/Model/Model.h"
#include "Graphics/ShaderProgram/ShaderProgram.h"
#include "Graphics/ParticleSystem/Particle.h"
#include "resourcemanager/Resourcemanager.h"

namespace GFX {

  Mesh::Mesh(aiMesh* _mesh, const aiScene* _scene, const std::string& _directory)
    : mDirectory{ _directory }
  {
    // Reserve enough memory for all the vertices
    mVertices.resize(_mesh->mNumVertices);

    // Reset the min and max points (for each coord)
    mMinVtx = glm::vec3{ std::numeric_limits<float>::max() };
    mMaxVtx = glm::vec3{ -std::numeric_limits<float>::max() };

    // Copy each of the mesh's vertices
    for (unsigned int i = 0; i < _mesh->mNumVertices; i++)
    {
      // Assimp uses its own vector class that doesn't directly convert to glm's vec3 class 
      // so we transfer the data to this placeholder glm::vec3 first.
      mVertices[i].Position.x = _mesh->mVertices[i].x;
      mVertices[i].Position.y = _mesh->mVertices[i].y;
      mVertices[i].Position.z = _mesh->mVertices[i].z;

      for (unsigned j = 0; j < 3; ++j) {
        mMinVtx[j] = glm::min(mMinVtx[j], mVertices[i].Position[j]);
        mMaxVtx[j] = glm::max(mMaxVtx[j], mVertices[i].Position[j]);
      }

      // Normals
      if (_mesh->HasNormals())
      {
        mVertices[i].Normal.x = _mesh->mNormals[i].x;
        mVertices[i].Normal.y = _mesh->mNormals[i].y;
        mVertices[i].Normal.z = _mesh->mNormals[i].z;
      }

      // Texture coordinates
      if (_mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
      {
        // A vertex can contain up to 8 different texture coordinates. We thus make the 
        // assumption that we won't use models where a vertex can have multiple texture 
        // coordinates so we always take the first set (0).
        mVertices[i].TexCoords.x = _mesh->mTextureCoords[0][i].x;
        mVertices[i].TexCoords.y = _mesh->mTextureCoords[0][i].y;

        // Tangent
        mVertices[i].Tangent.x = _mesh->mTangents[i].x;
        mVertices[i].Tangent.y = _mesh->mTangents[i].y;
        mVertices[i].Tangent.z = _mesh->mTangents[i].z;

        // Bitangent
        mVertices[i].Bitangent.x = _mesh->mBitangents[i].x;
        mVertices[i].Bitangent.y = _mesh->mBitangents[i].y;
        mVertices[i].Bitangent.z = _mesh->mBitangents[i].z;
      }
      else
        mVertices[i].TexCoords = glm::vec2(0.0f, 0.0f);
    }

    // Reserve the aproximate amount of memory we will use (probably the minimum and if the mesh
    // is triangulated likely the exact amount)
    mIndices.reserve(_mesh->mNumFaces * 3);

    // Walk through each of the mesh's faces and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < _mesh->mNumFaces; i++)
    {
      aiFace face = _mesh->mFaces[i];

      // Retrieve all indices of the face and store them in the indices vector
      for (unsigned int j = 0; j < face.mNumIndices; j++)
        mIndices.push_back(face.mIndices[j]);
    }

    // Process materials
    aiMaterial* material = _scene->mMaterials[_mesh->mMaterialIndex];

    // Get the material properties
    for (unsigned i = 0; i < material->mNumProperties; ++i)
    {
      auto& mat_prop = material->mProperties[i];

      if (mat_prop->mKey == aiString("$clr.ambient"))
        mMaterial.mAmbient = *(reinterpret_cast<glm::vec3*>(mat_prop->mData));
      else if (mat_prop->mKey == aiString("$clr.diffuse"))
        mMaterial.mDiffuse = *(reinterpret_cast<glm::vec3*>(mat_prop->mData));
      else if (mat_prop->mKey == aiString("$clr.specular"))
        mMaterial.mSpecular = *(reinterpret_cast<glm::vec3*>(mat_prop->mData));
      else if (mat_prop->mKey == aiString("$mat.shininess"))
        mMaterial.mShininess = *(reinterpret_cast<float*>(mat_prop->mData));
    }

    // Load the textures of the different types
    LoadMaterialTextures(material, aiTextureType_DIFFUSE, Texture2D::TextureType::Diffuse);
    LoadMaterialTextures(material, aiTextureType_SPECULAR, Texture2D::TextureType::Specular);
    LoadMaterialTextures(material, aiTextureType_HEIGHT, Texture2D::TextureType::Normal);
    LoadMaterialTextures(material, aiTextureType_AMBIENT, Texture2D::TextureType::Height);

    // Return a mesh object created from the extracted mesh data
    UploadMesh();

    // Models laoded this way do use materials (or at least they probably should)
    mShaderProgram = Shader_t::Lighting;
  }

  Mesh::Mesh(const std::vector<glm::vec3>& positions, const std::vector<unsigned>& indices)
  {
    // Reset the min and max points (for each coord)
    mMinVtx = glm::vec3{std::numeric_limits<float>::max()};
    mMaxVtx = glm::vec3{-std::numeric_limits<float>::max() };

    // Copy the vertex positions, we will not be using normals or any of the fancy stuff
    this->mVertices.resize(positions.size());
    for (unsigned i = 0; i < (unsigned)positions.size(); ++i)
    {
      this->mVertices[i].Position = positions[i];

      for (unsigned j = 0; j < 3; ++j) {
        mMinVtx[j] = glm::min(mMinVtx[j], mVertices[i].Position[j]);
        mMaxVtx[j] = glm::max(mMaxVtx[j], mVertices[i].Position[j]);
      }
    }

    // Set the indices
    this->mIndices = indices;

    // Models laoded this way won't use materials
    mShaderProgram = Shader_t::Basic;

    // Now that we have all the required data, set the vertex buffers and its 
    // attribute pointers.
    //UploadMesh();
  }

  Mesh::~Mesh()
  {
    DestroyData();
  }

  void Mesh::UploadMesh()
  {
    // create buffers/arrays
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &mVBO);
    glGenBuffers(1, &mEBO);

    mbLoaded = true;

    glBindVertexArray(mVAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0],
      GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(unsigned int),
      &mIndices[0], GL_STATIC_DRAW);

    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, TexCoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
  }

  void Mesh::DestroyData()
  {
    if (!mbLoaded) return;

    mbLoaded = false;

    // Delete the OpenGL resources used
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mVBO);
    glDeleteBuffers(1, &mEBO);

    if (mInstanceBO != 0)
        glDeleteBuffers(1, &mInstanceBO);
    mInstanceBO = 0;
  }

  GLuint Mesh::GetSize() const
  {
    return static_cast<unsigned>(mIndices.size());
  }

  GLuint Mesh::GetShape() const
  {
    switch (mShape)
    {
    case Shape_t::Point:
      return GL_POINTS;
    case Shape_t::Line:
      return GL_LINES;
    case Shape_t::Triangles:
    default:
      return GL_TRIANGLES;
    }
  }

  void Mesh::GenerateParticleInstanceBuffer()
  {
      // Bind our first VAO
      glBindVertexArray(mVAO);

      // INSTANCE DATA //

      glGenBuffers(1, &mInstanceBO);
      glBindBuffer(GL_ARRAY_BUFFER, mInstanceBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleSystem::Particle) * MAX_PARTICLE_COUNT, 
          NULL, GL_STATIC_DRAW);

      glEnableVertexAttribArray(5);
      glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE,
          sizeof(ParticleSystem::Particle), 0);				// instance position
      glEnableVertexAttribArray(6);
      glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleSystem::Particle),
          reinterpret_cast<void*>(sizeof(float) * 3));		// instance scale
      glEnableVertexAttribArray(7);
      glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleSystem::Particle),
          reinterpret_cast<void*>(sizeof(float) * 6));	    // instance color

      // Mark this paramters as to change per instance 
      glVertexAttribDivisor(5, 1);
      glVertexAttribDivisor(6, 1);
      glVertexAttribDivisor(7, 1);

      /* Unbid */
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      mShaderProgram = Shader_t::Particle;
  }

  void Mesh::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, 
    Texture2D::TextureType typeName)
  {
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
      // Get the name of the texture (only the file name not the full directory)
      aiString str;
      mat->GetTexture(type, i, &str);

      // Get a filepath the Resource manager can work with
      std::string path(mDirectory + "/" + std::string(str.C_Str()));

      // Load the actual resource (or just retrieve it if it was already loaded)
      auto resource = ResourceMgr.GetResource<Texture2D>(path.c_str());
      if (!resource) continue;

      // Set the texture tupe and add it to the mesh's texture vector
      resource->get()->mType = typeName;
      mTextures.push_back(resource);
    }
  }
}
