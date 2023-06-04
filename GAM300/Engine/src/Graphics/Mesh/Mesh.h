#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "Graphics/Texture/Texture.h"
#include "assimp/material.h"

using GLuint = unsigned;

struct aiMesh;
struct aiScene;
struct aiMaterial;

namespace GFX {

  struct Vertex
  {
    glm::vec3 Position = {};
    glm::vec3 Normal = {};
    glm::vec2 TexCoords = {};
    glm::vec3 Tangent = {};
    glm::vec3 Bitangent = {};
  };

  struct Material
  {
    glm::vec3 mAmbient{};
    float mShininess = 0.0f;

    glm::vec3 mDiffuse{};
    float __pad;
    glm::vec3 mSpecular{};
    float __pad2;
  };

  class Mesh /*: public IResource*/
  {
  public:
    Mesh() = default;
    Mesh(aiMesh* _mesh, const aiScene* _scene, const std::string& _directory);
    Mesh(const std::vector<glm::vec3>& positions, const std::vector<unsigned>& indices);
    ~Mesh();

    void UploadMesh();
    void DestroyData();

    GLuint GetHandle() const { return mVAO; }
    GLuint GetSize() const;
    GLuint GetShape() const;
    GLuint GetInstanceBuffer() const { return mInstanceBO; }
    Shader_t GetShaderProgram() const { return mShaderProgram; }

    const Material& GetMaterial() const { return mMaterial; }    
    unsigned mMaterialID;

    const std::vector<TextureRes>& GetTextures() const { return mTextures; }

    void SetShape(Shape_t _shape) { mShape = _shape; }
    void GenerateParticleInstanceBuffer();

    static Mesh Point;
    static Mesh Segment;

    glm::vec3 mMinVtx{};
    glm::vec3 mMaxVtx{};

  private:

    // Checks all material textures of a given type and loads the textures if they're 
    // not loaded yet. The required info is returned as a Texture struct.
    void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, 
      Texture2D::TextureType typeName);

    // Actual mesh data
    std::vector<Vertex>      mVertices;
    std::vector<unsigned>    mIndices;
    std::vector<TextureRes>  mTextures;
    std::string              mDirectory;

    Material mMaterial;

    // OpenGL handlers
    GLuint mVAO = {};
    GLuint mVBO = {};
    GLuint mEBO = {};
    GLuint mInstanceBO = {};

    bool      mbLoaded = false;
    Shape_t   mShape = Shape_t::Triangles;
    Shader_t  mShaderProgram = Shader_t::Basic;

    friend class Model;
  };

}
