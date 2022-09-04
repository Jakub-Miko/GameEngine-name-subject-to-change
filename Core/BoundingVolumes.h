#pragma once 
#include <glm/glm.hpp>
#include <Core/Geometry.h>
#include <Core/RuntimeTag.h>

struct Frustum;
class BoundingBox;
class BoundingSphere;

enum class BoundingVolumeType : unsigned char {
	NONE = 0, BOUNDING_BOX = 1, BOUNDING_SPHERE = 2
};

enum class OverlapResult : unsigned char {
	NO_OVERLAP = 0, PARTIAL_OVERLAP = 1, FULL_OVERLAP = 2
};

class BoundingVolume {
public:

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const = 0;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const = 0;
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const = 0;
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const = 0;
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const = 0;

};

class BoundingBox;

bool OverlapPointPlane(const glm::vec3& point, const Plane& plane);
bool OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum);
bool OverlapBoxBox(const BoundingBox& box_1, const BoundingBox& box_2);
bool OverlapPointBox(const glm::vec3& point, const BoundingBox& box);
bool OverlapPointOrientedBox(const glm::vec3& point, const OrientedBoundingBox& box);
bool OverlapPointSphere(const glm::vec3& point, const BoundingSphere& sphere);
OverlapResult OverlapBoxPlane(const BoundingBox& box, const Plane& plane);

class BoundingBox : public BoundingVolume {
	//For Serialization purposes
	RuntimeTag("BoundingBox")
public:
	BoundingBox(glm::vec3 box_size = glm::vec3(1.0f), glm::vec3 box_offset = glm::vec3(0.0f)) : box_min(-glm::abs(box_size/2.0f) + box_offset), box_max(glm::abs(box_size / 2.0f) + box_offset) {}

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const override;

	glm::vec3 GetBoxSize() const {
		return box_max - box_min;
	}
		
	glm::vec3 GetBoxOffset() const {
		return (box_max + box_min) / 2.0f;
	}

	BoundingBox GetAdjustedBox(const glm::mat4& matrix) const;

	glm::vec3 box_min;
	glm::vec3 box_max;
private:
	friend OverlapResult OverlapBoxPlane(const BoundingBox& box, const Plane& plane);
	friend bool OverlapBoxBox(const BoundingBox& box_1, const BoundingBox& box_2);
	bool OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum) const;
	bool OverlapBoxPlane_internal(const glm::mat4& transform, const Plane& plane) const;
};

class BoundingSphere : public BoundingVolume {
	//For Serialization purposes
	RuntimeTag("BoundingSphere")
public:
	BoundingSphere(float sphere_size = 1.0f, glm::vec3 sphere_offset = glm::vec3(0.0f)) : sphere_size(sphere_size), sphere_offset(sphere_offset) {}

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const override;

	float GetSphereSize() const {
		return sphere_size;
	}

	const glm::vec3& GetSphereOffset() const {
		return sphere_offset;
	}

private:

	bool OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane) const;

private:

	glm::vec3 sphere_offset;
	float sphere_size;

};

//Note: isnt affected by the scale parameter
class BoundingPointLightSphere : public BoundingVolume {
	//For Serialization purposes
	RuntimeTag("BoundingPointLightSphere")
public:
	BoundingPointLightSphere(float sphere_size = 1.0f, glm::vec3 sphere_offset = glm::vec3(0.0f)) : sphere_size(sphere_size), sphere_offset(sphere_offset) {}

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const override;

	float GetSphereSize() const {
		return sphere_size;
	}

	const glm::vec3& GetSphereOffset() const {
		return sphere_offset;
	}

private:

	bool OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane) const;

private:

	glm::vec3 sphere_offset;
	float sphere_size;

};

//Used to represent an infinetely large object than get loaded no matter whan(ie directional light)
class BoundingInfinity : public BoundingVolume {
	//For Serialization purposes
	RuntimeTag("BoundingInfinity")
public:
	BoundingInfinity() {}

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const override;
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const override;


};