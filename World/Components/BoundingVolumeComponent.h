#pragma once 
#include <Core/BoundingVolumes.h>
#include <variant>
#include <utility>
#include <type_traits>
#include <Core/RuntimeTag.h>
#include <World/World.h>

struct NullBoundingVolume : public BoundingVolume {
public:
	NullBoundingVolume() = default;
	NullBoundingVolume(const NullBoundingVolume& other) {};
	NullBoundingVolume& operator=(const NullBoundingVolume& other) { return *this; }

	virtual bool OverlapsFrustum(const Frustum& frustum, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual OverlapResult OverlapsPlane(const Plane& plane, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool OverlapsBox(const BoundingBox& box, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool OverlapsOrientedBox(const OrientedBoundingBox& box, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool OverlapsSphere(const BoundingSphere& sphere, const glm::mat4& model_matrix) const { throw std::runtime_error("This bounding volume type should not be used directly");};
	virtual bool IntersectRay(const Ray& ray, const glm::mat4& model_matrix, std::vector<glm::vec3>& hit_results) const { throw std::runtime_error("This bounding volume type should not be used directly"); };
};



class BoundingVolumeComponent {
	RUNTIME_TAG("BoundingVolumeComponent")
public:
	using bounding_volume_variant_type = std::variant<BoundingBox, BoundingSphere, BoundingInfinity, BoundingPointLightSphere, NullBoundingVolume>;
public:
	
	BoundingVolumeComponent(const BoundingVolumeComponent& other) : bounding_volume_variant(other.bounding_volume_variant) {}

	template<typename Volume, typename dummy = std::enable_if_t<!std::is_same_v<BoundingVolumeComponent, std::decay_t<Volume>>>>
	BoundingVolumeComponent(Volume&& volume) : bounding_volume_variant(std::forward<Volume>(volume)) {}


	static bounding_volume_variant_type GetBoundingVolume(Entity ent);

private:

	bounding_volume_variant_type bounding_volume_variant;

};

template<>
class ComponentInitProxy<BoundingVolumeComponent> {
public:
	static constexpr bool can_copy = true;

};