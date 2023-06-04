#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "Objects/Components.h"
#include "Graphics/Texture/Texture.h"
#include "Graphics/Framebuffer/Framebuffer.h"

class GameObject;
class LightComponent;

namespace GFX {

  class Camera /* : public Serializable? Component? */
  {
  public:

    virtual void Update() /* override? */;

    CubemapRes GetCubemap() const { return mCubemap; }
    GBuffer const& GetGBuffer() const { return mGBuffer; }
    Framebuffer const& GetFramebuffer() const { return mFrameBuffer; }

    // Get the matrix for the transformations from World Space to
    // Projection Space
    const glm::mat4& GetProj() const;
    const glm::mat4& GetW2Proj() const;
    const glm::mat4& GetW2Cam() const;
    const glm::mat4& GetCam2Proj() const;
    const glm::vec3& GetPosition() const;
    const glm::vec3& GetTarget() const;

    // Directly set the variables of the camera
    void SetProjection(float angle, glm::ivec2 viewport_size, float near, float far);
    void SetPosition(glm::vec3 position);
    void SetTarget(glm::vec3 target);
    void SetUpVec(glm::vec3 up);

    // Pure Graphics part of the camera
    void SetCubeMap(CubemapRes _cubemap) { mCubemap = _cubemap; }

    //settors
    void SetNear(float near);
    void SetFar(float far);
    void SetAngle(float angle);
    void SetViewSize(glm::ivec2 viewsize);

    //gettors
    float GetNear() const { return mNear;}
    float GetFar() const { return mFar; }
    float GetAngle() const { return mAngle; }
    const glm::ivec2& GetViewSize() const { return mViewportSize; }

  protected:
    glm::vec3 mPosition = { 10.0f, 3.0f, 0.0f };
    glm::vec3 mTarget = {};
    glm::vec3 mUpVec = { 0.0f, 1.0f, 0.0f };

    glm::mat4 mW2Cam = {};
    glm::mat4 mCam2Proj = {};

    glm::mat4 mW2Proj = {};

    // General camera info
    float mNear = 0.05f;
    float mFar = 700.0f;
    float mAngle = 60.0f;
    glm::ivec2 mViewportSize = {};

    // Other rendering variables
    CubemapRes  mCubemap = nullptr;
    GBuffer     mGBuffer;
    Framebuffer mFrameBuffer;
    unsigned    mRenderPosition = 0;
  };

  class LightCamera : public Camera
  {
  public:
    void Update() override;
    void SetLight(LightComponent* _light);
  };
}

class CameraComp : public IComp, public GFX::Camera
{
public:
  void   Update() override;
  void   RegenerateBuffers();

#ifdef EDITOR
  bool   Edit() override;
#endif
  
  void   Initialize() override;
  IComp* Clone() override;
  void   LookAt(glm::vec3 target);

  void ToJson(nlohmann::json& j) const override;
  void FromJson(nlohmann::json& j) override;

  float mLightActionRadius = 300.0f;
  float mShadowLightRadius = 150.0f;

  bool render = true;
  bool IsObjectVisible(const glm::vec3& position, glm::vec3& _pos_projection);
private:

  bool mbStaticCamera = false;
  bool mbDefaultValues = true;
  bool mbDrawDebug = false;

  glm::vec2 mMousePos = {};
  glm::vec2 angle = {0.0f, 90.0f};
};
