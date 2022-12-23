#include "AudioUpdateSystem.h"
#include <World/System.h>
#include <Audio/AudioSystem.h>
#include <World/Components/AudioComponent.h>

void AudioUpdateSystem(World& world)
{
    Entity camera = Application::GetWorld().GetPrimaryEntity();
    glm::vec3 camera_pos = Application::GetWorld().GetComponent<TransformComponent>(camera).TransformMatrix[3];
    AudioSystem::Get()->SetListenerPosition(camera_pos);
    
    auto func_1 = [&world](ComponentCollection compcol, system_view_type<AudioComponent>& comps, entt::registry* reg) {
        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            AudioSystem::Get()->UpdateAudioComponent(*iter);
        }
    };

    RunSystemSimple<AudioComponent>(world, func_1);
}
