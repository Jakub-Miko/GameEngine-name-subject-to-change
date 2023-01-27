#include "Animation.h"
#include <Renderer/RenderResourceManager.h>
#include <Renderer/Renderer3D/Animations/AnimationManager.h>
#include <FrameManager.h>

glm::mat4 AnimationPlayback::blend_matricies(const glm::mat4& mat1,const glm::mat4& mat2, float blend_factor)
{
	glm::vec3 scale_1 = { glm::length(mat1[0]),glm::length(mat1[1]),glm::length(mat1[2]) };
	glm::vec3 scale_2 = { glm::length(mat2[0]),glm::length(mat2[1]),glm::length(mat2[2]) };
	glm::vec3 scale_3 = glm::mix(scale_1, scale_2, blend_factor);

	glm::quat rot_1 = glm::toQuat(glm::scale(mat1, glm::vec3(1.0f) / scale_1));
	glm::quat rot_2 = glm::toQuat(glm::scale(mat2, glm::vec3(1.0f) / scale_2));
	glm::quat rot_3 = glm::slerp(rot_1, rot_2, blend_factor);

	glm::vec3 trans_1 = mat1[3];
	glm::vec3 trans_2 = mat2[3];
	glm::vec3 trans_3 = glm::mix(trans_1, trans_2, blend_factor);

	return glm::translate(glm::mat4(1.0f), trans_3) * glm::toMat4(rot_3) * glm::scale(glm::mat4(1.0f), scale_3);
}

std::vector<glm::mat4> AnimationPlayback::GetBoneTransforms(std::shared_ptr<Mesh> skeletal_mesh) {
	const Skeleton& skel = skeletal_mesh->GetSkeleton();
	for (auto& layer : playback_layers) {
		if (layer.anim->GetAnimationStatus() != Animation::animation_status::READY) continue;
		if (layer.playback_state.bone_playback_states.size() != layer.anim->bone_anim.size()) throw std::runtime_error("Invalid playback state: it doesn't match number or entries with the number of skeleton bones.");
		if (skel.parent_bone_array.size() != layer.anim->bone_anim.size()) throw std::runtime_error("Number of bones of skeletal mesh doesn't match that of the animation.");
	}
	if (!skeletal_mesh->IsSkeletal()) throw std::runtime_error("Invalid Skeletal Mesh.");
	std::vector<glm::mat4> bone_transforms;
	for (int x = 0; x < skel.parent_bone_array.size();x++) {
		bone_transforms.push_back(glm::mat4(1.0f));
	}
	int layer_num = 0;
	for (auto& layer : playback_layers) {
		int i = 0;
		if (layer.anim->GetAnimationStatus() != Animation::animation_status::READY) continue;
		for (auto& anim : layer.anim->bone_anim) {
			bone_transforms[i] = blend_matricies(bone_transforms[i], anim.GetAnimationMatrix(layer.time, &layer.playback_state ? &layer.playback_state.bone_playback_states[i] : nullptr), layer_num == 0 ? 1.0f : layer.weight);
			i++;
		}
		layer_num++;
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

void AnimationPlayback::AddLayer(const AnimationPlaybackLayer& layer)
{
	playback_layers.push_back(layer);
}

void AnimationPlayback::RemoveLayer()
{
	//Change this in the future
	playback_layers.pop_back();
	if (playback_layers.empty()) {
		AnimationPlaybackLayer layer;
		layer.anim = AnimationManager::Get()->GetDefaultAnimation();
		layer.playback_state = AnimationPlaybackState();
		layer.weight = 1.0f;
		playback_layers.push_back(layer);
	}
}

AnimationPlayback::AnimationPlaybackLayer& AnimationPlayback::GetLayer(int index)
{
	if (index >= playback_layers.size()) {
		throw std::runtime_error("Animation Layer " + std::to_string(index) + " doesn't exist");
	}
	return playback_layers[index];
}

void AnimationPlayback::PromoteLayerToPrimary(int index)
{
	if (index >= playback_layers.size()) {
		throw std::runtime_error("Animation Layer " + std::to_string(index) + " doesn't exist");
	}
	std::swap(playback_layers[index], playback_layers.front());
}

glm::vec3 BoneAnimation::GetPosition(float time, BoneAnimationPlaybackState* next_state_hint) const {
	int begin = next_state_hint ? next_state_hint->last_position_index : 0;
	if (begin >= position_keyframes.size() - 1) {
		return position_keyframes[begin].position;
	}
	BoneAnimationPositionKeyFrame cur_pos;
	int index = begin;
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
	if (begin >= rotation_keyframes.size() - 1) {
		return rotation_keyframes[begin].rotation;
	}
	BoneAnimationRotationKeyFrame cur_rot;
	int index = begin;
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
	return glm::slerp(cur_rot.rotation, next.rotation, mix);
}

glm::vec3 BoneAnimation::GetScale(float time, BoneAnimationPlaybackState* next_state_hint) const {
	int begin = next_state_hint ? next_state_hint->last_scale_index : 0;
	if (begin >= scale_keyframes.size() - 1) {
		return scale_keyframes[begin].scale;
	}
	BoneAnimationScaleKeyFrame cur_scale;
	int index = begin;
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

AnimationPlayback::AnimationPlayback() : playback_layers()
{
	AnimationPlaybackLayer layer;
	layer.anim = AnimationManager::Get()->GetDefaultAnimation();
	layer.playback_state = AnimationPlaybackState();
	layer.weight = 1.0f;
	playback_layers.push_back(layer);
}

AnimationPlayback::AnimationPlayback(std::shared_ptr<Animation> animation) : playback_layers()
{
	AnimationPlaybackLayer layer;
	layer.anim = animation;
	layer.playback_state = AnimationPlaybackState();
	layer.weight = 1.0f;
	playback_layers.push_back(layer);
	int num_of_bones = animation->GetAnimationBoneNumber();
	playback_layers[0].playback_state.bone_playback_states.reserve(num_of_bones);
	for (int i = 0; i < num_of_bones; i++) {
		playback_layers[0].playback_state.bone_playback_states.emplace_back();
	}
}

bool AnimationPlayback::UpdateAnimation(float delta_time, RenderCommandList* list, std::shared_ptr<Mesh> skeletal_mesh)
{
	if (last_time_updated == FrameManager::Get()->GetCurrentFrameNumber()) return was_succesful;
	last_time_updated = FrameManager::Get()->GetCurrentFrameNumber();
	if (!bone_buffer) {
		RenderBufferDescriptor const_bone_desc(sizeof(glm::mat4) * max_num_of_bones + sizeof(unsigned int), RenderBufferType::UPLOAD, RenderBufferUsage::CONSTANT_BUFFER);
		bone_buffer = RenderResourceManager::Get()->CreateBuffer(const_bone_desc);
	}
	
	unsigned int value = 1;
	std::vector<glm::mat4> bone_transforms;
	for (auto& layer : playback_layers) {
		if (layer.speed_match) {
			float base_duration = playback_layers[0].anim->GetDuration();
			float current_duration = layer.anim->GetDuration();
			if (base_duration == 0.0f || current_duration == 0.0f) continue;
			float speedup_ratio = base_duration / current_duration;
			float slowdown_ratio = current_duration / base_duration;
			float blended_speedup = (1.0f - layer.weight) * 1.0f + speedup_ratio * layer.weight;
			float blended_slowdown = (1.0f - layer.weight) * slowdown_ratio + 1.0f * layer.weight;

			layer.time += blended_slowdown * delta_time * 0.001 * layer.anim->GetTicksPerSecond();
			playback_layers[0].time -= delta_time * 0.001 * playback_layers[0].anim->GetTicksPerSecond();
			playback_layers[0].time += blended_speedup * delta_time * 0.001 * layer.anim->GetTicksPerSecond();
		}
		else {
			layer.time += delta_time * 0.001 * layer.anim->GetTicksPerSecond();

		}
		if (layer.time >= layer.anim->GetDuration()) {
			layer.time = 0.0f;
			layer.playback_state.ClearState();
		}
	}
	if (skeletal_mesh->GetMeshStatus() == Mesh_status::LOADING) {
		value = 0;
		RenderResourceManager::Get()->UploadDataToBuffer(list, bone_buffer, &value, sizeof(unsigned int), 100 * sizeof(glm::mat4));
		was_succesful = false;
		return false;
	}

	auto queue = Renderer::Get()->GetCommandQueue();
	if (playback_layers.empty() || playback_layers[0].anim->IsEmpty()) {
		value = 0;
		RenderResourceManager::Get()->UploadDataToBuffer(list, bone_buffer, &value, sizeof(unsigned int) , 100 *sizeof(glm::mat4));
		was_succesful = false;
		return false;
	}
	else {
		for (auto& layer : playback_layers) {
			if (layer.anim->GetAnimationStatus() != Animation::animation_status::READY) continue;
			if (layer.anim->GetAnimationBoneNumber() != layer.playback_state.bone_playback_states.size()) {
				layer.playback_state.bone_playback_states.clear();
				int num_of_bones = layer.anim->GetAnimationBoneNumber();
				layer.playback_state.bone_playback_states.reserve(num_of_bones);
				for (int i = 0; i < num_of_bones; i++) {
					layer.playback_state.bone_playback_states.emplace_back();
				}
			}
		}
		bone_transforms = std::move(GetBoneTransforms(skeletal_mesh));
	}
	if (bone_buffer->GetBufferDescriptor().buffer_size < sizeof(glm::mat4) * bone_transforms.size() + sizeof(unsigned int)) throw std::runtime_error("Invalid Animation Buffer Size");

	RenderResourceManager::Get()->UploadDataToBuffer(list, bone_buffer, &value, sizeof(unsigned int), 100 * sizeof(glm::mat4));
	RenderResourceManager::Get()->UploadDataToBuffer(list, bone_buffer,bone_transforms.data(),sizeof(glm::mat4) * bone_transforms.size(), 0);
	was_succesful = true;
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
