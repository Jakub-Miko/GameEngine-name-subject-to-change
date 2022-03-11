#pragma once 
#include <glm/glm.hpp>
#include <World/Components/CameraComponent.h>
#include <Core/Geometry.h>

enum class BoundingVolumeType : unsigned char {
	NONE = 0, BOUNDING_BOX = 1, BOUNDING_SPHERE = 2
};

enum class OverlapResult : unsigned char {
	NO_OVERLAP = 0, PARTIAL_OVERLAP = 1, FULL_OVERLAP = 2
};

class BoundingVolume {
public:

	virtual OverlapResult OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) = 0;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) = 0;

};

class BoundingBox : public BoundingVolume {
public:
	BoundingBox(glm::vec3 box_size, glm::vec3 box_offset) : box_size(box_size), box_offset(box_offset) {}

	virtual OverlapResult OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) override;

private:

	bool OverlapPointPlane(const glm::vec3& point, const Plane& plane);
	bool OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum);

private:

	glm::vec3 box_size;
	glm::vec3 box_offset;

};