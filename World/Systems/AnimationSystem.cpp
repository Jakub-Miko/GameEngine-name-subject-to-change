#include "AnimationSystem.h"
#include <World/System.h>

#include "World/Components/AnimationComponent.h"
#include "World/Components/TransformComponent.h"

void AnimationSystem(World& world, float delta_time) {
    auto func_1 = [&world, delta_time](ComponentCollection compcol, system_view_type<AnimationComponent>& comps, entt::registry* reg) {
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            auto& comp = reg->get<AnimationComponent>(*iter);
            comp.UpdateComponent();
            if(comp.driving_channel) {
                if(comp.play_mode == AnimationComponent::PlayMode::PLAYING) {
                    comp.time += delta_time * comp.playback_speed;
                }
                if(comp.time > comp.driving_channel->GetDuration()) {
                    if(comp.looping) {
                        comp.SetTime(std::fmod(comp.time, comp.driving_channel->GetDuration()));
                    } else {
                        comp.SetTime(0.0);
                        comp.play_mode = AnimationComponent::PlayMode::STOPPED;
                    }
                }
                auto transform = comp.driving_channel->GetInterpolatedTransform(comp.time);
                world.SetEntityTransform(Entity(*iter), transform);
            }
        }

    };
    AnimationManager::Get()->UpdateLoadedAnimations();
    RunSystemSimple<AnimationComponent>(world, func_1);
}
