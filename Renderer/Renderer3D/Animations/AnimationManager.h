#pragma once
#include <unordered_map>
#include <Renderer/Renderer3D/Animations/Animation.h>
#include <mutex>
#include <deque>
#include <Renderer/RenderResource.h>
#include <Renderer/Renderer3D/Animations/Skeleton.h>
#include <AsyncTaskDispatcher.h>

struct aiScene;

class AnimationManager {
public:
	static void Init();

	static AnimationManager* Get();

	static void Shutdown();
	
	std::shared_ptr<Animation> LoadAnimationAsync(const std::string& path);

	void UpdateLoadedAnimations();

private:
	friend class MeshManager;
	AnimationManager();
	static AnimationManager* instance;


	std::shared_ptr<Animation> RegisterAnimation(std::shared_ptr<Animation> animation_to_register, const std::string& file_path);
	Animation LoadAnimationFromFile_impl(const std::string& path);
	void MakeAnimations(Skeleton& reference_skeleton, aiScene* scene, const std::string& output_directory);


	struct animation_load_future {
		Future<Animation> anim;
		std::shared_ptr<Animation> animation_object;
		std::string path;
		bool processed = false;
	};

	std::mutex animation_map_mutex;
	std::unordered_map<std::string, std::shared_ptr<Animation>> animation_map;
	std::mutex load_queue_mutex;
	std::deque<animation_load_future> load_queue;
};