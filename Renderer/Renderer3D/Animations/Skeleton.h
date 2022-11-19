#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <cstdint>

struct Bone {
	uint16_t parent_index = -1;
	std::string name = "Unknown";
	glm::mat4 offset_matrix = glm::mat4(1.0f);
};

class Skeleton {
public:
	struct bone_hashmap_entry {
		uint16_t array_entry = -1;
		uint16_t animation_file_entry = -1;
	};


	Skeleton() : parent_bone_array(), bone_hashmap() {}
	Skeleton(const Skeleton& other) : parent_bone_array(other.parent_bone_array), bone_hashmap(other.bone_hashmap) {}
	Skeleton& operator=(const Skeleton& other) {
		parent_bone_array = other.parent_bone_array;
		bone_hashmap = other.bone_hashmap;
		return *this;
	}

	bone_hashmap_entry GetBoneEntryByName(const std::string& name) {
		auto fnd = bone_hashmap.find(name);
		if (fnd == bone_hashmap.end()) {
			throw std::runtime_error("Bone with name " + name + " could not be found");
		}
		return fnd->second;
	}

	const Bone& GetBoneById(uint16_t id) {
		if (id >= parent_bone_array.size() || id < 0) {
			throw std::runtime_error("Invalid Bone Id");
		}
		return parent_bone_array[id];
	}

	const Bone& GetBoneByName(const std::string& name) {
		uint16_t id = GetBoneEntryByName(name).array_entry;
		return GetBoneById(id);
	}

	bool BoneExists(const std::string& name) {
		auto fnd = bone_hashmap.find(name);
		if (fnd == bone_hashmap.end()) {
			return false;
		}
		return true;
	}

private:
	bone_hashmap_entry& GetBoneEntryReferenceByName(const std::string& name) {
		auto fnd = bone_hashmap.find(name);
		if (fnd == bone_hashmap.end()) {
			throw std::runtime_error("Bone with name " + name + " could not be found");
		}
		return fnd->second;
	}
	class aiNode;
	friend class MeshManager;
	using bone_array_type = typename std::template vector<Bone>;
	using bone_hasmap_type = typename std::template unordered_map<std::string, bone_hashmap_entry>;
	
	bone_array_type parent_bone_array;
	bone_hasmap_type bone_hashmap;

};