#pragma once
#include "AnimationChannel.h"

class AnimationChannelGroup {
public:
    enum class status {
        LOADING,
        LOADED,
        ERROR
    };

    AnimationChannelGroup() = default;

    std::shared_ptr<AnimationChannel> GetChannel(int index) const {
        if(index < 0 || index >= channels.size()) {
            return nullptr;
        }
        return channels[index];
    }

    int GetChannelCount() const {
        return channels.size();
    }

    void AddChannel(std::shared_ptr<AnimationChannel> new_channel) {
        this->channels.push_back(new_channel);
    }

    void Clear() {
        channels.clear();
    }

    status GetStatus() const { return status; }

private:
    friend class AnimationManager;
    std::vector<std::shared_ptr<AnimationChannel>> channels;
    status status = status::ERROR;
};
