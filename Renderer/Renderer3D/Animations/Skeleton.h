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

	int GetNumberOfBones() const {
		return parent_bone_array.size();
	}

	bone_hashmap_entry GetBoneEntryByName(const std::string& name) const;

	const Bone& GetBoneById(uint16_t id) const;

	const Bone& GetBoneByName(const std::string& name) const;

	bool BoneExists(const std::string& name) const;

private:
	bone_hashmap_entry& GetBoneEntryReferenceByName(const std::string& name);
	class aiNode;
	friend class MeshManager;
	friend class Animation;
	friend class AnimationManager;
	friend class AnimationPlayback;
	using bone_array_type = typename std::template vector<Bone>;
	using bone_hasmap_type = typename std::template unordered_map<std::string, bone_hashmap_entry>;
	
	bone_array_type parent_bone_array;
	bone_hasmap_type bone_hashmap;

};