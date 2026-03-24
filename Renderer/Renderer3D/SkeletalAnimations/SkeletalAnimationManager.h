#pragma once
#include <unordered_map>
#include <Renderer/Renderer3D/SkeletalAnimations/SkeletalAnimation.h>
#include <mutex>
#include <deque>
#include <Renderer/RenderResource.h>
#include <Renderer/Renderer3D/SkeletalAnimations/Skeleton.h>
#include <AsyncTaskDispatcher.h>

/** @page animation_page Animations
 * Hello world
 */

struct aiScene;

class SkeletalAnimationManager {
public:
	static void Init();

	static SkeletalAnimationManager* Get();

	static void Shutdown();
	
	std::shared_ptr<SkeletalAnimation> LoadAnimationAsync(const std::string& path);

	void UpdateLoadedAnimations();

	std::shared_ptr<SkeletalAnimation> GetDefaultAnimation() const {
		return default_animation;
	}

private:
	friend class MeshManager;
	SkeletalAnimationManager();
	static SkeletalAnimationManager* instance;


	std::shared_ptr<SkeletalAnimation> RegisterAnimation(std::shared_ptr<SkeletalAnimation> animation_to_register, const std::string& file_path);
	SkeletalAnimation LoadAnimationFromFile_impl(const std::string& path);
	void MakeAnimations(Skeleton& reference_skeleton, aiScene* scene, const std::string& output_directory);
	friend class World;

	void ClearAnimationCache();

	struct animation_load_future {
		Future<SkeletalAnimation> anim;
		std::shared_ptr<SkeletalAnimation> animation_object;
		std::string path;
		bool processed = false;
	};

	std::shared_ptr<SkeletalAnimation> default_animation;
	std::mutex animation_map_mutex;
	std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>> animation_map;
	std::mutex load_queue_mutex;
	std::deque<animation_load_future> load_queue;
};