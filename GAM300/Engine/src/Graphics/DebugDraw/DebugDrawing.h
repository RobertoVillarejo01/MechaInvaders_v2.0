#pragma once

#include "Utilities/Singleton.h"
#include "Geometry/Geometry.h"
#include "Graphics/Mesh/Mesh.h"
#include "Graphics/Camera/Camera.h"

namespace Debug
{
  class DebugRenderer
  {
    MAKE_SINGLETON(DebugRenderer)

  public:
    // Debug drawing delayed and with a certain camera
    struct Point {
      glm::vec3 pos;
      float point_size;
    };
    struct Segment {
      geometry::segment s;
      float width;
    };

    // General control over the debug drawing
    void Clear();
    void ClearBuffers();
    void SetCamera(GFX::Camera* _cam);
    void Render(); // Render all the stored debug shapes and clear buffers

    // Actual implementations
    void DirectDrawPoint(Point const& p, glm::vec4 const& c);
    void DirectDrawLine(Segment const& seg, glm::vec4 const& c);
    void DirectDrawAABB(geometry::aabb const& aabb, glm::vec4 const& c);
    void DirectDrawOBB(geometry::obb const& obb, glm::vec4 const& c);
    void DirectDrawSphere(geometry::sphere const& sph, glm::vec4 const& c);
    void DirectDrawCylinder(geometry::cylinder const& cyl, glm::vec4 const& c);
    void DirectDrawPlane(geometry::plane const& plane, glm::vec4 const& c);

    std::vector<std::pair<Point, glm::vec4>>    mPointBuffer;
    std::vector<std::pair<Segment, glm::vec4>>  mSegmentBuffer;
    std::vector<std::pair<geometry::aabb, glm::vec4>>     mAABBBuffer;
    std::vector<std::pair<geometry::obb, glm::vec4>>      mOBBBuffer;
    std::vector<std::pair<geometry::sphere, glm::vec4>>   mSphereBuffer;
    std::vector<std::pair<geometry::cylinder, glm::vec4>> mCylinderBuffer;
    std::vector<std::pair<geometry::plane, glm::vec4>>    mPlaneBuffer;

    GFX::Camera* mCamera = nullptr;
  };

	void DrawPoint(glm::vec3 pos, glm::vec4 color, float point_size = 1.0f);
	void DrawLine(const geometry::segment& s, glm::vec4 color, float thickness = 1.0f);
	void DrawAABB(const geometry::aabb& a, glm::vec4 color);
	void DrawSphere(const geometry::sphere& s, glm::vec4 color);
	void DrawCylinder(const geometry::cylinder& s, glm::vec4 color);
	void DrawPlane(const geometry::plane& p, glm::vec4 color);
	void DrawOBB(const geometry::obb& o, glm::vec4 color);
  //void DrawTriangle(const geometry::triangle& t, glm::vec4 color);
	//void DrawFrustum(const frustum& s, glm::vec4 color);
}
