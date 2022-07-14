#include "BoundingVolumes.h"


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

bool BoundingBox::OverlapPointPlane(const glm::vec3& point, const Plane& plane)
{
	return (glm::dot(point, plane.normal) - plane.distance) >= 0;
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
