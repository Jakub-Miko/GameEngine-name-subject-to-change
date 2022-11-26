#pragma once 
#include <Renderer/MeshManager.h>
#include "Skeleton.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Renderer/RenderResource.h>

struct BoneAnimationPositionKeyFrame {
	glm::vec3 position;
	double time_stamp;
};

struct BoneAnimationRotationKeyFrame {
	glm::quat rotation;
	double time_stamp;
};

struct BoneAnimationScaleKeyFrame {
	glm::vec3 scale;
	double time_stamp;
};


struct BoneAnimationPlaybackState {
	int last_position_index;
	int last_rotation_index;
	int last_scale_index;
};

struct AnimationPlaybackState {
	std::vector<BoneAnimationPlaybackState> bone_playback_states;
};

struct BoneAnimation {
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

	std::vector<glm::mat4> GetBoneTransforms(std::shared_ptr<Mesh> skeletal_mesh, float time, AnimationPlaybackState* playback_state);
	bool IsEmpty() const {
		return bone_anim.empty();
	}
	int GetAnimationBoneNumber() const {
		return bone_anim.size();
	}
	enum class animation_status : char {
		UNINITIALIZED = 0, LOADING = 1, READY = 2, ERROR = 3
	};

private:
	friend class AnimationManager;
	float duration;
	int ticks_per_second;
	animation_status status = animation_status::UNINITIALIZED;
	std::vector<BoneAnimation> bone_anim;
};

class AnimationPlayback {
public:
	AnimationPlayback(std::shared_ptr<Animation> animation, std::shared_ptr<Mesh> skeletal_mesh);
	//returns false if animation was not loaded yet
	bool UpdateAnimation(float delta_time);
	std::shared_ptr<RenderBufferResource> GetBuffer() const {
		return animation_buffer;
	}
private:
	float current_time = 0.0f;
	std::shared_ptr<Animation> anim;
	std::shared_ptr<Mesh> skeletal_mesh;
	std::shared_ptr<RenderBufferResource> animation_buffer;
	AnimationPlaybackState playback_state;
};