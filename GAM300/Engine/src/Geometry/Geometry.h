#pragma once
#include "Utilities/math.h"

namespace geometry 
{
	struct ray
	{
		ray();
		ray(const glm::vec3& _origin, const glm::vec3& _direction)
			: mOrigin{ _origin }, mDirection{ _direction } {}
		glm::vec3 mOrigin;
		glm::vec3 mDirection;
	};

	struct plane
	{
		plane();
		plane(const glm::vec3& point, const glm::vec3& normal, float scale = cEpsilon);
		glm::vec3 mPoint;
		glm::vec3 mNormal;
		float mScale;
	};

	struct segment
	{
		segment() = default;
		segment(const glm::vec3& start, const glm::vec3& end) : p1{ start }, p2{ end } {}
		glm::vec3 p1 = glm::vec3{ 0.0f };
		glm::vec3 p2 = glm::vec3{ 1.0f };
	};

	struct triangle
	{
		triangle() = default;
		triangle(const glm::vec3& _p1, const glm::vec3& _p2, const glm::vec3& _p3)
			: p1{ _p1 }, p2{ _p2 }, p3{ _p3 } {}

		const glm::vec3& operator[](unsigned _idx) const;

		glm::vec3 p1{};
		glm::vec3 p2{};
		glm::vec3 p3{};
	};

	struct aabb
	{
		aabb();
		aabb(const glm::vec3& _min, const glm::vec3& _max);
		glm::vec3 min;
		glm::vec3 max;
	};

	struct capsule
	{
		capsule();
		capsule(float _radius, float _height, const glm::vec3& _position);
		float radius;
		float height;
		glm::vec3 position;
	};

	//done with game physics cookbook.
	struct Interval
	{
		float min;
		float max;
	};

	struct obb
	{
		obb();
		obb(const glm::vec3& _pos, const glm::vec3& _halfSize, const glm::mat4& _orientation);
		glm::vec3 position;
		glm::vec3 halfSize;
		//local axis
		glm::mat4 orientation = glm::identity<glm::mat4>();
	};

	struct sphere
	{
		sphere();
		sphere(const glm::vec3& center, float radius);
		glm::vec3 mCenter;
		float mRadius;
	};

	struct frustum {

		frustum() = default;
		explicit frustum(glm::mat4 _mtx) : mtx{ _mtx } {}

		plane top{};
		plane bottom{};
		plane right{};
		plane left{};
		plane near{};
		plane far{};

		glm::mat4 mtx{};
	};

	struct cylinder
	{
		cylinder() = default;
		cylinder(float _height, float _radius, const glm::vec3& _center);
		float mHeight;
		float mRadius;
		glm::vec3 mCenter;
	};

	enum class classification_t {
		inside, outside, overlapping
	};

	//these functions retrieve the interval of projecting every vertex of the aabb/obb into a given axis
	segment ClosestSegmentSegment(const segment& s1, const segment& s2);
	glm::vec3 ClosestPointOBB(const obb& OBB, const glm::vec3& pt);
	Interval ProjectOnAxis(const aabb& a, const glm::vec3& axis);
	Interval ProjectOnAxis(const obb& a, const glm::vec3& axis);

	//here, we check whether the intervals of projecting aabb and obb onto the axis are intersecting or not.
	//returns true if there is an overlap (aka no separating axis was found)
	bool OverlapOnAxis(const aabb& a, const obb& o, const glm::vec3& axis);

	glm::vec3 closest_point_plane(const glm::vec3& _point, const plane& _plane);

	bool intersection_point_aabb(const glm::vec3& _pt, const aabb& _aabb);

	float   intersection_ray_plane(const ray& _ray, const plane& _plane);
	float   intersection_ray_aabb(const ray& _ray, const aabb& _aabb);
	float   intersection_ray_obb(const ray& _ray, const obb& _obb);
	float   intersection_ray_sphere(const ray& _ray, const sphere& _sphere);
	float   intersection_ray_triangle(const ray& _ray, const triangle& _triangle);

	classification_t classify_plane_point(const plane& _plane, const glm::vec3& _point,
		float _thickness);
	classification_t classify_plane_triangle(const plane& _plane,
		const triangle& _triangle, float _thickness);
	classification_t classify_plane_aabb(const plane& _plane, const aabb& _aabb,
		float _thickness);
	classification_t classify_plane_sphere(const plane& _plane,
		const sphere& _sphere, float _thickness);
	classification_t classify_frustum_sphere_naive(const frustum _frustum,
		const sphere _sphere);
	classification_t classify_frustum_aabb_naive(const frustum _frustum,
		const aabb _aabb);
}
