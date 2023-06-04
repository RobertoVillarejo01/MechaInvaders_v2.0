#include <glm/gtx/transform.hpp>
#include "EditorCamera.hpp"
#include "Utilities/Input/Input.h"
#include "Utilities/FrameRateController/FrameRateController.h"
#include "Geometry/Geometry.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "Objects/GameObject.h"

void EditorCamera::MoveHorizontally(float factor)
{
  auto view_vec = mTarget - mPosition;
  auto right_vec = glm::normalize(glm::cross(view_vec, mUpVec));

  mPosition += right_vec * factor;
  mTarget += right_vec * factor;
}

void EditorCamera::MoveVertically(float factor)
{
  mPosition += mUpVec * factor;
  mTarget += mUpVec * factor;
}

glm::vec3 EditorCamera::Rotate(const glm::vec2& displacement)
{
    float dt = FRC.GetFrameTime();
    auto view = glm::normalize(mTarget - mPosition);

    auto  right = glm::normalize(glm::cross(view, { 0, 1, 0 }));
    auto  up = glm::normalize(glm::cross(view, right));

    // View
    auto cursor_delta = mMousePrevPosition - InputManager.WindowMousePos();

    view = glm::vec3(glm::vec4(view, 0) * glm::rotate(glm::radians(5.0f) * dt * displacement.y, right));
    view = glm::vec3(glm::vec4(view, 0) * glm::rotate(glm::radians(5.0f) * dt * -displacement.x, mUpVec));

    return view;
}


void EditorCamera::RotateHorizontally(float factor)
{
  factor *= 0.5f;

  auto view_vec = mTarget - mPosition;
  auto rotated_vec = glm::vec3(glm::rotate(glm::radians(factor), glm::vec3{ 0.0f,1.0f,0.0f })
    * glm::vec4(view_vec, 0.0f));

  mPosition = mTarget - rotated_vec;
}

void EditorCamera::RotateVertically(float factor)
{
  factor *= 0.5f;

  auto view_vec = mTarget - mPosition;
  auto right_vec = glm::normalize(glm::cross(view_vec, glm::vec3{ 0.0f,1.0f,0.0f }));

  auto rotated_vec = glm::vec3(glm::rotate(glm::radians(factor), right_vec)
    * glm::vec4(view_vec, 0.0f));

  if (glm::abs(glm::normalize(rotated_vec).y) < 0.975f)
    mPosition = mTarget - rotated_vec;
}

void EditorCamera::Zoom(float factor)
{
  if (ImGui::IsAnyWindowHovered()) return;

  //auto t = geometry::intersection_ray_plane(
  //  { mPosition, mTarget - mPosition }, { {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} });
  //
  //auto point = mPosition + (mTarget - mPosition) * t;
  ////Debug::DrawPoint(point, { 1.0f, 1.0f, 1.0f, 1.0f }, 4.0f);
  //
  //auto displacement = (point - mPosition) * factor * 0.3f / sqrtf(glm::distance(point, mPosition));
  //auto pos = mPosition + displacement;
  //
  //if (glm::dot(mTarget - mPosition, mTarget - pos) > 0)
  mPosition += (mTarget - mPosition) * factor;
}

void EditorCamera::Regular()
{
  // Control zoom
  Zoom(InputManager.GetWheelScroll() * 0.2f);

  // Initialize the previous mouse position when the wheel button is triggered
  if (MouseTriggered(MouseKey::RIGHT)) mMousePrevPosition = InputManager.WindowMousePos();

  // While the right button is pressed
  if (MouseDown(MouseKey::RIGHT))
  {
    // Get the mouse displacement between frames
    auto displacement = mMousePrevPosition - InputManager.WindowMousePos();

    // Depending on the mouse button used, move or rotate
    if (KeyDown(Key::LAlt))
    {
      RotateHorizontally(displacement.x);
      RotateVertically(-displacement.y);
    }
    else
    {
      MoveHorizontally(displacement.x);
      MoveVertically(displacement.y);
    }

    // Save the current mouse position for the next iteration
    mMousePrevPosition = InputManager.WindowMousePos();
  }
}

void EditorCamera::Free()
{
    Zoom(InputManager.GetWheelScroll() * 0.2f);
    if (MouseDown(MouseKey::RIGHT))
    {
        // View
        auto cursor_delta = mMousePrevPosition - InputManager.WindowMousePos();
        auto ViewVector = Rotate(cursor_delta * 3.0f);
    
        mTarget = mPosition + ViewVector;
    
        glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0.0F, 1.0F, 0.0F), ViewVector));
    
        float speed = KeyDown(Key::LShift) ? (2 * mSpeed) : mSpeed;
    
        if (KeyDown(Key::W))
        {
            mPosition += ViewVector * speed;
            mTarget += ViewVector * speed;
        }
    
        if (KeyDown(Key::S))
        {
            mPosition -= ViewVector * speed;
            mTarget -= ViewVector * speed;
        }
    
        if (KeyDown(Key::A))
        {
            mPosition += right * speed;
            mTarget += right * speed;
        }
    
        if (KeyDown(Key::D))
        {
            mPosition -= right * speed;
            mTarget -= right * speed;
        }
    
        if (KeyDown(Key::Space))
        {
            mPosition += mUpVec * speed;
            mTarget += mUpVec * speed;
        }
    }
    
    mMousePrevPosition = InputManager.WindowMousePos();
}

void EditorCamera::Spherical()
{
    bool moved = false;
    // Control radius of the sphere over which rotate around
    Zoom(InputManager.GetWheelScroll() * 0.2f);

    mSphereRad = glm::length(mTarget - mPosition);

    if (mUpVec != glm::vec3(0, 1, 0))
        mUpVec = glm::vec3(0, 1, 0);

    // Initialize the previous mouse position when the wheel button is triggered
    if (MouseTriggered(MouseKey::RIGHT)) mMousePrevPosition = InputManager.WindowMousePos();

    // While the wheel is pressed
    if (MouseDown(MouseKey::RIGHT))
    {
        moved = true;
        // Get the mouse displacement between frames
        auto Displacement = InputManager.WindowMousePos() - mMousePrevPosition;

        mSphereRots.x += Displacement.x * -0.1f;
        mSphereRots.y += Displacement.y * -0.1f;

        // Save the current mouse position for the next iteration
        mMousePrevPosition = InputManager.WindowMousePos();

    }
    
    if(moved)
        ComputePosSphere();

}

void EditorCamera::ComputePosSphere()
{
    //computing the position and setting it
    float posX = (mSphereRad * cosf(glm::radians(mSphereRots.y))) * sinf(glm::radians(mSphereRots.x));
    float posY = (mSphereRad * sinf(glm::radians(mSphereRots.y)));
    float posZ = (mSphereRad * cosf(glm::radians(mSphereRots.y))) * cosf(glm::radians(mSphereRots.x));

    mPosition = mTarget + glm::vec3(posX, posY, posZ);
}


void EditorCamera::SetW2Cam(const glm::mat4& mat, GameObject* obj)
{
    if (!obj) return;

    auto inverse = glm::inverse(glm::transpose(mat));

    glm::vec3 view(-mat[0][2], -mat[1][2], -mat[2][2]);
    glm::vec3 up(0.0F, 1.0F, 0.0F);
    glm::vec3 right(-glm::normalize(glm::cross(up, view)));
    
    //if (mCamType == eCamType::FreeMovement)
    if(KeyDown(Key::LAlt))
        SetTarget(obj->mTransform.mPosition);
    else
        SetTarget(mPosition + (view * 70.0F));
   // else
    //    

    mW2Cam = mat;

    ////setting the right vector
    //mW2Cam[0][0] = right.x;
    //mW2Cam[1][0] = right.y;
    //mW2Cam[2][0] = right.z;
    //mW2Cam[3][0] = -glm::dot(right, mPosition);
    //
    ////setting the up vector
    //mW2Cam[0][1] = up.x;
    //mW2Cam[1][1] = up.y;
    //mW2Cam[2][1] = up.z;
    //mW2Cam[3][1] = -glm::dot(up, mPosition);
}

//void EditorCamera::Update()
//{
//    mW2Proj = mCam2Proj * mW2Cam;
//}

void EditorCamera::Reset()
{
    mPosition = glm::vec3{ 0.0f };
    mTarget = glm::vec3{ 1.0f, 0.0f, 0.0f };
}


void EditorCamera::SetSphereTarget(glm::vec3 targ)
{
    GFX::Camera::SetTarget(targ);
    mSpherePos = targ;

    glm::vec3 view = mSpherePos - mPosition;
    glm::vec3 right = glm::normalize(glm::cross(view, glm::vec3(0, 1, 0)));
    mUpVec = glm::normalize(glm::cross(right, view));

    mSphereRad = glm::length(mSpherePos - mPosition);
}

void EditorCamera::SetMode(eCamType _type, const glm::vec3& target)
{
    mCamType = _type;

    if (_type == eCamType::Spherical)
        SetSphereTarget(target);

    if (_type != eCamType::Spherical)
    {
        glm::vec3 view = mTarget - mPosition;
        mUpVec = { 0.0f, 1.0f, 0.0f };
        glm::vec3 right = glm::normalize(glm::cross(view, glm::vec3(0, 1, 0)));
    }
}

void EditorCamera::Move()
{
    switch (mCamType)
    {
    case EditorCamera::eCamType::Regular:
        Regular();
        break;
    case EditorCamera::eCamType::FreeMovement:
        Free();
        break;
    case EditorCamera::eCamType::Spherical:
        Spherical();
        break;
    default:
        Regular();
        break;
    }

    mW2Cam = glm::lookAt(mPosition, mTarget, mUpVec);

    if (KeyTriggered(Key::F7)) {
        mPosition = glm::vec3{ 0.0f };
    }
}

EditorCamera::eCamType EditorCamera::GetMode()
{
    return mCamType;
}
