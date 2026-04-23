#pragma once
#include "Renderer/Renderer3D/SkeletalAnimations/SkeletalAnimation.h"

template<typename T>
struct AnimationKeyframe {
    T value;
    double time;
};

template<typename T>
struct KeyFrameInterpolation {
    static T Interpolate(T start, T, double) {
        return start;
    }
};

template<>
struct KeyFrameInterpolation<glm::vec3> {
    static  glm::vec3 Interpolate(glm::vec3 start, glm::vec3 end, double t) {
        return glm::mix(start, end, t);
    }
};

template<>
struct KeyFrameInterpolation<glm::quat> {
    static glm::quat Interpolate(glm::quat start, glm::quat end, double t) {
        return glm::slerp(start, end, (float)t);
    }
};



class AnimationChannel {
public:
    template<typename T, typename keyframe_type = AnimationKeyframe<T>>
    static std::pair<keyframe_type, keyframe_type> GetSurroundingKeyframes(double time, const std::vector<keyframe_type>& keyframes) {
        if(keyframes.empty()) {
            return std::make_pair(keyframe_type{}, keyframe_type{});
        }

        if(keyframes.size() == 1) {
            return std::make_pair(keyframes[0], keyframes[0]);
        }

        auto lower_bound = std::lower_bound(keyframes.begin(), keyframes.end(), keyframe_type{ T(), time},
            [](const keyframe_type& k1, const keyframe_type& k2) {
                return k1.time < k2.time;
            });

        if(lower_bound == keyframes.end()) {
            return std::make_pair(keyframes[keyframes.size() - 2], keyframes.back());
        }

        if(lower_bound == keyframes.begin()) {
            return std::make_pair(keyframes[0], keyframes[1]);
        }

        return std::make_pair(*(lower_bound - 1), *lower_bound);
    }

    template<typename T, typename keyframe_type = AnimationKeyframe<T>>
    static T GetInterpolatedValue(double time, const std::vector<keyframe_type>& keyframes) {
        auto [start, end] = GetSurroundingKeyframes<T>(time, keyframes);
        double t = (time - start.time) / (end.time - start.time);
        if(end.time - start.time == 0) {
            t = 1.0f;
        }
        return KeyFrameInterpolation<T>::Interpolate(start.value, end.value, t);
    }

    glm::mat4 GetInterpolatedTransform(double time) const {
        auto transform = glm::mat4(1.0f);
        transform = glm::toMat4(GetInterpolatedValue<glm::quat>(time, rotation_keyframes)) * transform;
        //transform = glm::scale(glm::mat4(1.0f), GetInterpolatedValue<glm::vec3>(time, scale_keyframes)) * transform;
        transform = glm::translate(glm::mat4(1.0f), GetInterpolatedValue<glm::vec3>(time, position_keyframes)) * transform;
        return transform;
    }

    const std::string& GetName() const { return name; }

    double GetDuration() const { return duration; }
    double GetTicksPerSecond() const { return ticks_per_second; }

private:
    friend class AnimationManager;
    std::vector<AnimationKeyframe<glm::vec3>> position_keyframes;
    std::vector<AnimationKeyframe<glm::vec3>> scale_keyframes;
    std::vector<AnimationKeyframe<glm::quat>> rotation_keyframes;
    double duration;
    double ticks_per_second;
    std::string name;
};