#pragma once
#include <memory>

#include "TypeId.h"
#include "Animation/AnimationChannel.h"
#include "Animation/AnimationManager.h"

class AnimationChannelGroup;

class AnimationComponent {
    RUNTIME_TAG("AnimationComponent");
public:
    AnimationComponent() = default;

    enum class PlayMode {
        STOPPED = 0, PLAYING = 1
    };

    void SetAnimation(const std::string& path, int index) {
        channel_index = index;
        animation_group_path = path;
        dirty = true;
    }

    void UpdateComponent(bool force = false) {
        if(!dirty && !force) {
            return;
        }
        auto group = AnimationManager::Get()->LoadAnimationChannelGroupAsync(animation_group_path);
        switch(group->GetStatus()) {
            case AnimationChannelGroup::status::LOADING:
                break;
            case AnimationChannelGroup::status::ERROR:
            {
                ResetComponent();
                break;
            }
            case AnimationChannelGroup::status::LOADED:
            {
                auto channel = group->GetChannel(channel_index);
                if(channel) {
                    driving_channel = channel;
                    owning_group = group;
                    dirty = false;
                } else {
                    ResetComponent();
                }
                break;
            }
            default:
                dirty = false;
                break;
        }
    }

    void ResetComponent() {
        owning_group = nullptr;
        driving_channel = nullptr;
        animation_group_path = "";
        channel_index = 0;
        play_mode = PlayMode::STOPPED;
        looping = false;
        time = 0.0f;
        playback_speed = 1.0f;
        dirty = false;
    }

    void Play() {
        play_mode = PlayMode::PLAYING;
    }

    void Stop() {
        play_mode = PlayMode::STOPPED;
    }

    void SetLooping(bool loop) {
        looping = loop;
    }

    void SetTime(float time) {
        this->time = time;
    }

    void SetPlaybackSpeed(float speed) {
        playback_speed = speed;
    }

public:
    std::shared_ptr<AnimationChannel> driving_channel = nullptr;
    std::shared_ptr<AnimationChannelGroup> owning_group = nullptr;
    uint32_t channel_index = 0;
    std::string animation_group_path = "";
    float time = 0.0f;
    PlayMode play_mode = PlayMode::STOPPED;
    bool looping = false;
    bool dirty = true;
    float playback_speed = 1.0f;
};

JSON_SERIALIZABLE(AnimationComponent, animation_group_path, channel_index)