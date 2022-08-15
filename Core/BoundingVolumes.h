#pragma once 
#include <glm/glm.hpp>
#include <Core/Geometry.h>
#include <Core/RuntimeTag.h>

struct Frustum;

enum class BoundingVolumeType : unsigned char {
	NONE = 0, BOUNDING_BOX = 1, BOUNDING_SPHERE = 2
};

enum class OverlapResult : unsigned char {
	NO_OVERLAP = 0, PARTIAL_OVERLAP = 1, FULL_OVERLAP = 2
};

class BoundingVolume {
public:

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) = 0;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) = 0;

};

bool OverlapPointPlane(const glm::vec3& point, const Plane& plane);
bool OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum);

class BoundingBox : public BoundingVolume {
	//For Serialization purposes
	RuntimeTag("BoundingBox")
public:
	BoundingBox(glm::vec3 box_size = glm::vec3(1.0f), glm::vec3 box_offset = glm::vec3(0.0f)) : box_size(box_size), box_offset(box_offset) {}

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) override;

	const glm::vec3& GetBoxSize() const {
		return box_size;
	}
		
	const glm::vec3& GetBoxOffset() const {
		return box_offset;
	}

private:

	bool OverlapPointFrustum(const glm::vec3& point, const Frustum& frustum);
	bool OverlapBoxPlane(const glm::mat4& transform, const Plane& plane);

private:

	glm::vec3 box_size;
	glm::vec3 box_offset;

};

class BoundingSphere : public BoundingVolume {
	//For Serialization purposes
	RuntimeTag("BoundingSphere")
public:
	BoundingSphere(float sphere_size = 1.0f, glm::vec3 sphere_offset = glm::vec3(0.0f)) : sphere_size(sphere_size), sphere_offset(sphere_offset) {}

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) override;

	float GetSphereSize() const {
		return sphere_size;
	}

	const glm::vec3& GetSphereOffset() const {
		return sphere_offset;
	}

private:

	bool OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane);

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

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) override;

	float GetSphereSize() const {
		return sphere_size;
	}

	const glm::vec3& GetSphereOffset() const {
		return sphere_offset;
	}

private:

	bool OverlapSpherePlane(const glm::vec3& position, float radius, const Plane& plane);

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

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) override;
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) override;


};