#pragma once

#include <string>
#include "glm/glm.hpp"
#include "Graphics/Renderable/Renderable.h"

using GLuint = unsigned;

namespace GFX {

  class Mesh;
  class Camera;

  class ShaderProgram
  {
  public:
    ~ShaderProgram();
    void CreateShader(std::string vert_shader_path, std::string frag_shader_path, 
      std::string geom_shader_path = "");
    void ReloadShader();

    GLuint GetHandle() const { return mProgramHandle; }

    virtual void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w,
      const Camera* _cam, renderable* _renderable) const {}

  protected:
    void         DestroyShader();
    virtual void SaveLocations() {}

    GLuint 				mProgramHandle = {};
    std::string   mVtxShaderPath = {};
    std::string   mFragShaderPath = {};
    std::string   mGeomShaderPath = {};
  };


  /**
   * It only has 2 uniforms (that all shaders should probably share), transformation mtx and color
   */
  class BasicShaderProgram : public ShaderProgram
  {
  public:
    void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w,
      const Camera* _cam, renderable* _renderable) const override;

  private:
    virtual void SaveLocations() override {}
  };



  /**
   * Appart from the basic, it also has materials and lights to care about (lights should probably go
   * with UBOs or smth similar)
   */
  class LightShaderProgram : public ShaderProgram
  {
  public:
    void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, 
      const Camera* _cam, renderable* _renderable) const override;

  private:
    virtual void SaveLocations() override;

    GLuint ambient_loc = 0;
    GLuint diffuse_loc = 0;
    GLuint specular_loc = 0;
    GLuint emissive_loc = 0;

    GLuint opacity_loc = 0;
    GLuint shininess_loc = 0;
    GLuint refractive_loc = 0;
  };


  /**
   * Appart from the basic, it also has materials and lights to care about (lights should probably go
   * with UBOs or smth similar)
   */
  class ToGBufferShaderProgram : public ShaderProgram
  {
  public:
      void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w,
          const Camera* _cam, renderable* _renderable) const override;

  private:
      virtual void SaveLocations() override {}
  };

  /**
   * It only has 2 uniforms (that all shaders should probably share), transformation mtx and color
   */
  class DepthCubeShaderProgram : public ShaderProgram
  {
  public:
    void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w, 
      const Camera* _cam, renderable* _renderable) const override;

  private:
    virtual void SaveLocations() override {}
  };



  /**
   * It only has 2 uniforms transformation mtx and sampler / cube texture
   */
  class CubemapShaderProgram : public ShaderProgram
  {
  public:
    void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w,
      const Camera* _cam, renderable* _renderable) const override;

  private:
    virtual void SaveLocations() override {}
  };



  /**
   * 2 uniforms: World 2 Cam mtx and particle texture
   */
  class ParticleShaderProgram : public ShaderProgram
  {
  public:
      void SetUniforms(const Mesh* _mesh, const glm::mat4& _m2w,
          const Camera* _cam, renderable* _renderable) const override;

  private:
      virtual void SaveLocations() override {}
  };
}