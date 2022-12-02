#include "Animation.h"
#include <Renderer/RenderResourceManager.h>
#include <Renderer/Renderer3D/Animations/AnimationManager.h>

std::vector<glm::mat4> Animation::GetBoneTransforms(std::shared_ptr<Mesh> skeletal_mesh, float time, AnimationPlaybackState* playback_state) {
	if (playback_state->bone_playback_states.size() != bone_anim.size()) throw std::runtime_error("Invalid playback state: it doesn't match number or entries with the number of skeleton bones.");
	if (!skeletal_mesh->IsSkeletal()) throw std::runtime_error("Invalid Skeletal Mesh.");
	const Skeleton& skel = skeletal_mesh->GetSkeleton();
	if (skel.parent_bone_array.size() != bone_anim.size()) throw std::runtime_error("Number of bones of skeletal mesh doesn't match that of the animation.");
	std::vector<glm::mat4> bone_transforms;
	int i = 0;
	for (auto& anim : bone_anim) {
		bone_transforms.push_back(anim.GetAnimationMatrix(time, playback_state ? &playback_state->bone_playback_states[i] : nullptr));
		i++;
	}

	for (int i = 0; i < skel.parent_bone_array.size(); i++) {
		const Bone& bone = skel.parent_bone_array[i];
		if (bone.parent_index == (uint16_t)-1) {
			continue;
		}
		bone_transforms[i] = bone_transforms[bone.parent_index] * bone_transforms[i];
	}

	for (int i = 0; i < skel.parent_bone_array.size(); i++) {
		const Bone& bone = skel.parent_bone_array[i];
		bone_transforms[i] = bone_transforms[i] * bone.offset_matrix;
		//bone_transforms[i] = glm::mat4(1.0f);
	}

	return bone_transforms;
}

glm::vec3 BoneAnimation::GetPosition(float time, BoneAnimationPlaybackState* next_state_hint) const {
	int begin = next_state_hint ? next_state_hint->last_position_index : 0;
	BoneAnimationPositionKeyFrame cur_pos;
	int index = 0;
	for (int i = begin; i < position_keyframes.size() - 1; i++) {
		if (time < position_keyframes[i + 1].time_stamp) {
			cur_pos = position_keyframes[i];
			index = i;
			if (next_state_hint) {
				next_state_hint->last_position_index = i;
			}
			break;
		}
	}
	auto& next = position_keyframes[index + 1];
	float mix = (time - cur_pos.time_stamp) / (next.time_stamp - cur_pos.time_stamp);
	return glm::mix(cur_pos.position, next.position, mix);
}

glm::quat BoneAnimation::GetRotation(float time, BoneAnimationPlaybackState* next_state_hint) const {
	int begin = next_state_hint ? next_state_hint->last_rotation_index : 0;
	BoneAnimationRotationKeyFrame cur_rot;
	int index = 0;
	for (int i = begin; i < rotation_keyframes.size() - 1; i++) {
		if (time < rotation_keyframes[i + 1].time_stamp) {
			cur_rot = rotation_keyframes[i];
			index = i;
			if (next_state_hint) {
				next_state_hint->last_rotation_index = i;
			}
			break;
		}
	}
	auto& next = rotation_keyframes[index + 1];
	float mix = (time - cur_rot.time_stamp) / (next.time_stamp - cur_rot.time_stamp);
	return glm::mix(cur_rot.rotation, next.rotation, mix);
}

glm::vec3 BoneAnimation::GetScale(float time, BoneAnimationPlaybackState* next_state_hint) const {
	int begin = next_state_hint ? next_state_hint->last_scale_index : 0;
	BoneAnimationScaleKeyFrame cur_scale;
	int index = 0;
	for (int i = begin; i < scale_keyframes.size() - 1; i++) {
		if (time < scale_keyframes[i + 1].time_stamp) {
			cur_scale = scale_keyframes[i];
			index = i;
			if (next_state_hint) {
				next_state_hint->last_scale_index = i;
			}
			break;
		}
	}
	auto& next = scale_keyframes[index + 1];
	float mix = (time - cur_scale.time_stamp) / (next.time_stamp - cur_scale.time_stamp);
	return glm::mix(cur_scale.scale, next.scale, mix);
}

glm::mat4 BoneAnimation::GetAnimationMatrix(float time, BoneAnimationPlaybackState* next_state_hint) const {
	return glm::translate(glm::mat4(1.0f), GetPosition(time, next_state_hint)) * glm::toMat4(GetRotation(time, next_state_hint)) * glm::scale(glm::mat4(1.0f), GetScale(time, next_state_hint));
}

AnimationPlayback::AnimationPlayback() : anim(AnimationManager::Get()->GetDefaultAnimation())
{

}

AnimationPlayback::AnimationPlayback(std::shared_ptr<Animation> animation) : anim(animation), playback_state()
{
	int num_of_bones = anim->GetAnimationBoneNumber();
	playback_state.bone_playback_states.reserve(num_of_bones);
	for (int i = 0; i < num_of_bones; i++) {
		playback_state.bone_playback_states.emplace_back();
	}
}

bool AnimationPlayback::UpdateAnimation(float delta_time, std::shared_ptr<RenderBufferResource> animation_buffer, RenderCommandList* list, std::shared_ptr<Mesh> skeletal_mesh)
{
	std::vector<glm::mat4> bone_transforms;
	current_time += delta_time * 0.001;
	if (current_time >= anim->GetDuration()) {
		current_time = 0.0f;
		playback_state.ClearState();
	}

	auto queue = Renderer::Get()->GetCommandQueue();
	unsigned int value = 1;
	if (anim->IsEmpty()) {
		value = 0;
		RenderResourceManager::Get()->UploadDataToBuffer(list, animation_buffer, &value, sizeof(unsigned int) , 80*sizeof(glm::mat4));

		return false;
	}
	else {
		if (anim->GetAnimationBoneNumber() != playback_state.bone_playback_states.size()) {
			playback_state.bone_playback_states.clear();
			int num_of_bones = anim->GetAnimationBoneNumber();
			playback_state.bone_playback_states.reserve(num_of_bones);
			for (int i = 0; i < num_of_bones; i++) {
				playback_state.bone_playback_states.emplace_back();
			}
		}
		bone_transforms = std::move(anim->GetBoneTransforms(skeletal_mesh, current_time, &playback_state));
	}
	if (animation_buffer->GetBufferDescriptor().buffer_size < sizeof(glm::mat4) * bone_transforms.size() + sizeof(unsigned int)) throw std::runtime_error("Invalid Animation Buffer Size");

	RenderResourceManager::Get()->UploadDataToBuffer(list, animation_buffer, &value, sizeof(unsigned int), 80 * sizeof(glm::mat4));
	RenderResourceManager::Get()->UploadDataToBuffer(list,animation_buffer,bone_transforms.data(),sizeof(glm::mat4) * bone_transforms.size(), 0);

	return true;
}

void AnimationPlaybackState::ClearState()
{
	for (auto& bone : bone_playback_states) {
		bone.last_position_index = 0;
		bone.last_rotation_index = 0;
		bone.last_scale_index = 0;
	}
}
