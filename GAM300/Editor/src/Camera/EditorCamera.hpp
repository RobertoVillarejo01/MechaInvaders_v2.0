#include "Graphics/Camera/Camera.h"

using namespace GFX;

class EditorCamera : public Camera
{
public:
    enum class eCamType
    {
        Regular,
        FreeMovement,
        Spherical
    };

    void Reset();

    void SetSphereTarget(glm::vec3 targ);
    void SetMode(eCamType _type, const glm::vec3& target);
    void Move();
    eCamType  GetMode();
    void SetW2Cam(const glm::mat4& mat, GameObject* obj = nullptr);

private:

    // Different camera movements (through input)
    void Regular();
    void Free();
    void Spherical();

    // Change the position or orientation of the camera
    void MoveHorizontally(float factor);
    void MoveVertically(float factor);

    void ComputePosSphere();

    glm::vec3 Rotate(const glm::vec2& displacement);
    void RotateHorizontally(float factor);
    void RotateVertically(float factor);

    void Zoom(float factor);

    glm::vec2 mSphereRots = {};
    glm::vec3 mSpherePos = {};
    float mSphereRad = 0.0F;

    eCamType mCamType = eCamType::Regular;

    glm::vec2 mMousePrevPosition = {};
    float     mSpeed = 2.0f;
};