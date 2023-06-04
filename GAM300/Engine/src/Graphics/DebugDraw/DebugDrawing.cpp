#include "DebugDrawing.h"

#include <memory>

#include "GL/glew.h"
#include "Graphics/Graphics.h"
#include "resourcemanager/Resourcemanager.h"

using namespace GFX;

void Debug::DrawPoint(glm::vec3 pos, glm::vec4 color, float point_size)
{
  DebugRenderer::Instance().mPointBuffer.push_back({ {pos, point_size}, color });
}

void Debug::DrawLine(const geometry::segment& s, glm::vec4 color, float thickness)
{
  DebugRenderer::Instance().mSegmentBuffer.push_back({ {s, thickness}, color });
}

//void Debug::DrawTriangle(const geometry::triangle& t, glm::vec4 color)
//{
//  // Helpfull references
//  auto mesh = Mesh{ std::vector<glm::vec3>{ t.p1, t.p2, t.p3}, std::vector<unsigned>{0, 1, 2} };
//  mesh.UploadMesh();
//  
//  // Call the generic draw function
//  glCullFace(GL_FRONT);
//  RenderMgr.RenderMesh(&mesh, color * 0.5f, glm::mat4(1.0f), PolygonMode_t::Solid);
//  glCullFace(GL_BACK);
//  RenderMgr.RenderMesh(&mesh, color, glm::mat4(1.0f), PolygonMode_t::Solid);
//}

void Debug::DrawAABB(const geometry::aabb& a, glm::vec4 color)
{
  DebugRenderer::Instance().mAABBBuffer.push_back({ a, color });
}

void Debug::DrawOBB(const geometry::obb& o, glm::vec4 color)
{
  DebugRenderer::Instance().mOBBBuffer.push_back({ o, color });
}

void Debug::DrawSphere(const geometry::sphere& s, glm::vec4 color)
{
  DebugRenderer::Instance().mSphereBuffer.push_back({ s, color });
}

void Debug::DrawCylinder(const geometry::cylinder& s, glm::vec4 color)
{
  DebugRenderer::Instance().mCylinderBuffer.push_back({ s, color });
}

void Debug::DrawPlane(const geometry::plane& p, glm::vec4 color)
{
  DebugRenderer::Instance().mPlaneBuffer.push_back({ p, color });
}


void Debug::DebugRenderer::Clear()
{
	ClearBuffers();
	SetCamera(nullptr);
}

void Debug::DebugRenderer::SetCamera(Camera* _cam)
{
	mCamera = _cam;
}

void Debug::DebugRenderer::Render()
{
	// Set the camera (this is the only dangerous part about this method, the camera must 
	// be properly set to null before deleting it from the scene)
	RenderMgr.SetCamera(mCamera);

	// Render all the lines
	for (auto& p : mPointBuffer) {
		DirectDrawPoint(p.first, p.second);
	}
	for (auto& s : mSegmentBuffer) {
    DirectDrawLine(s.first, s.second);
	}
	for (auto& a : mAABBBuffer) {
    DirectDrawAABB(a.first, a.second);
	}
	for (auto& o : mOBBBuffer) {
    DirectDrawOBB(o.first, o.second);
	}
	for (auto& s : mSphereBuffer) {
    DirectDrawSphere(s.first, s.second);
	}
	for (auto& c : mCylinderBuffer) {
    DirectDrawCylinder(c.first, c.second);
	}
	for (auto& p : mPlaneBuffer) {
    DirectDrawPlane(p.first, p.second);
	}

	ClearBuffers();
}

void Debug::DebugRenderer::ClearBuffers()
{
	mPointBuffer.clear();
	mSegmentBuffer.clear();
	mAABBBuffer.clear();
	mOBBBuffer.clear();
	mSphereBuffer.clear();
	mCylinderBuffer.clear();
	mPlaneBuffer.clear();
}

void Debug::DebugRenderer::DirectDrawPoint(Point const& p, glm::vec4 const& c)
{
  glPointSize(p.point_size);
  RenderMgr.RenderMesh(&Mesh::Point, c, glm::translate(p.pos), PolygonMode_t::PointCloud);
}

void Debug::DebugRenderer::DirectDrawLine(Segment const& seg, glm::vec4 const& c)
{
  // Transformations
  auto  T = glm::translate(seg.s.p1);
  auto  S = glm::scale(seg.s.p2 - seg.s.p1);
  auto  m2w = T * S;

  glLineWidth(seg.width);
  RenderMgr.RenderMesh(&Mesh::Segment, c, m2w, PolygonMode_t::Wireframe);
}

void Debug::DebugRenderer::DirectDrawAABB(geometry::aabb const& aabb, glm::vec4 const& c)
{
  // Transformations
  auto  T = glm::translate((aabb.min + aabb.max) / 2.0f);
  auto  S = glm::scale(glm::abs(aabb.min - aabb.max));
  auto  m2w = T * S;

  // Call the generic draw function
  RenderMgr.RenderModel(&Model::Cube, c, m2w, PolygonMode_t::Wireframe);
}

void Debug::DebugRenderer::DirectDrawOBB(geometry::obb const& obb, glm::vec4 const& c)
{
  // Transformations
  auto  T = glm::translate(obb.position);
  auto  R = obb.orientation;
  auto  S = glm::scale(obb.halfSize * 2.0f);
  auto  m2w = T * R * S;

  // Call the generic draw function
  RenderMgr.RenderModel(&Model::Cube, c, m2w, PolygonMode_t::Wireframe);
}

void Debug::DebugRenderer::DirectDrawSphere(geometry::sphere const& sph, glm::vec4 const& c)
{
  // Transformations
  auto  T = glm::translate(sph.mCenter);
  auto  S = glm::scale(glm::vec3(sph.mRadius));
  auto  m2w = T * S;

  // Meshes
  auto model = ResourceMgr.GetResource<Model>("./../Resources/Meshes/sphere.obj");

  // Call the generic draw function
  RenderMgr.RenderModel(model->get(), c, m2w, PolygonMode_t::Wireframe);
}

void Debug::DebugRenderer::DirectDrawCylinder(geometry::cylinder const& cyl, glm::vec4 const& c)
{
  auto T = glm::translate(cyl.mCenter);
  auto S = glm::scale(glm::vec3(cyl.mRadius, cyl.mHeight / 2.0f, cyl.mRadius));
  auto  m2w = T * S;

  // Meshes
  auto model = ResourceMgr.GetResource<Model>("./../Resources/Meshes/cylinder.obj");

  // Call the generic draw function
  RenderMgr.RenderModel(model->get(), c, m2w, PolygonMode_t::Wireframe);
}

void Debug::DebugRenderer::DirectDrawPlane(geometry::plane const& plane, glm::vec4 const& c)
{
  // Transformations
  auto  T = glm::translate(plane.mPoint);
  auto  S = glm::scale(glm::vec3{ 50.0f });
  auto  R = glm::inverse(glm::lookAt(glm::vec3(0.0f), plane.mNormal, glm::vec3{ 0.0f, 1.0f, 0.0f }));
  auto  m2w = T * R * S;

  // Call the generic draw function (back face)
  glCullFace(GL_FRONT);
  RenderMgr.RenderModel(&Model::Quad, c * 0.5f, m2w, PolygonMode_t::Solid);

  // Call the generic draw function (front face)
  glCullFace(GL_BACK);
  RenderMgr.RenderModel(&Model::Quad, c, m2w, PolygonMode_t::Solid);

  if (plane.mScale > cEpsilon)
  {
    glLineWidth(1.0f);

    // Draw the thickness of the plane
    RenderMgr.RenderModel(&Model::Quad, c * 0.75f,
      glm::translate(plane.mNormal * plane.mScale) * m2w, PolygonMode_t::Wireframe);
    RenderMgr.RenderModel(&Model::Quad, c * 0.75f,
      glm::translate(-plane.mNormal * plane.mScale) * m2w, PolygonMode_t::Wireframe);
  }

  // Draw the normal of the plane
  DirectDrawLine({ {plane.mPoint, plane.mPoint + plane.mNormal * 50.0f * 0.25f}, 2.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f });
  DirectDrawPoint({ plane.mPoint + plane.mNormal * 50.0f * 0.25f, 5.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f });
  DirectDrawPoint({ plane.mPoint, 5.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });
}