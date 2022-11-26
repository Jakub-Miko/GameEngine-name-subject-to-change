#include "AnimationManager.h"
#include <assimp/scene.h>
#include <fstream>
#include <FileManager.h>
#include <Application.h>

AnimationManager* AnimationManager::instance = nullptr;

void AnimationManager::Init()
{
	if (!instance) {
		instance = new AnimationManager();
	}
}

AnimationManager* AnimationManager::Get()
{
	return instance;
}

void AnimationManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

std::shared_ptr<Animation> AnimationManager::RegisterAnimation(std::shared_ptr<Animation> animation_to_register, const std::string& file_path)
{
	auto animation_final = animation_map.insert(std::make_pair(file_path, animation_to_register));
	return animation_final.first->second;
}

std::shared_ptr<Animation> AnimationManager::LoadAnimationAsync(const std::string& file_path)
{
	using namespace std::filesystem;
	std::string absolute_path = absolute(path(file_path)).generic_string();
	std::string relative_path = FileManager::Get()->GetRelativeFilePath(absolute_path);


	std::unique_lock<std::mutex> lock(animation_map_mutex);
	auto fnd = animation_map.find(relative_path); 
	if (fnd != animation_map.end()) {
		return fnd->second;
	}

	Animation anim;

	anim.duration = 0.0f;
	anim.bone_anim = std::vector<BoneAnimation>();
	anim.ticks_per_second = 0;
	anim.status = Animation::animation_status::LOADING;

	auto anim_final = RegisterAnimation(std::make_unique<Animation>(anim), relative_path);


	auto async_queue = Application::GetAsyncDispather();

	auto task = async_queue->CreateTask<Animation>([absolute_path, this]() -> Animation {
		return LoadAnimationFromFile_impl(absolute_path);
		});

	async_queue->Submit(task);

	animation_load_future future;
	future.anim = task->GetFuture();
	future.animation_object = anim_final;
	future.path = relative_path;

	std::lock_guard<std::mutex> lock2(load_queue_mutex);
	load_queue.push_back(future);

	return anim_final; 
}

void AnimationManager::UpdateLoadedAnimations()
{
	std::lock_guard<std::mutex> lock(load_queue_mutex);
	for (auto& loaded_anim : load_queue) {
		if (!loaded_anim.anim.IsAvailable() || loaded_anim.processed) continue;
		try {
			*(loaded_anim.animation_object) = std::move(loaded_anim.anim.GetValue());

			loaded_anim.processed = true;
		}
		catch (...) {
			loaded_anim.animation_object->status = Animation::animation_status::ERROR;
			std::lock_guard<std::mutex> lock(animation_map_mutex);
			animation_map.erase(loaded_anim.path);
			loaded_anim.processed = true;
		}
	}


	while (!load_queue.empty() && load_queue.front().processed) {
		load_queue.pop_front();
	}

}

AnimationManager::AnimationManager()
{

}

void AnimationManager::MakeAnimations(Skeleton& reference_skeleton, aiScene* scene, const std::string& output_directory)
{
	if (!scene) throw std::runtime_error("Scene was not supplied to MakeAnimations");
	for (int i = 0; i < scene->mNumAnimations; i++) {
		auto anim = scene->mAnimations[i];
		if (reference_skeleton.GetNumberOfBones() != anim->mNumChannels) continue;
		std::string label = anim->mName.C_Str();
		std::string path = output_directory + "/" + (label.empty() ? "default" : label) + ".anim";
		std::ofstream file(path, std::ios::binary | std::ios::out);
		if (!file.is_open()) throw std::runtime_error("Animation File " + path + " could not be created.");
		
		file << anim->mDuration << " " << anim->mTicksPerSecond << " " << anim->mNumChannels << "\n";
		
		std::vector<BoneAnimation> anims;
		anims.reserve(anim->mNumChannels);
		for (int fill = 0; fill < anim->mNumChannels; fill++) {
			anims.emplace_back();
		}
		for (int channnel_index = 0; channnel_index < anim->mNumChannels; channnel_index++) {
			auto channel = anim->mChannels[channnel_index];
			if (!reference_skeleton.BoneExists(channel->mNodeName.C_Str())) throw std::runtime_error("Skeleton Incompatible with animation");
			int index = reference_skeleton.GetBoneEntryByName(channel->mNodeName.C_Str()).array_entry;
			BoneAnimation& bone = anims[index];
			for (int pos = 0; pos < channel->mNumPositionKeys; pos++) {
				auto position = channel->mPositionKeys[pos];
				bone.position_keyframes.push_back(BoneAnimationPositionKeyFrame{ *(glm::vec3*)&position.mValue,position.mTime });
			}
			for (int scl = 0; scl < channel->mNumScalingKeys; scl++) {
				auto scale = channel->mScalingKeys[scl];
				bone.scale_keyframes.push_back(BoneAnimationScaleKeyFrame{ *(glm::vec3*)&scale.mValue,scale.mTime });
			}
			for (int rot = 0; rot < channel->mNumRotationKeys; rot++) {
				auto rotation = channel->mRotationKeys[rot];
				bone.rotation_keyframes.push_back(BoneAnimationRotationKeyFrame{ *(glm::quat*)&rotation.mValue,rotation.mTime });
			}
		}

		for (auto& bone : anims) {
			file << "Pos" << " " << bone.position_keyframes.size() << "\n";
			for (auto& pos : bone.position_keyframes) {
				file.write((const char*)glm::value_ptr(pos.position), sizeof(glm::vec3));
				file.write((const char*)&pos.time_stamp, sizeof(double));
			}
			file << "\n";
			file << "Scale" << " " << bone.scale_keyframes.size() << "\n";
			for (auto& scl : bone.scale_keyframes) {
				file.write((const char*)glm::value_ptr(scl.scale), sizeof(glm::vec3));
				file.write((const char*)&scl.time_stamp, sizeof(double));
			}
			file << "\n";
			file << "Rot" << " " << bone.rotation_keyframes.size() << "\n";
			for (auto& rot : bone.rotation_keyframes) {
				file.write((const char*)glm::value_ptr(rot.rotation), sizeof(glm::quat));
				file.write((const char*)&rot.time_stamp, sizeof(double));
			}
			file << "\n";
		}
		file << "end";

		file.close();
	}
}

Animation AnimationManager::LoadAnimationFromFile_impl(const std::string& path)
{
	Animation anim;
	anim.status = Animation::animation_status::READY;
	std::ifstream file(path, std::ios::binary | std::ios::in);
	if (!file.is_open()) throw std::runtime_error("Animation File " + path + " could not be opened.");
	std::string check;
	int num_of_bones;
	file >> anim.duration >> anim.ticks_per_second >> num_of_bones;
	anim.bone_anim.reserve(num_of_bones);
	file.get();
	for (int bone = 0; bone < num_of_bones; bone++) {
		anim.bone_anim.emplace_back();
		auto& bone_anim = anim.bone_anim.back();
		
		file >> check;
		if (check != "Pos") throw std::runtime_error("Invalid Animation format");
		int num_of_pos_keyframes;
		file >> num_of_pos_keyframes;
		file.get();
		for (int i = 0; i < num_of_pos_keyframes; i++) {
			BoneAnimationPositionKeyFrame keyframe;
			file.read((char*)glm::value_ptr(keyframe.position), sizeof(glm::vec3));
			file.read((char*)&keyframe.time_stamp, sizeof(double));
			bone_anim.position_keyframes.push_back(keyframe);
		}
		file.get();

		file >> check;
		if (check != "Scale") throw std::runtime_error("Invalid Animation format");
		int num_of_scl_keyframes;
		file >> num_of_scl_keyframes;
		file.get();
		for (int i = 0; i < num_of_scl_keyframes; i++) {
			BoneAnimationScaleKeyFrame keyframe;
			file.read((char*)glm::value_ptr(keyframe.scale), sizeof(glm::vec3));
			file.read((char*)&keyframe.time_stamp, sizeof(double));
			bone_anim.scale_keyframes.push_back(keyframe);
		}
		file.get();

		file >> check;
		if (check != "Rot") throw std::runtime_error("Invalid Animation format");
		int num_of_rot_keyframes;
		file >> num_of_rot_keyframes;
		file.get();
		for (int i = 0; i < num_of_rot_keyframes; i++) {
			BoneAnimationRotationKeyFrame keyframe;
			file.read((char*)glm::value_ptr(keyframe.rotation), sizeof(glm::quat));
			file.read((char*)&keyframe.time_stamp, sizeof(double));
			bone_anim.rotation_keyframes.push_back(keyframe);
		}
		file.get();
	
	}
	file >> check;
	if (check != "end") throw std::runtime_error("Invalid Animation format");
	file.close();

	return anim;

}
