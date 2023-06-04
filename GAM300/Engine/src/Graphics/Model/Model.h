#pragma once

#include <vector>
#include "Graphics/Mesh/Mesh.h"
#include "Geometry/Geometry.h"

struct aiNode;
struct aiScene;

namespace GFX {

  class Model
  {
  public:

    Model() = default;

    // Constructor, expects a filepath to a 3D model.
    explicit Model(std::string const& path) { LoadModel(path); }
    const geometry::aabb& GetAABB() const;

    std::vector<std::shared_ptr<Mesh>> mMeshes;

    // Some basic models (that are the default ones)
    static Model Quad;
    static Model Cube;
    static Model Sphere;
    static Model Cylinder;
    static Model Particle;

  private:
    // Loads a model with supported ASSIMP extensions from file
    void LoadModel(std::string const& path, bool use_materials = true);

    // Processes a node in a recursive fashion. 
    void ProcessNode(aiNode* node, const aiScene* scene);

    std::string    mDirectory;
    geometry::aabb mAABB;

    friend class RenderManager;
  };

  using ModelRes = std::shared_ptr<TResource<Model>>;

}