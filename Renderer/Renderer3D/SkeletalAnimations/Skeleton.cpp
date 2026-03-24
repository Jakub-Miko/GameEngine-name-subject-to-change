#include "Skeleton.h"

Skeleton::bone_hashmap_entry Skeleton::GetBoneEntryByName(const std::string& name) const {
	auto fnd = bone_hashmap.find(name);
	if (fnd == bone_hashmap.end()) {
		throw std::runtime_error("Bone with name " + name + " could not be found");
	}
	return fnd->second;
}

const Bone& Skeleton::GetBoneById(uint16_t id) const {
	if (id >= parent_bone_array.size() || id < 0) {
		throw std::runtime_error("Invalid Bone Id");
	}
	return parent_bone_array[id];
}

const Bone& Skeleton::GetBoneByName(const std::string& name) const {
	uint16_t id = GetBoneEntryByName(name).array_entry;
	return GetBoneById(id);
}

bool Skeleton::BoneExists(const std::string& name) const {
	auto fnd = bone_hashmap.find(name);
	if (fnd == bone_hashmap.end()) {
		return false;
	}
	return true;
}

Skeleton::bone_hashmap_entry& Skeleton::GetBoneEntryReferenceByName(const std::string& name) {
	auto fnd = bone_hashmap.find(name);
	if (fnd == bone_hashmap.end()) {
		throw std::runtime_error("Bone with name " + name + " could not be found");
	}
	return fnd->second;
}
