#pragma once 
#include <Core/BoundingVolumes.h>
#include <variant>
#include <utility>
#include <type_traits>
#include <Core/RuntimeTag.h>

class BoundingVolumeComponent {
	RuntimeTag("BoundingVolumeComponent")
public:
	using bounding_volume_variant_type = std::variant<BoundingBox>;
public:
	
	template<typename Volume>
	BoundingVolumeComponent(Volume&& volume) : bounding_volume_variant(std::forward<Volume>(volume)) {}

public:

	bounding_volume_variant_type bounding_volume_variant;

};