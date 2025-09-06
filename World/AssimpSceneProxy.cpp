#include "AssimpSceneProxy.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Renderer/MeshManager.h>
#include <World/Components/MeshComponent.h>
#include <World/World.h>
#include <FileManager.h>


void AssimpSceneProxy::ProcessNode(LoadState& state, aiNode* node, aiNode* parent_node, Entity parent_entity) {
    auto& world = state.world;
    
    auto transform = node->mTransformation;
    aiVector3D translate,rotate_axis, scale;
    float rotate_angle;
    transform.Decompose(scale, rotate_axis, rotate_angle, translate);

    auto entity = world.CreateEntity(parent_entity, glm::vec3{translate.x, translate.y, translate.z}, 
                                               glm::vec3{scale.x, scale.y, scale.z}, 
                                               glm::vec3{rotate_axis.x, rotate_axis.y, rotate_axis.z}, rotate_angle);

    world.SetComponent<LabelComponent>(entity, LabelComponent(node->mName.C_Str()));
    
    if(node->mNumMeshes > 0) {
        if(node->mNumMeshes > 1) {
            for(int i = 0; i < node->mNumMeshes; i++) {
                auto mesh_entity = world.CreateEntity(entity);
                auto ai_mesh = state.scene->mMeshes[node->mMeshes[i]];
                world.SetComponent<LabelComponent>(mesh_entity, LabelComponent(node->mName.C_Str() + std::string("_") + ai_mesh->mName.C_Str()));
                //world.SetComponent<MeshComponent>(mesh_entity, MeshComponent(state.meshes[node->mMeshes[i]]));
            }
        } else {
            //world.SetComponent<MeshComponent>(entity, MeshComponent(state.meshes[node->mMeshes[0]]));
        }
    } 

    for(int i = 0; i < node->mNumChildren; i++) {
        auto child = node->mChildren[i];
        ProcessNode(state, child, node, entity);
    }

}

std::shared_ptr<Mesh> AssimpSceneProxy::LoadMesh(aiMesh *mesh)
{
    return std::shared_ptr<Mesh>();
}

void AssimpSceneProxy::LoadMeshes(LoadState& state) {
    for(int i = 0; i < state.scene->mNumMeshes; i++) {
        auto ai_mesh = state.scene->mMeshes[i];
        state.meshes.push_back(LoadMesh(ai_mesh));
    }
}

AssimpSceneProxy::LoadInfo AssimpSceneProxy::LoadScene(World &world)
{
    LoadInfo info = {};
    info.has_script = false;
    info.script = "";

    Assimp::Importer importer;
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    const aiScene* scene = importer.ReadFile(FileManager::Get()->GetPath(path), aiProcess_Triangulate);
    LoadState state = {scene, world, {}};

    if(!scene) {
        throw std::runtime_error("Scene could" + path + " not be loaded.");
    }

    LoadMeshes(state);

    ProcessNode(state, scene->mRootNode, nullptr, Entity());

    return info;
}
