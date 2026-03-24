#pragma once
#include <deque>
#include <memory>
#include <unordered_map>

#include "AnimationChannel.h"
#include "AnimationChannelGroup.h"
#include "Promise.h"

struct aiScene;

class AnimationManager {
public:
    static void Init();
    static AnimationManager* Get();
    static void Shutdown();

    std::shared_ptr<AnimationChannelGroup> LoadAnimationChannelGroupAsync(const std::string& path);

    void UpdateLoadedAnimations();

    void MakeAnimationChannelGroupsFromAssimpScene(const aiScene* scene, const std::string& output_directory);

private:
    AnimationManager();
    static AnimationManager* instance;


    std::shared_ptr<AnimationChannelGroup> RegisterAnimationChannelGroup(std::shared_ptr<AnimationChannelGroup> animation_to_register, const std::string& file_path);
    AnimationChannelGroup LoadAnimationChannelGroupFromFile_impl(const std::string& path);
    friend class World;

    void ClearAnimationCache();

    struct animation_load_future {
        Future<AnimationChannelGroup> anim;
        std::shared_ptr<AnimationChannelGroup> animation_object;
        std::string path;
        bool processed = false;
    };

    std::mutex animation_map_mutex;
    std::unordered_map<std::string, std::shared_ptr<AnimationChannelGroup>> animation_map;
    std::mutex load_queue_mutex;
    std::deque<animation_load_future> load_queue;
};