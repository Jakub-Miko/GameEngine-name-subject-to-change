#pragma once 
#include <Renderer/MeshManager.h>
#include "Skeleton.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Renderer/RenderResource.h>
#ifndef MAX_NUM_OF_BONES
#define MAX_NUM_OF_BONES 80
#endif

static const int max_num_of_bones = MAX_NUM_OF_BONES;

class RenderCommandList;

struct BoneAnimationPositionKeyFrame {
	glm::vec3 position = glm::vec3(0.0f);
	double time_stamp = 0.0f;
};

struct BoneAnimationRotationKeyFrame {
	glm::quat rotation = glm::quat(0.0f,0.0f,0.0f,0.0f);
	double time_stamp = 0.0f;
};

struct BoneAnimationScaleKeyFrame {
	glm::vec3 scale = glm::vec3(1.0f);
	double time_stamp = 0.0f;
};


struct BoneAnimationPlaybackState {
	int last_position_index;
	int last_rotation_index;
	int last_scale_index;
};

struct AnimationPlaybackState {
	void ClearState();
	std::vector<BoneAnimationPlaybackState> bone_playback_states;
};

struct BoneAnimation {

	BoneAnimation() :position_keyframes(), rotation_keyframes(), scale_keyframes(){
		position_keyframes.emplace_back();
		rotation_keyframes.emplace_back();
		scale_keyframes.emplace_back();
	}

	std::vector<BoneAnimationPositionKeyFrame> position_keyframes;
	std::vector<BoneAnimationRotationKeyFrame> rotation_keyframes;
	std::vector<BoneAnimationScaleKeyFrame> scale_keyframes;

	glm::vec3 GetPosition(float time, BoneAnimationPlaybackState* next_state_hint = nullptr) const;

	glm::quat GetRotation(float time, BoneAnimationPlaybackState* next_state_hint = nullptr) const;

	glm::vec3 GetScale(float time, BoneAnimationPlaybackState* next_state_hint = nullptr) const;

	glm::mat4 GetAnimationMatrix(float time, BoneAnimationPlaybackState* next_state_hint = nullptr) const;

};

class Animation {
public:
	enum class animation_status : char {
		UNINITIALIZED = 0, LOADING = 1, READY = 2, ERROR = 3
	};

	std::vector<glm::mat4> GetBoneTransforms(std::shared_ptr<Mesh> skeletal_mesh, float time, AnimationPlaybackState* playback_state);
	bool IsEmpty() const {
		return bone_anim.empty();
	}
	int GetAnimationBoneNumber() const {
		return bone_anim.size();
	}
	float GetDuration() const {
		return duration;
	}

	animation_status GetAnimationStatus() const {
		return status;
	}

	int GetTicksPerSecond() const {
		return ticks_per_second;
	}


private:
	friend class AnimationManager;
	float duration;
	int ticks_per_second;
	animation_status status = animation_status::UNINITIALIZED;
	std::vector<BoneAnimation> bone_anim;
};

class AnimationPlayback {
public:
	AnimationPlayback();
	AnimationPlayback(std::shared_ptr<Animation> animation);
	//returns false if animation was not loaded yet

	Animation::animation_status GetAnimationStatus() const {
		return anim->GetAnimationStatus();
	}

	void SetTime(float time) {
		current_time = time;
		playback_state.ClearState();
	}

	bool IsValidAnim() const {
		return !anim->IsEmpty();
	}

	std::shared_ptr<RenderBufferResource> GetBoneBuffer() const {
		return bone_buffer;
	}

	bool UpdateAnimation(float delta_time, RenderCommandList* list, std::shared_ptr<Mesh> skeletal_mesh);
private:
	float current_time = 0.0f;
	uint32_t last_time_updated = 0;
	bool was_succesful = false;
	std::shared_ptr<Animation> anim;
	std::shared_ptr<RenderBufferResource> bone_buffer = nullptr;
	AnimationPlaybackState playback_state;
};