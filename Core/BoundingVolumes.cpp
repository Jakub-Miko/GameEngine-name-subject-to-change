#include "BoundingVolumes.h"
#include <World/Components/CameraComponent.h>

bool BoundingBox::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const
{
	auto adjusted = GetAdjustedBox(model_matrix);
	return OverlapBoxPlane(adjusted, frustum.bottom) != OverlapResult::NO_OVERLAP &&
		OverlapBoxPlane(adjusted, frustum.top) != OverlapResult::NO_OVERLAP &&
		OverlapBoxPlane(adjusted, frustum.left) != OverlapResult::NO_OVERLAP &&
		OverlapBoxPlane(adjusted, frustum.right) != OverlapResult::NO_OVERLAP &&
		OverlapBoxPlane(adjusted, frustum.far) != OverlapResult::NO_OVERLAP &&
		OverlapBoxPlane(adjusted, frustum.near) != OverlapResult::NO_OVERLAP;
}

OverlapResult BoundingBox::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const
{
	auto adjusted = GetAdjustedBox(model_matrix);
	return OverlapBoxPlane(adjusted, plane);


	//constexpr glm::vec3 box_points[8] = {
	//	glm::vec3(1.0f,-1.0f,-1.0f), glm::vec3(1.0f,1.0f,-1.0f), glm::vec3(1.0f,-1.0f,1.0f), glm::vec3(1.0f,1.0f,1.0f),
	//glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(-1.0f,1.0f,-1.0f), glm::vec3(-1.0f,-1.0f,1.0f), glm::vec3(-1.0f,1.0f,1.0f) };

	//bool inside = false;
	//bool outside = false;
	//for (auto& point : box_points) {
	//	if (OverlapPointPlane(model_matrix * glm::vec4(point * (box_size / 2.0f) + box_offset, 1.0f), plane)) {
	//		inside = true;
	//	}
	//	else {
	//		outside = true;
	//	}
	//}

	//return outside ? (inside ? OverlapResult::PARTIAL_OVERLAP : OverlapResult::NO_OVERLAP) : OverlapResult::FULL_OVERLAP;
}

bool BoundingBox::OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const
{
	auto adjusted_box = GetAdjustedBox(model_matrix);
	return OverlapBoxBox(box, adjusted_box);

}

bool BoundingBox::OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const
{
	auto box_a = GetAdjustedBox(model_matrix);
	auto& box_b = box;

	auto box_a_center = (box_a.box_min + box_a.box_max) / 2.0f;
	auto box_b_center = box_b.center;

	auto a = (box_a.box_max - box_a.box_min) / 2.0f;
	auto b = box_b.size/2.0f;

	glm::vec3 T = box_b_center - box_a_center;
	glm::mat3 R = box_b.rotation_matrix;

	float ra, rb, t;
	long i, k;

	glm::mat3 A = glm::mat4(1.0f);
	for (i = 0; i < 3; i++)
		for (k = 0; k < 3; k++)
			R[i][k] = glm::dot(A[i],box_b.rotation_matrix[k]);
	for (i = 0; i < 3; i++)
	{
		ra = a[i];

		rb = b[0] * fabs(R[i][0]) + b[1] * fabs(R[i][1]) + b[2] * fabs(R[i][2]);

		t = fabs(T[i]);

		if (t > ra + rb)
			return false;

	}

	//B's basis vectors
	for (k = 0; k < 3; k++)
	{
		ra = a[0] * fabs(R[0][k]) + a[1] * fabs(R[1][k]) + a[2] * fabs(R[2][k]);

		rb = b[k];

		t = fabs(T[0] * R[0][k] + T[1] * R[1][k] + T[2] * R[2][k]);

		if (t > ra + rb)
			return false;
	}
	//9 cross products

//L = A0 x B0
	ra = a[1] * fabs(R[2][0]) + a[2] * fabs(R[1][0]);
	rb = b[1] * fabs(R[0][2]) + b[2] * fabs(R[0][1]);
	t = fabs(T[2] * R[1][0] - T[1] * R[2][0]);
	if (t > ra + rb)
		return false;

	//L = A0 x B1
	ra = a[1] * fabs(R[2][1]) + a[2] * fabs(R[1][1]);
	rb = b[0] * fabs(R[0][2]) + b[2] * fabs(R[0][0]);
	t = fabs(T[2] * R[1][1] - T[1] * R[2][1]);
	if (t > ra + rb)
		return false;

	//L = A0 x B2
	ra = a[1] * fabs(R[2][2]) + a[2] * fabs(R[1][2]);
	rb = b[0] * fabs(R[0][1]) + b[1] * fabs(R[0][0]);
	t = fabs(T[2] * R[1][2] - T[1] * R[2][2]);
	if (t > ra + rb)
		return false;

	//L = A1 x B0
	ra = a[0] * fabs(R[2][0]) + a[2] * fabs(R[0][0]);
	rb = b[1] * fabs(R[1][2]) + b[2] * fabs(R[1][1]);
	t = fabs(T[0] * R[2][0] - T[2] * R[0][0]);
	if (t > ra + rb)
		return false;

	//L = A1 x B1
	ra = a[0] * fabs(R[2][1]) + a[2] * fabs(R[0][1]);
	rb = b[0] * fabs(R[1][2]) + b[2] * fabs(R[1][0]);
	t = fabs(T[0] * R[2][1] - T[2] * R[0][1]);
	if (t > ra + rb)
		return false;

	//L = A1 x B2
	ra = a[0] * fabs(R[2][2]) + a[2] * fabs(R[0][2]);
	rb = b[0] * fabs(R[1][1]) + b[1] * fabs(R[1][0]);
	t = fabs(T[0] * R[2][2] - T[2] * R[0][2]);
	if (t > ra + rb)
		return false;

	//L = A2 x B0
	ra = a[0] * fabs(R[1][0]) + a[1] * fabs(R[0][0]);
	rb = b[1] * fabs(R[2][2]) + b[2] * fabs(R[2][1]);
	t = fabs(T[1] * R[0][0] - T[0] * R[1][0]);
	if (t > ra + rb)
		return false;

	//L = A2 x B1
	ra = a[0] * fabs(R[1][1]) + a[1] * fabs(R[0][1]);
	rb = b[0] * fabs(R[2][2]) + b[2] * fabs(R[2][0]);
	t = fabs(T[1] * R[0][1] - T[0] * R[1][1]);
	if (t > ra + rb)
		return false;

	//L = A2 x B2
	ra = a[0] * fabs(R[1][2]) + a[1] * fabs(R[0][2]);
	rb = b[0] * fabs(R[2][1]) + b[1] * fabs(R[2][0]);
	t = fabs(T[1] * R[0][2] - T[0] * R[1][2]);
	if (t > ra + rb)
		return false;
	/*no separating axis found,
	the two boxes overlap */

	return true;
}

bool BoundingBox::OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const
{
	auto adjusted_box = GetAdjustedBox(model_matrix);
	return sphere.OverlapsBox(adjusted_box, glm::mat4(1.0f));
}

bool OverlapPointPlane(const glm::vec3& point, const Plane& plane)
{
	return (glm::dot(point, plane.normal) - plane.distance) >= 0;
}

bool OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum)
{
	return OverlapPointPlane(point, frustum.bottom) && OverlapPointPlane(point, frustum.top) &&
		OverlapPointPlane(point, frustum.near) && OverlapPointPlane(point, frustum.far) &&
		OverlapPointPlane(point, frustum.right) && OverlapPointPlane(point, frustum.left);
}

OverlapResult OverlapBoxPlane(const BoundingBox& box, const Plane& plane)
{
	glm::vec3 center = (box.box_max + box.box_min) / 2.0f;
	glm::vec3 extents = box.box_max - center;

	float radius = extents[0] * glm::abs(plane.normal[0]) + extents[1] * glm::abs(plane.normal[1]) + extents[2] * glm::abs(plane.normal[2]);

	float distance = glm::dot(plane.normal, center) - plane.distance;

	if (glm::abs(distance) <= radius)
		return OverlapResult::PARTIAL_OVERLAP;

	return distance > radius ? OverlapResult::FULL_OVERLAP : OverlapResult::NO_OVERLAP;
}

bool OverlapBoxBox(const BoundingBox& box_1, const BoundingBox& box_2)
{
	bool partial = false;
	for (int i = 0; i < 3; i++) {
		if (!(box_1.box_min[i] <= box_2.box_max[i] && box_1.box_max[i] >= box_2.box_min[i])) {
			return false;
		}
	}
	return true;
}

bool OverlapPointBox(const glm::vec3& point, const BoundingBox& box)
{
	return (point.x >= box.box_min.x && point.x <= box.box_max.x) &&
		(point.y >= box.box_min.y && point.y <= box.box_max.y) &&
		(point.z >= box.box_min.z && point.z <= box.box_max.z);
}

bool OverlapPointOrientedBox(const glm::vec3& point, const OrientedBoundingBox& box)
{
	glm::mat4 box_matrix = glm::mat4(box.rotation_matrix) * glm::translate(glm::mat4(1.0f), box.center);
	glm::vec3 objPos = glm::inverse(box_matrix) * glm::vec4(point,1.0f);
	return
		abs(objPos.x) <= box.size.x * 0.5 &&
		abs(objPos.y) <= box.size.y * 0.5 &&
		abs(objPos.z) <= box.size.z * 0.5;
}

bool OverlapPointSphere(const glm::vec3& point, const BoundingSphere& sphere)
{
	return glm::abs(glm::length(sphere.GetSphereOffset() - point)) < sphere.GetSphereSize();
}

//Well this is a mess.
bool BoundingBox::OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum) const
{
	return OverlapPointPlane(point, frustum.bottom) && OverlapPointPlane(point, frustum.top) &&
		OverlapPointPlane(point, frustum.near) && OverlapPointPlane(point, frustum.far) &&
		OverlapPointPlane(point, frustum.right) && OverlapPointPlane(point, frustum.left);

}

bool BoundingBox::OverlapBoxPlane_internal(const glm::mat4& transform, const Plane& plane) const
{
	return OverlapsPlane(plane, transform) != OverlapResult::NO_OVERLAP;
}

BoundingBox BoundingBox::GetAdjustedBox(const glm::mat4& matrix) const
{
	glm::mat3 rotation_scale = glm::transpose(matrix);
	glm::vec3 translation = glm::vec3(matrix[3]);
	BoundingBox box_adjusted(glm::vec3(0.0f), translation);
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) {
			float a = rotation_scale[x][y] * box_min[y];
			float b = rotation_scale[x][y] * box_max[y];
			box_adjusted.box_min[x] += a < b ? a : b;
			box_adjusted.box_max[x] += a < b ? b : a;
		}
	}
	return box_adjusted;
}

bool BoundingSphere::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float max = std::max(len_x, std::max(len_y, len_z)) * sphere_size;

	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset,1.0);

	return OverlapSpherePlane(pos, max, frustum.bottom) && OverlapSpherePlane(pos, max, frustum.top) &&
		OverlapSpherePlane(pos, max, frustum.near) && OverlapSpherePlane(pos, max, frustum.far) &&
		OverlapSpherePlane(pos, max, frustum.right) && OverlapSpherePlane(pos, max, frustum.left);
}

OverlapResult BoundingSphere::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float max = std::max(len_x, std::max(len_y, len_z)) * sphere_size;

	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset, 1.0);

	float distance = (glm::dot(pos, plane.normal) - plane.distance);

	if (distance >= -max) {
		if(distance >= max) {
			return OverlapResult::FULL_OVERLAP;
		} 
		return OverlapResult::PARTIAL_OVERLAP;
	}
	return OverlapResult::NO_OVERLAP();
}

bool BoundingSphere::OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float radius = std::max(len_x, std::max(len_y, len_z)) * sphere_size;

	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset, 1.0);

	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++) {
		float v = pos[i];
		if (v < box.box_min[i]) sqDist += (box.box_min[i] - v) * (box.box_min[i] - v);
		if (v > box.box_max[i]) sqDist += (v - box.box_max[i]) * (v - box.box_max[i]);
	}

	return sqDist <= radius * radius;

}

bool BoundingSphere::OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float radius = std::max(len_x, std::max(len_y, len_z)) * sphere_size;

	glm::mat4 box_matrix = glm::mat4(box.rotation_matrix) * glm::translate(glm::mat4(1.0f), box.center);

	glm::vec3 pos = glm::inverse(box_matrix) * glm::vec4(model_matrix * glm::vec4(sphere_offset, 1.0));

	glm::vec3 box_min = (-glm::abs(box.size/2.0f));
	glm::vec3 box_max = (glm::abs(box.size/2.0f));

	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++) {
		float v = pos[i];
		if (v < box_min[i]) sqDist += (box_min[i] - v) * (box_min[i] - v);
		if (v > box_max[i]) sqDist += (v - box_max[i]) * (v - box_max[i]);
	}

	return sqDist <= radius * radius;
}

bool BoundingSphere::OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float radius = std::max(len_x, std::max(len_y, len_z)) * sphere_size;

	glm::vec3 pos =  glm::vec4(model_matrix * glm::vec4(sphere_offset, 1.0));

	return glm::abs(glm::length(pos - sphere.sphere_offset)) < radius + sphere.sphere_size;
}

bool BoundingSphere::OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane) const
{
	return (glm::dot(position, plane.normal) - plane.distance) >= -radius;
}

OverlapResult BoundingInfinity::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const
{
	return OverlapResult::PARTIAL_OVERLAP;
}

bool BoundingInfinity::OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const
{
	return true;
}

bool BoundingInfinity::OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const
{
	return true;
}

bool BoundingInfinity::OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const
{
	return true;
}

bool BoundingInfinity::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const {
	return true;
}

bool BoundingPointLightSphere::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const
{
	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset, 1.0);

	return OverlapSpherePlane(pos, sphere_size, frustum.bottom) && OverlapSpherePlane(pos, sphere_size, frustum.top) &&
		OverlapSpherePlane(pos, sphere_size, frustum.near) && OverlapSpherePlane(pos, sphere_size, frustum.far) &&
		OverlapSpherePlane(pos, sphere_size, frustum.right) && OverlapSpherePlane(pos, sphere_size, frustum.left);
}

OverlapResult BoundingPointLightSphere::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const
{
	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset, 1.0);

	float distance = (glm::dot(pos, plane.normal) - plane.distance);

	if (distance >= -sphere_size) {
		if (distance >= sphere_size) {
			return OverlapResult::FULL_OVERLAP;
		}
		return OverlapResult::PARTIAL_OVERLAP;
	}
	return OverlapResult::NO_OVERLAP();
}

bool BoundingPointLightSphere::OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const
{
	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset, 1.0);

	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++) {
		float v = pos[i];
		if (v < box.box_min[i]) sqDist += (box.box_min[i] - v) * (box.box_min[i] - v);
		if (v > box.box_max[i]) sqDist += (v - box.box_max[i]) * (v - box.box_max[i]);
	}

	return sqDist <= sphere_size * sphere_size;
}

bool BoundingPointLightSphere::OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const
{
	glm::mat4 box_matrix = glm::mat4(box.rotation_matrix) * glm::translate(glm::mat4(1.0f), box.center);
	
	glm::vec3 pos = glm::inverse(box_matrix) * glm::vec4(model_matrix * glm::vec4(sphere_offset, 1.0));

	glm::vec3 box_min = (-glm::abs(box.size / 2.0f));
	glm::vec3 box_max = (glm::abs(box.size / 2.0f));

	float sqDist = 0.0f;
	for (int i = 0; i < 3; i++) {
		float v = pos[i];
		if (v < box_min[i]) sqDist += (box_min[i] - v) * (box_min[i] - v);
		if (v > box_max[i]) sqDist += (v - box_max[i]) * (v - box_max[i]);
	}

	return sqDist <= sphere_size * sphere_size;
}

bool BoundingPointLightSphere::OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const
{
	glm::vec3 pos = glm::vec4(model_matrix * glm::vec4(sphere_offset, 1.0));

	return glm::abs(glm::length(pos - sphere.GetSphereOffset())) < sphere_size + sphere.GetSphereSize();
}

bool BoundingPointLightSphere::OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane) const
{
	return (glm::dot(position, plane.normal) - plane.distance) >= -radius;
}
