#include "LoadingMeshSystem.h"
#include <World/World.h>
#include <World/System.h>
#include <World/Components/MeshComponent.h>
#include <World/Systems/ScriptSystemManagement.h>
#include <vector>
#include <mutex>

void LoadingMeshSystem(World& world) {
    std::mutex deletion_mutex;
    std::vector<entt::entity> deletion_list;
    
    auto func_1 = [&world,&deletion_list,&deletion_mutex](ComponentCollection compcol, system_view_type<LoadingMeshComponent>& comps, entt::registry* reg) {
        auto script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        if (!script_vm) {
            ScriptSystemManager::Get()->InitializeScriptSystemVM();
            script_vm = ScriptSystemManager::Get()->TryGetScriptSystemVM();
        }

        for (auto iter = comps.rbegin() + compcol.start_index; iter != comps.rbegin() + compcol.start_index + compcol.size; iter++) {
            if (reg->all_of<LoadingMeshComponent, MeshComponent>(*iter)) {
                auto& load = reg->get<LoadingMeshComponent>(*iter);
                auto& mesh = reg->get<MeshComponent>(*iter);
                if (load.mesh_future.IsAvailable()) {
                    auto mesh_l = load.mesh_future.GetValue();
                    mesh.mesh = mesh_l;
                    mesh.status = Mesh_status::READY;
                    std::lock_guard<std::mutex> lock(deletion_mutex);
                    deletion_list.push_back(*iter);
                }

                
            }
        };

    };

    RunSystemSimple<LoadingMeshComponent>(world, func_1);
    world.GetRegistry().remove<LoadingMeshComponent>(deletion_list.begin(), deletion_list.end());
}
