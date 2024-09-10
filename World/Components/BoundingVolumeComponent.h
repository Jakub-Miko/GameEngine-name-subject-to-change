#pragma once 
#include <Core/BoundingVolumes.h>
#include <variant>
#include <utility>
#include <type_traits>
#include <Core/RuntimeTag.h>
#include <World/World.h>

/**
 * @brief The default invalid implementation of BoundingVolume to be used as a placeholder 
 * @warning This signifies an invalid or non existing BoundingVolume and throws when a BoundingVolume functions are called with it. It is also ignored on Culling or Ray Intersection tests.
 */
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


/**
 * @brief Represents an Entities bounding volume.
 * 
 * This defines the spatial presence of all entities in the scene and is used in SpatialIndex operations such as Culling and Intersection tests.
 * 
 * All visible object are required to have a valid BoundingVolume (either through this component or the @ref BoundingVolumeComponent::GetBoundingVolume function), otherwise Frustum Culling is not going to return them, and they will not be Rendered.
 * 
 * @warning This component should not be assigned manually and is not serialized. It is assigned automatically when assigning other components such as a LightComponent. Components such as MeshComponent or SkeletalMeshComponent 
 * automatically use the bounding boxes of their meshes(see @ref BoundingVolumeComponent::GetBoundingVolume).
 */
class BoundingVolumeComponent {
	RUNTIME_TAG("BoundingVolumeComponent")
public:
	/**
	 * @brief A variant type representing all possible implementations of BoundingVolume used supported by this component
	 */
	using bounding_volume_variant_type = std::variant<BoundingBox, BoundingSphere, BoundingInfinity, BoundingPointLightSphere, NullBoundingVolume>;
public:
	
	/**
	 * @brief Copy constructor
	 * @param other BoundingVolumeComponent instance to copy 
	 */
	BoundingVolumeComponent(const BoundingVolumeComponent& other) : bounding_volume_variant(other.bounding_volume_variant) {}

	/**
	 * @brief Constructor which creates a BoundingVolumeComponent by moving an instance of a BoundingVolume implementation to this component.
	 *
	 * @warning Since move semantics are used, the passed instance is not longer guaranteed to be valid outside of this component.
	 * 
	 * @tparam Volume the type of BoundingVolume used 
	 * @tparam dummy a dummy template argument used for differentiating between a Move constructor and this constructor
	 * @param volume the BoundingVolume instance to be used.
	 */
	template<typename Volume, typename dummy = std::enable_if_t<!std::is_same_v<BoundingVolumeComponent, std::decay_t<Volume>>>>
	BoundingVolumeComponent(Volume&& volume) : bounding_volume_variant(std::forward<Volume>(volume)) {}

	/**
	 * @brief Gets the BoundingVolume of a BoundingVolumeComponent of an entity
	 * 
	 * @note This is the only way to retrieve a bounding volume of an Entity, because MeshComponents and SkeletalMeshComponents don't have a BoundingVolumeComponent and instead
	 * use the bounding volumes provided by the meshes themselves. This means that Bounding volumes of all entities using a mesh don't have to be updated when Meshes change.
	 * 
	 * @param ent Entity whose BoundingVolume to get
	 * @return The bounding_volume_variant_type of the Entities BoundingVolumeComponent
	 */
	static bounding_volume_variant_type GetBoundingVolume(Entity ent);

private:

	/**
	 * @brief The bounding_volume_variant_type variant used to store the BoundingVolume
	 * 
	 * @note A variant is used instead of a base class pointer to avoid indirection in storing of the Bounding volumes improving data locality.
	 */
	bounding_volume_variant_type bounding_volume_variant;

};

/**
 * @brief A specialization of the ComponentInitProxy specifying the properties of this component
 */
template<>
class ComponentInitProxy<BoundingVolumeComponent> {
public:
	static constexpr bool can_copy = true; ///< Declares this component as copyable

};