#include "BoundingVolumes.h"
#include <World/Components/CameraComponent.h>

bool BoundingBox::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix)
{
	return OverlapBoxPlane(model_matrix, frustum.bottom) && OverlapBoxPlane(model_matrix, frustum.top) &&
		OverlapBoxPlane(model_matrix, frustum.near) && OverlapBoxPlane(model_matrix, frustum.far) &&
		OverlapBoxPlane(model_matrix, frustum.right) && OverlapBoxPlane(model_matrix, frustum.left);
}

OverlapResult BoundingBox::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix)
{
	constexpr glm::vec3 box_points[8] = {
		glm::vec3(1.0f,-1.0f,-1.0f), glm::vec3(1.0f,1.0f,-1.0f), glm::vec3(1.0f,-1.0f,1.0f), glm::vec3(1.0f,1.0f,1.0f),
	glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(-1.0f,1.0f,-1.0f), glm::vec3(-1.0f,-1.0f,1.0f), glm::vec3(-1.0f,1.0f,1.0f) };

	bool inside = false;
	bool outside = false;
	for (auto& point : box_points) {
		if (OverlapPointPlane(model_matrix * glm::vec4(point * (box_size / 2.0f) + box_offset, 1.0f), plane)) {
			inside = true;
		}
		else {
			outside = true;
		}
	}

	return outside ? (inside ? OverlapResult::PARTIAL_OVERLAP : OverlapResult::NO_OVERLAP) : OverlapResult::FULL_OVERLAP;
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

//Well this is a mess.
bool BoundingBox::OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum)
{
	return OverlapPointPlane(point, frustum.bottom) && OverlapPointPlane(point, frustum.top) &&
		OverlapPointPlane(point, frustum.near) && OverlapPointPlane(point, frustum.far) &&
		OverlapPointPlane(point, frustum.right) && OverlapPointPlane(point, frustum.left);

}

bool BoundingBox::OverlapBoxPlane(const glm::mat4& transform, const Plane& plane)
{
	constexpr glm::vec3 box_points[8] = {
		glm::vec3(1.0f,-1.0f,-1.0f), glm::vec3(1.0f,1.0f,-1.0f), glm::vec3(1.0f,-1.0f,1.0f), glm::vec3(1.0f,1.0f,1.0f),
	glm::vec3(-1.0f,-1.0f,-1.0f), glm::vec3(-1.0f,1.0f,-1.0f), glm::vec3(-1.0f,-1.0f,1.0f), glm::vec3(-1.0f,1.0f,1.0f) };

	bool inside = false;
	bool outside = false;
	for (auto& point : box_points) {
		if (OverlapPointPlane(transform * glm::vec4(point * (box_size / 2.0f) + box_offset, 1.0f),plane)) {
			inside = true;
		}
		else {
			outside = true;
		}
	}

	return outside ? (inside ? true : false) : true;
}

bool BoundingSphere::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix)
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float max = std::max(len_x, std::max(len_y, len_z));

	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset,1.0);

	return OverlapSpherePlane(pos, max, frustum.bottom) && OverlapSpherePlane(pos, max, frustum.top) &&
		OverlapSpherePlane(pos, max, frustum.near) && OverlapSpherePlane(pos, max, frustum.far) &&
		OverlapSpherePlane(pos, max, frustum.right) && OverlapSpherePlane(pos, max, frustum.left);
}

OverlapResult BoundingSphere::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix)
{
	glm::mat3 matrix = glm::mat3(model_matrix);
	float len_x = glm::length(matrix[0]);
	float len_y = glm::length(matrix[1]);
	float len_z = glm::length(matrix[2]);
	float max = std::max(len_x, std::max(len_y, len_z));

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

bool BoundingSphere::OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane)
{
	return (glm::dot(position, plane.normal) - plane.distance) >= -radius;
}

OverlapResult BoundingInfinity::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix)
{
	return OverlapResult::PARTIAL_OVERLAP;
}

bool BoundingInfinity::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) {
	return true;
}

bool BoundingPointLightSphere::OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix)
{
	glm::vec3 pos = model_matrix * glm::vec4(sphere_offset, 1.0);

	return OverlapSpherePlane(pos, sphere_size, frustum.bottom) && OverlapSpherePlane(pos, sphere_size, frustum.top) &&
		OverlapSpherePlane(pos, sphere_size, frustum.near) && OverlapSpherePlane(pos, sphere_size, frustum.far) &&
		OverlapSpherePlane(pos, sphere_size, frustum.right) && OverlapSpherePlane(pos, sphere_size, frustum.left);
}

OverlapResult BoundingPointLightSphere::OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix)
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

bool BoundingPointLightSphere::OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane)
{
	return (glm::dot(position, plane.normal) - plane.distance) >= -radius;
}
