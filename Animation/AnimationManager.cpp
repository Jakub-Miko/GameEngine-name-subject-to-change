#include "AnimationManager.h"

#include "Application.h"
#include "FileManager.h"
#include "assimp/scene.h"

AnimationManager* AnimationManager::instance = nullptr;

void AnimationManager::Init() {
    if(!instance) {
        instance = new AnimationManager();
    }
}

AnimationManager* AnimationManager::Get() {
    return instance;
}

void AnimationManager::Shutdown() {
    if(instance) {
        delete instance;
    }
}

std::shared_ptr<AnimationChannelGroup> AnimationManager::LoadAnimationChannelGroupAsync(const std::string& path) {
    using namespace std::filesystem;
    std::string relative_path = FileManager::Get()->GetPath(path);

    std::unique_lock<std::mutex> lock(animation_map_mutex);
    auto fnd = animation_map.find(relative_path);
    if (fnd != animation_map.end()) {
        return fnd->second;
    }

    AnimationChannelGroup anim;

    anim.status = AnimationChannelGroup::status::LOADING;

    auto anim_final = RegisterAnimationChannelGroup(std::make_unique<AnimationChannelGroup>(anim), relative_path);

    auto async_queue = Application::GetAsyncDispather();

    auto task = async_queue->CreateTask<AnimationChannelGroup>([relative_path, this]() -> AnimationChannelGroup {
        return LoadAnimationChannelGroupFromFile_impl(relative_path);
        });

    async_queue->Submit(task);

    animation_load_future future;
    future.anim = task->GetFuture();
    future.animation_object = anim_final;
    future.path = relative_path;
	future.processed = false;

    std::lock_guard<std::mutex> lock2(load_queue_mutex);
    load_queue.push_back(future);

    return anim_final;
}

void AnimationManager::UpdateLoadedAnimations() {
    std::lock_guard<std::mutex> lock(load_queue_mutex);
    for (auto& loaded_anim : load_queue) {
        if (!loaded_anim.anim.IsAvailable() || loaded_anim.processed) continue;
        try {
            *(loaded_anim.animation_object) = std::move(loaded_anim.anim.GetValue());

            loaded_anim.processed = true;
        }
        catch (...) {
            loaded_anim.animation_object->status = AnimationChannelGroup::status::ERROR;
            std::lock_guard<std::mutex> lock(animation_map_mutex);
            animation_map.erase(loaded_anim.path);
            loaded_anim.processed = true;
        }
    }


    while (!load_queue.empty() && load_queue.front().processed) {
        load_queue.pop_front();
    }
}

AnimationManager::AnimationManager() {

}

std::shared_ptr<AnimationChannelGroup> AnimationManager::RegisterAnimationChannelGroup( std::shared_ptr<AnimationChannelGroup> animation_to_register, const std::string& file_path) {
    auto animation_final = animation_map.insert(std::make_pair(file_path, animation_to_register));
    return animation_final.first->second;
}

AnimationChannelGroup AnimationManager::LoadAnimationChannelGroupFromFile_impl(const std::string& path) {
    AnimationChannelGroup anim;
	anim.status = AnimationChannelGroup::status::LOADED;
	std::ifstream file(path, std::ios::binary | std::ios::in);
	if (!file.is_open()) throw std::runtime_error("Animation File " + path + " could not be opened.");
	std::string check;
	int num_of_channels = 0;
	file >> num_of_channels;
	file.get();
	for (int i = 0; i < num_of_channels; i++) {
		auto channel = std::make_shared<AnimationChannel>();
		std::getline(file, channel->name);
		file >> channel->duration >> channel->ticks_per_second;
		file.get();
		file >> check;
		if (check != "Pos") throw std::runtime_error("Invalid Animation format");
		int num_of_pos_keyframes;
		file >> num_of_pos_keyframes;
		file.get();
		for (int i = 0; i < num_of_pos_keyframes; i++) {
			AnimationKeyframe<glm::vec3> keyframe;
			file.read((char*)glm::value_ptr(keyframe.value), sizeof(glm::vec3));
			file.read((char*)&keyframe.time, sizeof(double));
			channel->position_keyframes.push_back(keyframe);
		}
		file.get();

		file >> check;
		if (check != "Scale") throw std::runtime_error("Invalid Animation format");
		int num_of_scl_keyframes;
		file >> num_of_scl_keyframes;
		file.get();
		for (int i = 0; i < num_of_scl_keyframes; i++) {
			AnimationKeyframe<glm::vec3> keyframe;
			file.read((char*)glm::value_ptr(keyframe.value), sizeof(glm::vec3));
			file.read((char*)&keyframe.time, sizeof(double));
			channel->scale_keyframes.push_back(keyframe);
		}
		file.get();

		file >> check;
		if (check != "Rot") throw std::runtime_error("Invalid Animation format");
		int num_of_rot_keyframes;
		file >> num_of_rot_keyframes;
		file.get();
		for (int i = 0; i < num_of_rot_keyframes; i++) {
			AnimationKeyframe<glm::quat> keyframe;
			file.read((char*)glm::value_ptr(keyframe.value), sizeof(glm::quat));
			file.read((char*)&keyframe.time, sizeof(double));
			channel->rotation_keyframes.push_back(keyframe);
		}
		file.get();
		file >> check;
		if (check != "end") throw std::runtime_error("Invalid Animation format");
		anim.AddChannel(channel);
	}
	file.close();

	return anim;
}

void AnimationManager::MakeAnimationChannelGroupsFromAssimpScene(const aiScene* scene, const std::string& output_directory) {
	if (!scene) throw std::runtime_error("Scene was not supplied to MakeAnimations");
	for (int i = 0; i < scene->mNumAnimations; i++) {
		auto anim = scene->mAnimations[i];
		std::string label = anim->mName.C_Str();
		std::string path = output_directory + "/" + (label.empty() ? "default" : label) + ".anim";
		std::ofstream file(path, std::ios::binary | std::ios::out);
		if (!file.is_open()) throw std::runtime_error("Animation File " + path + " could not be created.");

		std::vector<AnimationChannel> channels;
		channels.reserve(anim->mNumChannels);
		file << anim->mNumChannels << "\n";
		for (int channnel_index = 0; channnel_index < anim->mNumChannels; channnel_index++) {
			auto channel = anim->mChannels[channnel_index];
			auto& anim_channel = channels.emplace_back();
			file << channel->mNodeName.C_Str() << "\n";
			anim_channel.position_keyframes.clear();
			anim_channel.scale_keyframes.clear();
			anim_channel.rotation_keyframes.clear();
			for (int pos = 0; pos < channel->mNumPositionKeys; pos++) {
				auto position = channel->mPositionKeys[pos];
				anim_channel.position_keyframes.push_back( AnimationKeyframe<glm::vec3>{ *(glm::vec3*)&position.mValue, (float)position.mTime });
			}
			for (int scl = 0; scl < channel->mNumScalingKeys; scl++) {
				auto scale = channel->mScalingKeys[scl];
				anim_channel.scale_keyframes.push_back(AnimationKeyframe<glm::vec3>{ *(glm::vec3*)&scale.mValue, (float)scale.mTime });
			}
			for (int rot = 0; rot < channel->mNumRotationKeys; rot++) {
				auto rotation = channel->mRotationKeys[rot];
				anim_channel.rotation_keyframes.push_back(AnimationKeyframe<glm::quat>{ *(glm::quat*)&rotation.mValue,(float)rotation.mTime });
			}

		}

		for (auto& channel : channels) {
			file << anim->mDuration << " " << anim->mTicksPerSecond << "\n";
			file << "Pos" << " " << channel.position_keyframes.size() << "\n";
			for (auto& pos : channel.position_keyframes) {
				file.write((const char*)glm::value_ptr(pos.value), sizeof(glm::vec3));
				file.write((const char*)&pos.time, sizeof(double));
			}
			file << "\n";
			file << "Scale" << " " << channel.scale_keyframes.size() << "\n";
			for (auto& scl : channel.scale_keyframes) {
				file.write((const char*)glm::value_ptr(scl.value), sizeof(glm::vec3));
				file.write((const char*)&scl.time, sizeof(double));
			}
			file << "\n";
			file << "Rot" << " " << channel.rotation_keyframes.size() << "\n";
			for (auto& rot : channel.rotation_keyframes) {
				file.write((const char*)glm::value_ptr(rot.value), sizeof(glm::quat));
				file.write((const char*)&rot.time, sizeof(double));
			}
			file << "\n";
		}
		file << "end";

		file.close();
	}
}

void AnimationManager::ClearAnimationCache() {
	std::lock_guard<std::mutex> lock(animation_map_mutex);
	animation_map.clear();
}
