#include "Geometry.h"
#include <array>
#include <algorithm>
#include <glm/gtx/projection.hpp>

namespace geometry
{
  plane::plane() : mPoint(glm::vec3()), mNormal(glm::vec3(0.0f, 1.0f, 0.0f)), mScale(1.0f) {}

  /**
   * stores point and normal of plane
   * @param point -  point on the plane
   * @param normal - normal of the plane
  */
  plane::plane(const glm::vec3& point, const glm::vec3& normal, float scale) : mPoint(point), mNormal(normal), mScale(scale) {}

  /**
   * Initializes origin and direction of ray
  */
  ray::ray() : mOrigin(glm::vec3()), mDirection(glm::vec3(1.0f, 0.0f, 0.0f)) {}

  /**
   * stores default min and max point of aabb
  */
  aabb::aabb() : min(glm::vec3(-0.5f)), max(glm::vec3(0.5f)) {}

  /**
   * stores min and max points of aabb
   * @param min - min point of aabb
   * @param max - max point of aabb
  */
  aabb::aabb(const glm::vec3& _min, const glm::vec3& _max) : min(_min), max(_max) {}

	/**
	 * Return the asked vertex of this triangle.
	 * @param idx - If == 0, first vertex; if == 1, second; if == 2, third
	 */
	const glm::vec3& triangle::operator[](unsigned _idx) const
	{
		// There is no need to check if idx is negative since it is unsigned
		assert(_idx < 3);

		// Return the expected vertex of the segment
		switch (_idx)
		{
		case 0:		return p1; break;
		case 1:		return p2; break;
		default:	return p3; break;
		}
	}

  /**
   * stores default center and radius for the sphere
  */
  sphere::sphere() : mCenter(glm::vec3()), mRadius(1.0f) {}

  /**
   * stores center and radius of sphere
   * @param center - center of the sphere
   * @param radius - radius of the sphere
  */
  sphere::sphere(const glm::vec3& center, float radius) : mCenter(center), mRadius(radius) {}
  
	/**
	 * From a given point, get which is the closest point of a given plane
	 * @param _point
	 * @param _plane
	 */
  Interval ProjectOnAxis(const aabb& a, const glm::vec3& axis)
  {
	  std::array<glm::vec3, 8> aabbVertices =
	  {
		  a.min,
		  {a.min.x, a.min.y, a.max.z},
		  {a.min.x, a.max.y, a.max.z},
		  {a.min.x, a.max.y, a.min.z},
		  {a.max.x, a.min.y, a.max.z},
		  {a.max.x, a.min.y, a.min.z},
		  {a.max.x, a.max.y, a.min.z},
		  a.max
	  };
	  Interval result;
	  result.max = result.min = glm::dot(aabbVertices[0], axis);
	  for (size_t i = 0; i < aabbVertices.size(); ++i)
	  {
		  float projection = glm::dot(aabbVertices[i], axis);
		  result.min = std::min(projection, result.min);
		  result.max = std::max(projection, result.max);
	  }
	  return result;
  }
  Interval ProjectOnAxis(const obb& o, const glm::vec3& axis)
  {
	  auto center = o.position;
	  auto hSize = o.halfSize;
	  auto orient = o.orientation;
	  std::array<glm::vec3, 3> obbAxis =
	  {
		  glm::vec3{orient[0][0], orient[0][1], orient[0][2]},
		  glm::vec3{orient[1][0], orient[1][1], orient[1][2]},
		  glm::vec3{orient[2][0], orient[2][1], orient[2][2]},
	  };
	 
	  std::array<glm::vec3, 8> obbVertices =
	  {
		  center + obbAxis[0] * hSize[0] + obbAxis[1] * hSize[1] + obbAxis[2] * hSize[2],
		  center - obbAxis[0] * hSize[0] + obbAxis[1] * hSize[1] + obbAxis[2] * hSize[2],
		  center + obbAxis[0] * hSize[0] - obbAxis[1] * hSize[1] + obbAxis[2] * hSize[2],
		  center + obbAxis[0] * hSize[0] + obbAxis[1] * hSize[1] - obbAxis[2] * hSize[2],
		  center - obbAxis[0] * hSize[0] - obbAxis[1] * hSize[1] - obbAxis[2] * hSize[2],
		  center + obbAxis[0] * hSize[0] - obbAxis[1] * hSize[1] - obbAxis[2] * hSize[2],
		  center - obbAxis[0] * hSize[0] + obbAxis[1] * hSize[1] - obbAxis[2] * hSize[2],
		  center - obbAxis[0] * hSize[0] - obbAxis[1] * hSize[1] + obbAxis[2] * hSize[2],
	  };

	  Interval result;
	  result.max = result.min = glm::dot(obbVertices[0], axis);
	  for (size_t i = 0; i < obbVertices.size(); ++i)
	  {
		  float projection = glm::dot(obbVertices[i], axis);
		  result.min = std::min(projection, result.min);
		  result.max = std::max(projection, result.max);
	  }
	  return result;
  }
  segment ClosestSegmentSegment(const segment& _s1, const segment& _s2)
  {
	  glm::vec3 k = _s1.p1 - _s2.p1;

	  // Vector directors of both segment's lines
	  glm::vec3 v = _s1.p2 - _s1.p1;
	  glm::vec3 w = _s2.p2 - _s2.p1;

	  // Some convenient variables
	  auto a = glm::dot(v, v);
	  auto b = glm::dot(v, w);
	  auto c = glm::dot(w, w);
	  auto d = glm::dot(v, k);
	  auto e = glm::dot(w, k);

	  // Compute t where: segment s1 = s1.p1 + vt (0 <= t <= 1)
	  float t = 0.0f, s = 0.0f, denominator = (b * b - a * c);

	  if (denominator != 0.0f)
		  t = (b * d - a * e) / denominator;

	  // Clamp it
	  t = std::clamp(t, 0.0f, 1.0f);

	  // Compute s based on t where: segment s2 = s2.p1 + ws (0 <= s <= 1)
	  if (a != 0.0f)
		  s = (-d + b * t) / a;

	  // Clamp s and recompute t based on it
	  s = std::clamp(s, 0.0f, 1.0f);
	  if (c != 0.0f)
		  t = (e + b * s) / c;
	  t = std::clamp(t, 0.0f, 1.0f);

	  // Finally, return the segment
	  return segment{ _s1.p1 + v * s, _s2.p1 + w * t };
  }
  glm::vec3 ClosestPointOBB(const obb& OBB, const glm::vec3& pt)
  {
	  glm::vec3 vector = pt - OBB.position;
	  glm::vec3 closest = OBB.position;;
	  for (int i = 0; i < 3; ++i)
	  {
		  glm::vec3 axis = OBB.orientation[i];
		  float distance = glm::dot(vector, axis);
		  if (distance > OBB.halfSize[i])
			  distance = OBB.halfSize[i];
		  else if (distance < -OBB.halfSize[i])
			  distance = -OBB.halfSize[i];
		  closest += (axis * distance);
	  }
	  return closest;
  }
  bool OverlapOnAxis(const aabb& a, const obb& o, const glm::vec3& axis)
  {
	  auto result1 = ProjectOnAxis(a, axis);
	  auto result2 = ProjectOnAxis(o, axis);
	  
	  return ((result2.min <= result1.max) && (result2.max >= result1.min));
  }
  glm::vec3 closest_point_plane(const glm::vec3& _point, const plane& _plane)
	{
		// Get a random vector from the plane to the point and project it to the normal of the plane
		// The resulting vector is the shortest one that joins the plane and point. Therefore, we can
		// just subtract it to the original point to get the closest point in the plane
		return _point - glm::proj(_point - _plane.mPoint, _plane.mNormal);
	}


	/**
	 * From two given segments, get the closest point to segment 2 in segment 1
	 * This is done following the method specified in the slides 
	 * @param _s1
	 * @param _s2
	 */
	segment closest_segment_segment(const segment& _s1, const segment& _s2)
	{
		// Vector between the initial points of both segments
		glm::vec3 k = _s1.p1 - _s2.p1;

		// Vector directors of both segment's lines
		glm::vec3 v = _s1.p2 - _s1.p1;
		glm::vec3 w = _s2.p2 - _s2.p1;

		// Some convenient variables
		auto a = glm::dot(v, v);
		auto b = glm::dot(v, w);
		auto c = glm::dot(w, w);
		auto d = glm::dot(v, k);
		auto e = glm::dot(w, k);

		// Compute t where: segment s1 = s1.p1 + vt (0 <= t <= 1)
		float t = 0.0f, s = 0.0f, denominator = ( b * b - a * c );

		if (denominator != 0.0f)
			t = ( b * d - a * e ) / denominator;

		// Clamp it
		t = glm::clamp(t, 0.0f, 1.0f);

		// Compute s based on t where: segment s2 = s2.p1 + ws (0 <= s <= 1)
		if (a != 0.0f)
			s = ( -d + b * t ) / a;

		// Clamp s and recompute t based on it
		s = glm::clamp(s, 0.0f, 1.0f);
		if (c != 0.0f)
			t = (e + b * s) / c;
		t = glm::clamp(t, 0.0f, 1.0f);

		// Finally, return the segment
		return segment{ _s1.p1 + v * s, _s2.p1 + w * t };
	}

	/**
	 * Check if a point is inside an AABB
	 * @param _point
	 * @param _aabb
	 */
	bool intersection_point_aabb(const glm::vec3& _point, const aabb& _aabb)
	{
		// If and only if for all 3 axises the point is between the min and max points of 
		// the aabb, then it is inside
		return (_aabb.min.x <= _point.x && _aabb.max.x >= _point.x &&
				    _aabb.min.y <= _point.y && _aabb.max.y >= _point.y &&
				    _aabb.min.z <= _point.z && _aabb.max.z >= _point.z);
	}

	bool intersection_point_aabb_2D(const glm::vec2& point, const glm::vec2& min, const glm::vec2& max)
	{
		return (min.x <= point.x && max.x >= point.x &&
			    min.y <= point.y && max.y >= point.y);
	}

	/**
	 * Check if a point is inside a triangle
	 * @param _a1
	 * @param _a2
	 */
	bool intersection_point_triangle(const glm::vec3& _point, const triangle& _triangle)
	{
		// Compute barycentric coordinates using Crammer's rule to solve a linear system with 
		// 4 equations and 3 unknowns (we don't use u + v + w = 1 right away, just the sustition of the
		// 3 coordinates)
		glm::vec3 v0 = _triangle.p2 - _triangle.p1;
		glm::vec3 v1 = _triangle.p3 - _triangle.p1;
		glm::vec3 v2 = _point - _triangle.p1;

		float r1 = glm::dot(v0, v0);
		float r2 = glm::dot(v0, v1);
		float r3 = glm::dot(v1, v1);
		float r4 = glm::dot(v2, v0);
		float r5 = glm::dot(v2, v1);

		// Check if the denominator is 0
		float denominator = r1 * r3 - r2 * r2;
		if (denominator == 0.0f) return false;

		// Get the actual barycentric coordinates
		float v = (r3 * r4 - r2 * r5) / denominator;
		float w = (r1 * r5 - r2 * r4) / denominator;
		float u = 1.0f - v - w;

		// If all of them are bigger than 0, since u is 1 - v - w, they must all be also smaller than 1
		return (u > -cEpsilon && v > -cEpsilon && w > -cEpsilon);
	}

	/**
	 * Check if two aabbs are intersecting
	 * @param _a1
	 * @param _a2
	 */
	bool intersection_aabb_aabb(const aabb& _a1, const aabb& _a2)
	{
		// If and only if there is a collision of the projections onto the 3 axis the aabbs collide
		return (_a1.min.x <= _a2.max.x && _a1.max.x >= _a2.min.x &&
						_a1.min.y <= _a2.max.y && _a1.max.y >= _a2.min.y &&
						_a1.min.z <= _a2.max.z && _a1.max.z >= _a2.min.z);
	}

	/**
	 * Check if a point is inside a sphere
	 * @param _point
	 * @param _sphere
	 */
	bool intersection_point_sphere(const glm::vec3& _point, const sphere& _sphere)
	{
		// If and only if for all 3 axises the point is between the min and max points of 
		// the aabb, then it is inside
		return (_sphere.mRadius * _sphere.mRadius >= glm::distance2(_sphere.mCenter, _point));
	}

	/**
	 * Check if two spheres are intersecting
	 * @param _s1
	 * @param _s2
	 */
	bool intersection_sphere_sphere(const sphere& _s1, const sphere& _s2)
	{
		// Imagine growing one of the sphere's radius as we reduce the other, until one is a point
		// Then we just need to check point containment in sphere
		return intersection_point_sphere(_s1.mCenter, { _s2.mCenter, _s1.mRadius + _s2.mRadius });
	}

	/**
	 * Get the time of intersection between a ray and a plane (-1.0f if they do 
	 * not intersect)
	 * @param _ray
	 * @param _plane
	 */
	float intersection_ray_plane(const ray& _ray, const plane& _plane)
	{
		// First of all, normalize the normal
		auto dir = glm::normalize(_ray.mDirection);
		auto normal = glm::normalize(_plane.mNormal);

		// Get the D component of the general equation of the plane
		float D = glm::dot(normal, _plane.mPoint);

		// Compute t (susbtituting the parametic values of the ray position in the plane and 
		// solving for t)
		float denominator = glm::dot(normal, dir);

		// If the ray direction and the plane normal are almost perpendicular, it is probably better
		// to say they are never touching that giving a really large t (also we avoid dividing by 0)
		if (std::abs(denominator) < cEpsilon) 
			return -1.0f;
		else
		{
			float result = (D - glm::dot(_ray.mOrigin, normal)) / (denominator);

			return std::max(-1.0f, result);
		}
	}

	/**
	 * Get the time of intersection between a ray and an aabb (-1.0f if they do
	 * not intersect)
	 * @param _ray
	 * @param _aabb
	 */
	float intersection_ray_aabb(const ray& _ray, const aabb& _aabb)
	{
		// Before anything check if the origin of the ray is inside the aabb
		if (intersection_point_aabb(_ray.mOrigin, _aabb)) return 0.0f;

		// Get the intersection time with the 6 planes surrounding the aabb 
		float t_x1 = intersection_ray_plane(_ray, { _aabb.min, {1.0f, 0.0f, 0.0f} });
		float t_x2 = intersection_ray_plane(_ray, { _aabb.max, {1.0f, 0.0f, 0.0f} });
		float t_y1 = intersection_ray_plane(_ray, { _aabb.min, {0.0f, 1.0f, 0.0f} });
		float t_y2 = intersection_ray_plane(_ray, { _aabb.max, {0.0f, 1.0f, 0.0f} });
		float t_z1 = intersection_ray_plane(_ray, { _aabb.min, {0.0f, 0.0f, 1.0f} });
		float t_z2 = intersection_ray_plane(_ray, { _aabb.max, {0.0f, 0.0f, 1.0f} });

		/* At most, there are two intersection points in the case of an aabb vs ray, going in and 
		going out. However, unless the ray has a 0 component in one of its axis, (or starts already
		inside one of them) the ray will cross all 6 planes. 
		
		In a the case of a collision, three of the planes will be crossed right before the collision
		each one parallel to a different axis of the aabb. Among those 3, the plane that gets crossed
		last is the one that initiates the collision between the ray and the aabb. Similarly, the earliest
		cross between the remaining 3 will be the exit point from the aabb.

		If one of the plane crosses from the last 3 (the group of the latter crosses in each axis) 
		happen before any of the initial 3, then there is no intersection. 
		*/

		// Get the minimum times in each axis
		float t_xmin = std::min(t_x1, t_x2);
		float t_ymin = std::min(t_y1, t_y2);
		float t_zmin = std::min(t_z1, t_z2);

		// Get the initial intersection time
		float t_in = std::max(std::max(t_xmin, t_ymin), t_zmin);

		// Get the maximum time in each axis
		float t_xmax = std::max(t_x1, t_x2);
		float t_ymax = std::max(t_y1, t_y2);
		float t_zmax = std::max(t_z1, t_z2);
		
		// If the max value in an axis is -1 it means the ray is parallel to that axis, therefore we
		// can not use it for the computations, since we are performing the min operation, the max float
		// will not affect the result
		if (t_xmax == -1.0f) t_xmax = std::numeric_limits<float>::max();
		if (t_ymax == -1.0f) t_ymax = std::numeric_limits<float>::max();
		if (t_zmax == -1.0f) t_zmax = std::numeric_limits<float>::max();

		// Get the end intersection time
		float t_out = std::min(std::min(t_xmax, t_ymax), t_zmax);

		// Check if all the axises are intersecting at the same time
		if (t_in < t_out)
		{
			// We need an extra check because sometimes the normal may be parallel to one of the axises
			// (or even two axises and hence give a false positive). To make sure if the ray intersects
			// or not we can just check with a point that should be inside (the average between in and out)
			if (std::max(std::max(t_xmax, t_ymax), t_zmax) &&
				!intersection_point_aabb(_ray.mOrigin + _ray.mDirection * (t_in + t_out) * 0.5f, _aabb))
			{
				return -1.0f;
			}

			return t_in;
		}
		else
			return -1.0f;
	}

	float intersection_ray_obb(const ray& _ray, const obb& _obb)
	{
		auto orient = _obb.orientation;
		auto hSize = _obb.halfSize;
		std::array<glm::vec3, 3> obbAxis =
		{
			glm::vec3{orient[0][0], orient[0][1], orient[0][2]},
			glm::vec3{orient[1][0], orient[1][1], orient[1][2]},
			glm::vec3{orient[2][0], orient[2][1], orient[2][2]},
		};
		//used to test each slab
		glm::vec3 vec = _obb.position - _ray.mOrigin;
		//project ray on every obb axis (angle between the direction of the ray and the axis of the OBB)
		glm::vec3 projectedDirection = {glm::dot(obbAxis[0], _ray.mDirection), 
										glm::dot(obbAxis[1], _ray.mDirection),
										glm::dot(obbAxis[2], _ray.mDirection)};
		//project vector joining ray and OBB on obb axis (distance between the origin of the ray and the center of the OBB, projected
		//onto the normal of the corresponding OBB slab)
		glm::vec3 projectedVec = {glm::dot(obbAxis[0], vec),
								  glm::dot(obbAxis[1], vec),
								  glm::dot(obbAxis[2], vec)};
		//two times (min and max) per axis
		std::array<float, 6> times = {0.0f};
		float eps = std::numeric_limits<float>::epsilon();
		for (int i = 0; i < 3; ++i)
		{
			//ray is parallel to current slab
			if (projectedDirection[i] > -eps && projectedDirection[i] < eps)
			{
				//ray origin is out of the slab we're checking against 
				if (-projectedVec[i] - hSize[i] > eps || -projectedVec[i] + hSize[i] < -eps)
					return -1.0f;
				//in case the ray is parallel to the slab but also inside said slab, we need this to avoid 
				//dividing by 0 in the next step
				projectedDirection[i] = 0.0001f;
			}
			times[2 * i] = (projectedVec[i] + hSize[i]) / projectedDirection[i];
			times[2 * i + 1] = (projectedVec[i] - hSize[i]) / projectedDirection[i];
		}
		//take the largest minimum 
		float tMin = std::max(
						std::max(
							std::min(times[0], times[1]), std::min(times[2], times[3])),
							std::min(times[4], times[5]));
		//take the smallest maximum
		float tMax = std::min(
						std::min(
							std::max(times[0], times[1]), std::max(times[2], times[3])),
							std::max(times[4], times[5]));
		if (tMax < eps || tMin > tMax)
			return -1.0f;
		if (tMin < eps) return tMax;
		return tMin;
	}

	/**
	 * Get the time of intersection between a ray and a sphere (-1.0f if they do
	 * not intersect)
	 * @param _ray
	 * @param _sphere
	 */
	float intersection_ray_sphere(const ray& _ray, const sphere& _sphere)
	{
		// Before anything check if the origin of the ray is inside the sphere
		if (intersection_point_sphere(_ray.mOrigin, _sphere)) return 0.0f;

		// Get the 3 components of the quadratic equation that needs to be solved to get t
		float a = glm::dot(_ray.mDirection, _ray.mDirection);
		float b = 2.0f * glm::dot(_ray.mDirection, _ray.mOrigin - _sphere.mCenter);
		float c = glm::dot(_ray.mOrigin - _sphere.mCenter, _ray.mOrigin - _sphere.mCenter)
			- _sphere.mRadius * _sphere.mRadius;

		// Make sure the math is good enough
		if (a == 0.0f) return -1.0f;
		float inside_sqrt = b * b - 4.0f * a * c;
		if (inside_sqrt < 0.0f) return -1.0f;

		// Get the 2 solutions
		float t1 = (-b + sqrtf(inside_sqrt)) / (2.0f * a);
		float t2 = (-b - sqrtf(inside_sqrt)) / (2.0f * a);

		// Check the solutions (we need the minimum positive)
		if      (t1 < 0.0f && t2 < 0.0f) return -1.0f;
		else if (t1 > 0.0f && t2 < 0.0f) return t1;
		else if (t1 < 0.0f && t2 > 0.0f) return t2;
		else return std::min(t1, t2);
	}

	/**
	 * Get the time of intersection between a ray and a sphere (-1.0f if they do
	 * not intersect)
	 * @param _ray
	 * @param _sphere
	 */
	float intersection_ray_triangle(const ray& _ray, const triangle& _triangle)
	{
		float t_plane = intersection_ray_plane(_ray, { _triangle.p1,
			glm::cross(_triangle.p1 - _triangle.p2, _triangle.p1 - _triangle.p3) });

		if (intersection_point_triangle(_ray.mOrigin + _ray.mDirection * t_plane, _triangle))
			return t_plane;
		else
			return -1.0f;
	}

	/**
	 * Check the position of a point relative to a plane (inside = behind the normal, outside = in 
	 * the direction of the normal, overlapping = in the plane)
	 * @param _plane
	 * @param _point
	 * @param _thickness
	 */
	classification_t classify_plane_point(const plane& _plane, const glm::vec3& _point,
		float _thickness)
	{
		// Check if the distance to the closest point of the plane is bigger than its thickness
		auto dist = glm::length2(closest_point_plane(_point, _plane) - _point);
		auto thickness = _thickness * _thickness;
		
		if (dist > thickness)
		{
			// Here it can never be = 0.0f since the distance is definitely bigger than thickness 
			if (glm::dot(_plane.mNormal, _point - _plane.mPoint) > 0.0f)
				return classification_t::outside;
			else
				return classification_t::inside;
		}
		else
			return classification_t::overlapping;
	}

	/**
	 * Check the position of a triangle relative to a plane (inside = all vertices behind the normal, 
	 * outside = all in the direction of the normal, overlapping = anything else)
	 * @param _plane
	 * @param _triangle
	 * @param _thickness
	 */
	classification_t classify_plane_triangle(const plane& _plane, 
		const triangle& _triangle, float _thickness)
	{
		// Distance from each point of the triangle to the plane
		auto rel_pos_0 = classify_plane_point(_plane, _triangle[0], _thickness);
		auto rel_pos_1 = classify_plane_point(_plane, _triangle[1], _thickness);
		auto rel_pos_2 = classify_plane_point(_plane, _triangle[2], _thickness);

		// If all of them are overlapping, then it is overlapping
		if (rel_pos_0 == rel_pos_1 && rel_pos_1 == rel_pos_2 && rel_pos_2 == classification_t::overlapping)
			return classification_t::overlapping;

		// Once we have discarded the case of all overlapping, if non of the vertices is outside at 
		// least one must be inside and ther may be some overlapping, all of these cases meaning that
		// the triangle is insed
		else if (rel_pos_0 != classification_t::outside && rel_pos_1 != classification_t::outside &&
			       rel_pos_2 != classification_t::outside )
		{
			return classification_t::inside;
		}

		// Same logic as above here, but for outside triangles
		else if (rel_pos_0 != classification_t::inside && rel_pos_1 != classification_t::inside &&
						 rel_pos_2 != classification_t::inside)
		{
			return classification_t::outside;
		}

		// Any of the other cases (which necessarily include one vertex being inside and one outside)
		// are evaluated here and are all overlapping triangles
		else return classification_t::overlapping;
	}

	/**
	 * Check the position of an aabb relative to a plane (inside = all vertices behind the normal,
	 * outside = all in the direction of the normal, overlapping = anything else)
	 * @param _plane
	 * @param _aabb
	 * @param _thickness
	 */
	classification_t classify_plane_aabb(const plane& _plane, const aabb& _aabb, 
		float _thickness)
	{
		classification_t rel_pos[8] = {
			classify_plane_point(_plane, _aabb.min , _thickness),
			classify_plane_point(_plane, {_aabb.min.x, _aabb.min.y, _aabb.max.z }, _thickness),
			classify_plane_point(_plane, {_aabb.min.x, _aabb.max.y, _aabb.min.z }, _thickness),
			classify_plane_point(_plane, {_aabb.min.x, _aabb.max.y, _aabb.max.z }, _thickness),
			classify_plane_point(_plane, {_aabb.max.x, _aabb.min.y, _aabb.min.z }, _thickness),
			classify_plane_point(_plane, {_aabb.max.x, _aabb.min.y, _aabb.max.z }, _thickness),
			classify_plane_point(_plane, {_aabb.max.x, _aabb.max.y, _aabb.min.z }, _thickness),
			classify_plane_point(_plane, _aabb.max, _thickness)
		};

		// Check if all the vertices are overlapping (thick plane and small aabb)
		unsigned i;
		for (i = 0; i < 8; ++i) {
			if (rel_pos[i] != classification_t::overlapping)	break;
		}

		// ... and if the loop breaks early, i < 8
		if (i == 8) return classification_t::overlapping;



		// Check if all of them are inside (if one is not inside the loop is going to break early)
		for (i = 0; i < 8; ++i) {
			if (rel_pos[i] == classification_t::outside)	break;
		}
		if (i == 8) return classification_t::inside;



		// Same logic here, checking if all vertices them are outside
		for (i = 0; i < 8; ++i) {
			if (rel_pos[i] == classification_t::inside)	break;
		}
		if (i == 8) return classification_t::outside;


		// If non of this conditions are met, it means there are some inside and some outside
		return classification_t::overlapping;
	}

	/**
	 * Check the position of a sphere relative to a plane (inside = sphere behind the normal,
	 * outside = in the direction of the normal, overlapping = anything else)
	 * @param _plane
	 * @param _sphere
	 * @param _thickness
	 */
	classification_t classify_plane_sphere(const plane& _plane, 
		const sphere& _sphere, float _thickness)
	{
		// We can imagine that is the same situation as a point and a plane (the plane's thickness
		// being increased by the radius of the sphere)
		return classify_plane_point(_plane, _sphere.mCenter, _thickness + _sphere.mRadius);
	}

	/**
	 * Check the position of a sphere relative to a frustum (inside = inside all planes of frustum,
	 * outside = outside any of the planes, overlapping = not outside and overlapping at least a plane)
	 * @param _frustum
 	 * @param _sphere
	 */
	classification_t classify_frustum_sphere_naive(const frustum _frustum, 
		const sphere _sphere)
	{
		// Check against each of the 6 planes
		classification_t rel_pos[6] = {
			classify_plane_sphere(_frustum.left, _sphere, cEpsilon),
			classify_plane_sphere(_frustum.right, _sphere, cEpsilon),
			classify_plane_sphere(_frustum.bottom, _sphere, cEpsilon),
			classify_plane_sphere(_frustum.top, _sphere, cEpsilon),
			classify_plane_sphere(_frustum.near, _sphere, cEpsilon),
			classify_plane_sphere(_frustum.far, _sphere, cEpsilon)
		};

		// If the sphere is outside any of them, it is automatically outside
		for (unsigned i = 0; i < 6; ++i) {
			if (rel_pos[i] == classification_t::outside) return classification_t::outside;
		}

		// Check if the sphere is overlapping any of the planes (but is not outside any of them)
		// the sphere itself is overlapping
		for (unsigned i = 0; i < 6; ++i) {
			if (rel_pos[i] == classification_t::overlapping) return classification_t::overlapping;
		}

		// Anything else is inside (which is basically sphere inside all planes)
		return classification_t::inside;
	}

	/**
	 * Check the position of an aabb relative to a frustum (inside = inside all planes of frustum,
	 * outside = outside any of the planes, overlapping = not outside and overlapping at least a plane)
	 * @param _frustum
	 * @param _aabb
	 */
	classification_t classify_frustum_aabb_naive(const frustum _frustum, 
		const aabb _aabb)
	{
		// Check against each of the 6 planes
		classification_t rel_pos[6] = {
			classify_plane_aabb(_frustum.left, _aabb, cEpsilon),
			classify_plane_aabb(_frustum.right, _aabb, cEpsilon),
			classify_plane_aabb(_frustum.bottom, _aabb, cEpsilon),
			classify_plane_aabb(_frustum.top, _aabb, cEpsilon),
			classify_plane_aabb(_frustum.near, _aabb, cEpsilon),
			classify_plane_aabb(_frustum.far, _aabb, cEpsilon)
		};

		// If the sphere is outside any of them, it is automatically outside
		for (unsigned i = 0; i < 6; ++i) {
			if (rel_pos[i] == classification_t::outside) return classification_t::outside;
		}

		// Check if the sphere is overlapping any of the planes (but is not outside any of them)
		// the sphere itself is overlapping
		for (unsigned i = 0; i < 6; ++i) {
			if (rel_pos[i] == classification_t::overlapping) return classification_t::overlapping;
		}

		// Anything else is inside (which is basically sphere inside all planes)
		return classification_t::inside;
	}

	cylinder::cylinder(float _height, float _radius, const glm::vec3& _center) : mHeight(_height), mRadius(_radius), mCenter(_center)
	{
	}
	obb::obb()
	{
		position = {};
		halfSize = glm::vec3(0.5f);
		orientation = glm::identity<glm::mat4>();
	}
	obb::obb(const glm::vec3& _pos, const glm::vec3& _halfSize, const glm::mat4& _orientation) : position(_pos),
																								 halfSize(_halfSize),
																								 orientation(_orientation)
	{
	}
	capsule::capsule()
	{
		radius = 1.0f;
		height = 1.0f;
		position = {};
	}
	capsule::capsule(float _radius, float _height, const glm::vec3& _position) : radius(_radius), height(_height), position(_position)
	{
	}
}