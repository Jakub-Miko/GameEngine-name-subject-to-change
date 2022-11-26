#pragma once 
#include <Core/BoundingVolumes.h>
#include <variant>
#include <utility>
#include <type_traits>
#include <Core/RuntimeTag.h>

struct NullBoundingVolume : public BoundingVolume {
public:
	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
};

class BoundingVolumeComponent {
	RUNTIME_TAG("BoundingVolumeComponent")
public:
	using bounding_volume_variant_type = std::variant<BoundingBox, BoundingSphere, BoundingInfinity, BoundingPointLightSphere, NullBoundingVolume>;
public:
	
	template<typename Volume>
	BoundingVolumeComponent(Volume&& volume) : bounding_volume_variant(std::forward<Volume>(volume)) {}

	static bounding_volume_variant_type GetBoundingVolume(Entity ent);

private:

	bounding_volume_variant_type bounding_volume_variant;

};