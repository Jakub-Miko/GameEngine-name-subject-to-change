#include "AssimpSceneProxy.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Renderer/MeshManager.h>
#include <World/Components/MeshComponent.h>
#include <World/Components/LightComponent.h>
#include <World/Components/CameraComponent.h>
#include <World/World.h>
#include <FileManager.h>
#include <iostream>

#include "Components/SerializableComponent.h"


void AssimpSceneProxy::ProcessNode(LoadState& state, aiNode* node, aiNode* parent_node, Entity parent_entity) {
    // if(node->mMetaData) {
    //     Inspect_Metadata(state, node->mMetaData);
    // }
    auto& world = state.world;
    auto transform = node->mTransformation;
    aiVector3D translate,rotate_axis, scale;
    float rotate_angle;
    transform.Decompose(scale, rotate_axis, rotate_angle, translate);
    auto entity = world.CreateEntity(parent_entity, glm::vec3{translate.x, translate.y, translate.z}, 
                                               glm::vec3{scale.x, scale.y, scale.z}, 
                                               glm::vec3{rotate_axis.x, rotate_axis.y, rotate_axis.z}, rotate_angle);

    world.SetComponent<LabelComponent>(entity, LabelComponent(node->mName.C_Str()));
    if(state.serialize) {
        world.SetComponent<SerializableComponent>(entity, SerializableComponent{});
    }
    
    state.name_map.insert(std::make_pair(node->mName.C_Str(), LoadState::imported_entity {entity, node}));

    if(node->mNumMeshes > 0) {
        if(node->mNumMeshes > 1) {
            for(int i = 0; i < node->mNumMeshes; i++) {
                auto mesh_entity = world.CreateEntity(entity);
                auto ai_mesh = state.open_scene->scene->mMeshes[node->mMeshes[i]];
                world.SetComponent<LabelComponent>(mesh_entity, LabelComponent(node->mName.C_Str() + std::string("_") + ai_mesh->mName.C_Str()));
                world.SetComponent<MeshComponent>(mesh_entity, MeshComponent(state.meshes[node->mMeshes[i]],state.materials[ai_mesh->mMaterialIndex]));
                if(state.serialize) {
                    world.SetComponent<SerializableComponent>(mesh_entity);
                }
            }
        } else {
            auto ai_mesh = state.open_scene->scene->mMeshes[node->mMeshes[0]];
            world.SetComponent<MeshComponent>(entity, MeshComponent(state.meshes[node->mMeshes[0]], state.materials[ai_mesh->mMaterialIndex]));
            if(state.serialize) {
                world.SetComponent<SerializableComponent>(entity, SerializableComponent{});
            }
        }
    } 

    for(int i = 0; i < node->mNumChildren; i++) {
        auto child = node->mChildren[i];
        ProcessNode(state, child, node, entity);
    }

}

void AssimpSceneProxy::LoadMaterials(LoadState &state)
{
    auto manager = MaterialManager::Get();
    auto mat_template = MaterialManager::Get()->GetMaterialTemplate("DeferredGPassMaterial");
    auto root_path = std::filesystem::path(path).parent_path().generic_string() + "/";
    bool serialized = state.serialize;
    for(int i = 0; i < state.open_scene->scene->mNumMaterials; i++) {
        auto proxy = std::make_shared<AssimpMaterialProxy>(path, i, state.open_scene, serialized ? state.scene_resource_directory + "/materials/mat_" + std::to_string(i) + ".mat" : "");
        auto material = proxy->LoadMaterial();
        state.materials.push_back(material);
    }
}

void AssimpSceneProxy::LoadMeshes(LoadState& state) {
    auto manager = MeshManager::Get();
    for(int i = 0; i < state.open_scene->scene->mNumMeshes; i++) {
        auto proxy = std::make_shared<AssimpMeshProxy>(path, i, state.open_scene, state.serialize ? state.scene_resource_directory + "/" + state.open_scene->scene->mMeshes[i]->mName.C_Str() + ".mesh" : "");
        state.meshes.push_back(manager->LoadMeshFromProxyAsync(proxy));
    }
}

void AssimpSceneProxy::Inspect_Metadata(AssimpSceneProxy::LoadState& state, aiMetadata* data, int level) {
    for(int i = 0; i < data->mNumProperties; i++) {
        auto key = data->mKeys[i];
        auto value = data->mValues[i];

        for(int x = 0 ; x < level; x++){
            std::cout << "  ";
        }
        std::cout << key.C_Str() << "\n";


        switch(value.mType) {
        case aiMetadataType::AI_AIMETADATA:
            Inspect_Metadata(state, (aiMetadata*)value.mData, level + 1);
            break;
        case aiMetadataType::AI_DOUBLE:
            for(int x = 0 ; x < level; x++){
                std::cout << "  ";
            }
            std::cout << *(double*)value.mData << "\n";;
            break;
        case aiMetadataType::AI_UINT32:
            for(int x = 0 ; x < level; x++){
                std::cout << "  ";
            }
            std::cout << *(uint32_t*)value.mData << "\n";;
            break;
        case aiMetadataType::AI_UINT64:
            for(int x = 0 ; x < level; x++){
                std::cout << "  ";
            }
            std::cout << *(uint64_t*)value.mData << "\n";;
            break;
        }

    }
}


aiMetadataEntry* GetMetadata(const std::string& name, aiMetadata* parent) {
    if(!parent) {
        return nullptr;
    }
    for(int i = 0; i < parent->mNumProperties; i++) {
        if(parent->mKeys[i].C_Str() == name) {
            return &parent->mValues[i];
        }
    }
    
    return nullptr;
}

aiMetadataEntry *AssimpSceneProxy::GetNestedMetadata(const std::vector<std::string> &path, aiMetadata *parent)
{
    aiMetadataEntry* node = nullptr;
    for(int i = 0; i < path.size(); i++) {
        auto& path_segment = path[i];
        node = GetMetadata(path_segment ,parent);

        if(i == path.size() - 1 && node) {
            return node;
        }

        if(!node || node->mType != AI_AIMETADATA) {
            return nullptr;
        } 
        parent = (aiMetadata*)node->mData;
    }

    return nullptr;
}

void AssimpSceneProxy::ProbeSceneMetadata(LoadState &state) {
    auto gltf_light_array_meta = GetNestedMetadata({"extensions", "KHR_lights_punctual", "lights"}, state.open_scene->scene->mMetaData);

    if(!gltf_light_array_meta || gltf_light_array_meta->mType != AI_AIMETADATA) {
        return;
    }

    state.gltf_light_meta = (aiMetadata*)gltf_light_array_meta->mData;
}

double GetDouble(aiMetadataEntry* entry) {
    switch(entry->mType) {
    case aiMetadataType::AI_DOUBLE:
        return *(double*)entry->mData;
    case aiMetadataType::AI_UINT32:
        return *(uint32_t*)entry->mData;
    case aiMetadataType::AI_UINT64:
        return *(uint64_t*)entry->mData;
    default:
        return 0.0f;
    }
}
 
void AssimpSceneProxy::LoadLights(LoadState &state)
{
    //Inspect_Metadata(state, state.open_scene->scene->mMetaData);

    auto& world = state.world;
    for(int i = 0; i < state.open_scene->scene->mNumLights; i++) {
        auto light = state.open_scene->scene->mLights[i];
        auto translate = light->mPosition;

        auto fnd = state.name_map.find(light->mName.C_Str());
        if(fnd == state.name_map.end()) {
            continue;
        }
        auto light_imported_ent = fnd->second;
        auto light_ent = light_imported_ent.entity;
        glm::vec4 color = glm::vec4(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b, 1.0f);
        double range = 1.0f;

        if(state.gltf_light_meta && light_imported_ent.entity_node->mMetaData) {
            auto index = GetNestedMetadata({"extensions", "KHR_lights_punctual", "light"}, light_imported_ent.entity_node->mMetaData);
            if(index && index->mType == AI_UINT64) {
                auto index_int = *(uint64_t*)index->mData;
                if(index_int > state.gltf_light_meta->mNumProperties) {
                    throw std::runtime_error("Corrupted gltf light metadata.\n");
                }
                auto metadata_entry = state.gltf_light_meta->mValues[*(uint64_t*)index->mData];
                if(metadata_entry.mType != AI_AIMETADATA) {
                    throw std::runtime_error("Corrupted gltf light metadata.\n");
                }
                auto metadata = (aiMetadata*)metadata_entry.mData;

                auto color_entry = GetMetadata("color", metadata);
                if(color_entry && color_entry->mType == AI_AIMETADATA) {
                    auto color_meta = (aiMetadata*)color_entry->mData;
                    if(color_meta->mNumProperties != 3) {
                        throw std::runtime_error("Corrupted gltf light metadata.\n");
                    }
                    color.r = GetDouble(&color_meta->mValues[0]);
                    color.g = GetDouble(&color_meta->mValues[1]);
                    color.b = GetDouble(&color_meta->mValues[2]);
                }

                auto intensity_entry = GetMetadata("intensity", metadata);
                if(intensity_entry) {
                    color.a = GetDouble(intensity_entry);
                }

                auto range_entry = GetMetadata("range", metadata);
                if(range_entry) {
                    range = GetDouble(range_entry);
                } else {
                    range = sqrt(color.a / 0.005);
                }

            }
        }

        switch (light->mType)
        {
        case aiLightSourceType::aiLightSource_POINT:
            {
                world.SetComponent<LightComponent>(light_ent, LightComponent(range, color));
                if(state.serialize) {
                    world.SetComponent<SerializableComponent>(light_ent, SerializableComponent{});
                }
                break;
            }
        case aiLightSourceType::aiLightSource_DIRECTIONAL:
            {
                world.SetComponent<LightComponent>(light_ent, LightComponent(LightType::DIRECTIONAL,color));
                if(state.serialize) {
                    world.SetComponent<SerializableComponent>(light_ent, SerializableComponent{});
                }
                break;
            }
        default:
            break;
        }
        
    }
}

void AssimpSceneProxy::LoadCameras(LoadState &state)
{
    auto& world = state.world;
    for(int i = 0; i < state.open_scene->scene->mNumCameras; i++) {
        auto camera = state.open_scene->scene->mCameras[i];

        auto fnd = state.name_map.find(camera->mName.C_Str());
        if(fnd == state.name_map.end()) {
            continue;
        }
        auto camera_imported_ent = fnd->second;
        auto camera_ent = camera_imported_ent.entity;

        auto props = Application::Get()->GetWindow()->GetProperties();
		float aspect_ratio = (float)props.resolution_x / (float)props.resolution_y;

        world.SetComponent<CameraComponent>(camera_ent, CameraComponent(glm::degrees(camera->mHorizontalFOV), camera->mClipPlaneNear, camera->mClipPlaneFar, aspect_ratio));
        if(state.serialize) {
            world.SetComponent<SerializableComponent>(camera_ent, SerializableComponent{});
        }
    }
}

AssimpSceneProxy::LoadInfo AssimpSceneProxy::LoadScene(World &world)
{
    LoadInfo info = {};
    info.has_script = false;
    info.script = "";

    LoadState state = {world};
    state.serialize = !scene_save_path.empty();

    if(!scene_save_path.empty()) {
        auto scene_path = std::filesystem::path(FileManager::Get()->GetPath(scene_save_path));
        if(scene_path.extension() != ".scene") {
            throw std::runtime_error("Scene path must have the .scene extension.\n");
        }
        auto directory = scene_path.parent_path() / scene_path.stem();

        if(std::filesystem::exists(directory)) {
            if(!std::filesystem::is_directory(directory)) {
                throw std::runtime_error("Scene resource directory could not be created, because another file with the same name already exists. (Scene directory: " + directory.generic_string() + ")\n");
            }
            if(!std::filesystem::is_empty(directory)) {
                throw std::runtime_error("Scene resource directory exists but is not an empty directory: " + directory.generic_string() + "\n");
            }
        } else {
            std::filesystem::create_directory(directory);
        }
        state.scene_resource_directory = directory;
    }

    Assimp::Importer* importer = new Assimp::Importer;
    importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer->SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, scale_factor);
    importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_LIGHTS, true);

    auto layout = VertexLayoutFactory<MeshPreset>::GetLayout();
    bool has_normal = layout->has_normal();
    bool has_position = layout->has_position();
    int num_of_uv_channels = layout->GetUvCount();
    bool has_tangent = layout->has_tangent();

    Mesh mesh;
    unsigned int flags = 0;
    flags |= aiProcess_GenBoundingBoxes | (has_normal ? aiProcess_GenNormals : 0);
    flags |= aiProcess_GenBoundingBoxes | (has_tangent ? aiProcess_CalcTangentSpace : 0);

    const aiScene* scene = importer->ReadFile(FileManager::Get()->GetPath(path), flags |aiProcess_Triangulate | aiProcess_GlobalScale );
    auto open_scene = std::make_shared<AssimpOpenScene>();
    open_scene->importer = importer;
    open_scene->scene = scene;
    state.open_scene = open_scene;

    if(!scene) {
        throw std::runtime_error("Scene could" + path + " not be loaded.\n");
    }

    LoadMeshes(state);
    LoadMaterials(state);
    
    ProcessNode(state, scene->mRootNode, nullptr, Entity());
    ProbeSceneMetadata(state);

    LoadLights(state);
    LoadCameras(state);

    info.save_after_load = state.serialize;

    return info;
}

MeshSourceData AssimpMeshProxy::LoadMesh()
{
    MeshSourceData data;

    if(!open_scene) {
        throw std::runtime_error("Mesh loading error: The assimp scene file " + path + "is not longer open.\n");
    } 

    auto layout = VertexLayoutFactory<MeshPreset>::GetLayout();
    bool has_normal = layout->has_normal();
    bool has_position = layout->has_position();
    int num_of_uv_channels = layout->GetUvCount();
    bool has_tangent = layout->has_tangent();

    Mesh mesh;
    auto scene = open_scene->scene;
    aiMesh* imported_mesh = scene->mMeshes[mesh_index];
    bool has_skeleton = imported_mesh->HasBones();
    auto& aabb = imported_mesh->mAABB;

    auto size = aabb.mMax - aabb.mMin;
    auto pos = (aabb.mMax + aabb.mMin) / aiVector3D(2.0);
    data.bounding_box = BoundingBox(*reinterpret_cast<glm::vec3*>(&size), *reinterpret_cast<glm::vec3*>(&pos));


    uint64_t num_of_verticies = 0;
    uint64_t num_of_indicies = 0;
    glm::vec3* position = nullptr;
    glm::vec3* normal = nullptr;
    glm::vec3* tangent = nullptr;
    glm::vec3* bitangent = nullptr;
    glm::vec3** uvs = nullptr;

    if (has_position) {
        if (imported_mesh->HasPositions()) {
            position = reinterpret_cast<glm::vec3*>(imported_mesh->mVertices);
            num_of_verticies = imported_mesh->mNumVertices;
        }
        else {
            throw std::runtime_error("Positions could not be generated.");
        }
    }

    if (has_normal && imported_mesh->HasNormals()) {
        if (imported_mesh->HasNormals()) {
            normal = reinterpret_cast<glm::vec3*>(imported_mesh->mNormals);
        }
        else {
            throw std::runtime_error("Normals could not be generated.");
        }
    }

    if (has_tangent && imported_mesh->HasTangentsAndBitangents()) {
        if (imported_mesh->HasNormals()) {
            tangent = reinterpret_cast<glm::vec3*>(imported_mesh->mTangents);
            bitangent = reinterpret_cast<glm::vec3*>(imported_mesh->mBitangents);
        }
        else {
            throw std::runtime_error("Normals could not be generated.");
        }
    }

    if (num_of_uv_channels > 0) {
        if (imported_mesh->GetNumUVChannels() >= num_of_uv_channels) {
            uvs = new glm::vec3 * [num_of_uv_channels];
            for (int i = 0; i < num_of_uv_channels; i++) {
                uvs[i] = reinterpret_cast<glm::vec3*>(imported_mesh->mTextureCoords[i]);
            }
        }
        else {
            throw std::runtime_error("UVs could not be generated.");
        }
    }

    num_of_indicies = (size_t)imported_mesh->mNumFaces * 3; //TODO: make sure the mesh is triangulated
    unsigned int* indicies = new unsigned int[num_of_indicies];
    for (int i = 0; i < num_of_indicies / 3; i++) {
        indicies[i * 3] = imported_mesh->mFaces[i].mIndices[0];
        indicies[i * 3 + 1] = imported_mesh->mFaces[i].mIndices[1];
        indicies[i * 3 + 2] = imported_mesh->mFaces[i].mIndices[2];
    }

    // if (has_skeleton) {
    //     Skeleton* skeleton = new Skeleton;
    //     for (int i = 0; i < imported_mesh->mNumBones; i++) {
    //         skeleton->bone_hashmap.insert(std::make_pair(std::string(imported_mesh->mBones[i]->mName.C_Str()), Skeleton::bone_hashmap_entry{(uint16_t)-1,(uint16_t)i}));
    //     }

    //     data.bone_Indicies = new glm::uvec4[data.num_of_verticies];
    //     memset(data.bone_Indicies, 255, sizeof(glm::uvec4) * data.num_of_verticies);
    //     data.bone_weigths = new glm::vec4[data.num_of_verticies];
    //     aiNode* root = nullptr;
    //     CheckBoneRoot(*skeleton, scene->mRootNode, &root);
    //     if (!root) {
    //         throw std::runtime_error("Root bone could not be found");
    //     }
    //     skeleton->parent_bone_array.reserve(imported_mesh->mNumBones);
    //     BuildBoneHierarchy(*skeleton, root, -1, imported_mesh, &data);
    //     data.skeleton.reset(skeleton);
    // }

    char* vertex_buffer = new char[num_of_verticies * layout->stride];
    auto pos_element = layout->GetElement("position");
    auto normal_element = layout->GetElement("normal");
    auto tangent_element = layout->GetElement("tangent");
    VertexLayoutElement* uv_elements = new VertexLayoutElement[num_of_uv_channels];
    for (int i = 0; i < num_of_uv_channels; i++) {
        uv_elements[i] = layout->GetElement("uv" + std::to_string(i));
    }

    if (has_position) {
        switch (pos_element.size) {
        case 3:
            for (int i = 0; i < num_of_verticies; i++) {
                glm::vec3 pos_data = position[i];
                void* data = (void*)(vertex_buffer + ((layout->stride * i) + pos_element.offset));
                std::memcpy(data, &pos_data, sizeof(glm::vec3));
            }
            break;
        case 4:
            for (int i = 0; i < num_of_verticies; i++) {
                glm::vec4 pos_data = glm::vec4(position[i], 1.0f);
                void* data = (void*)(vertex_buffer + ((layout->stride * i) + pos_element.offset));
                std::memcpy(data, &pos_data, sizeof(glm::vec4));
            }
        }
    }

    if (has_normal) {
        switch (normal_element.size) {
        case 3:
            for (int i = 0; i < num_of_verticies; i++) {
                glm::vec3 normal_data = normal[i];
                void* data = (void*)(vertex_buffer + ((layout->stride * i) + normal_element.offset));
                std::memcpy(data, &normal_data, sizeof(glm::vec3));
            }
            break;
        case 4:
            for (int i = 0; i < num_of_verticies; i++) {
                glm::vec4 normal_data = glm::vec4(normal[i], 1.0f);
                void* data = (void*)(vertex_buffer + ((layout->stride * i) + normal_element.offset));
                std::memcpy(data, &normal_data, sizeof(glm::vec4));
            }
        }
    }

    if (has_tangent) {
        switch (tangent_element.size) {
        case 3:
            for (int i = 0; i < num_of_verticies; i++) {
                glm::vec3 tangent_data = tangent[i];
                void* data = (void*)(vertex_buffer + ((layout->stride * i) + tangent_element.offset));
                std::memcpy(data, &tangent_data, sizeof(glm::vec3));
            }
            break;
        case 4:
            for (int i = 0; i < num_of_verticies; i++) {
                float sign = (glm::dot(glm::cross(normal[i], tangent[i]), bitangent[i]) < 0.0f) ? -1.0f : 1.0f;
                glm::vec4 tangent_data = glm::vec4(tangent[i], sign);
                void* data = (void*)(vertex_buffer + ((layout->stride * i) + tangent_element.offset));
                std::memcpy(data, &tangent_data, sizeof(glm::vec4));
            }
        }
    }

    // if (has_bones) {
    //     auto bone_ids_element = layout.GetElement("bone_ids");
    //     auto bone_weights_element = layout.GetElement("bone_weights");
    //     for (int i = 0; i < input_data.num_of_verticies; i++) {
    //         glm::uvec4 bone_ids = input_data.bone_Indicies[i];
    //         void* data = (void*)(vertex_buffer + ((layout.stride * i) + bone_ids_element.offset));
    //         std::memcpy(data, &bone_ids, sizeof(glm::uvec4));
    //     }
    //     for (int i = 0; i < input_data.num_of_verticies; i++) {
    //         glm::vec4 bone_weights = input_data.bone_weigths[i];
    //         void* data = (void*)(vertex_buffer + ((layout.stride * i) + bone_weights_element.offset));
    //         std::memcpy(data, &bone_weights, sizeof(glm::vec4));
    //     }
    // }

    for (int x = 0; x < num_of_uv_channels; x++) {
        for (int i = 0; i < num_of_verticies; i++) {
            glm::vec2 uv_data = uvs[x][i];
            void* data = (void*)(vertex_buffer + ((layout->stride * i) + uv_elements[x].offset));
            std::memcpy(data, &uv_data, sizeof(glm::vec2));
        }
    }

    data.index_buffer = indicies;
    data.index_count = num_of_indicies;
    data.layout = *layout;
    data.skeleton = nullptr;
    data.vertex_buffer = vertex_buffer;
    data.vertex_count = num_of_verticies;
    data.vertex_size = layout->stride;

    if(native_file_path.empty()) {
        open_scene.reset();
        return data;
    }

    // Serialize into a native file

    static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<float>::is_iec559, "This pc doesn't comply to IEEE 754 and thus isn't supported");

    //bool has_bones = import_data.bone_Indicies != nullptr;

    std::string out_data = FileManager::Get()->GetPath(native_file_path);

    std::ofstream output_file(out_data , std::ios_base::binary);
    if (!output_file.is_open()) {
        throw std::runtime_error("File " + out_data + " could not be created");
    }

    output_file << "mesh_info\n";
    output_file << data.index_count << "\n";
    output_file << data.vertex_count << "\n";
    output_file << num_of_uv_channels << "\n";
    output_file << "normal_mesh" << "\n";
    output_file << "vertex_layout\n";
    output_file << layout->layout.size() << "\n";
    for (auto& output_element : layout->layout) {
        output_file << " " << output_element.name << " " << output_element.size << " " << (int)output_element.type << "\n";
    }
    output_file << "vertex_buffer\n";
    output_file.write((const char*)vertex_buffer, layout->stride * data.vertex_count);
    output_file << "\nindex_buffer\n";
    output_file.write((const char*)data.index_buffer, sizeof(unsigned int) * data.index_count);
    output_file << "\nbounding_box\n";
    output_file << data.bounding_box.GetBoxSize().x << " " << data.bounding_box.GetBoxSize().y << " " << data.bounding_box.GetBoxSize().z << "\n";
    output_file << data.bounding_box.GetBoxOffset().x << " " << data.bounding_box.GetBoxOffset().y << " " << data.bounding_box.GetBoxOffset().z << "\n";
    // if (has_bones) {
    //     output_file << "skeleton" << "\n";
    //     output_file << import_data.skeleton->parent_bone_array.size() << "\n";
    //     auto& skelton_bone_array = import_data.skeleton->parent_bone_array;
    //     for (auto& bone : skelton_bone_array) {
    //         auto offset_mat = glm::value_ptr(bone.offset_matrix);
    //         output_file << bone.name << " " << bone.parent_index << " " << import_data.skeleton->GetBoneEntryByName(bone.name).animation_file_entry << " ";
    //         output_file.write((const char*)offset_mat, sizeof(glm::mat4));
    //         output_file << "\n";
    //     }
    // }

    output_file << "\nend";

    output_file.close();

    open_scene.reset();
    return data;
}

const std::string &AssimpMeshProxy::GetNativeFilePath()
{
    if(!native_file_path.empty()) {
        return native_file_path;
    } else  {
       throw std::runtime_error("No native file exists for the mesh.");
    }
}

bool AssimpMeshProxy::IsSkeletal()
{
    return false;
}

std::shared_ptr<Material> AssimpMaterialProxy::LoadMaterial() {

    if(!open_scene) {
        throw std::runtime_error("Mesh loading error: The assimp scene file " + path + "is not longer open.\n");
    }

    std::string mat_resource_path = "";
    bool serialize = !native_file_path.empty();

    if(serialize) {
        auto filesys_path =  std::filesystem::path(FileManager::Get()->GetPath(native_file_path));
        mat_resource_path = filesys_path.parent_path();

        if(std::filesystem::exists(mat_resource_path)) {
            if(!std::filesystem::is_directory(mat_resource_path)) {
                throw std::runtime_error("Material texture path exists but is not a directory: " + mat_resource_path);
            }
        } else {
            std::filesystem::create_directory(mat_resource_path);
        }

        mat_resource_path = mat_resource_path / filesys_path.stem();

        if(std::filesystem::exists(mat_resource_path)) {
            if(!std::filesystem::is_directory(mat_resource_path)) {
                throw std::runtime_error("Material texture path exists but is not a directory: " + mat_resource_path);
            }
            if(!std::filesystem::is_empty(mat_resource_path)) {
                throw std::runtime_error("Material texture exists but is not an empty directory: " + mat_resource_path);
            }
        } else {
            std::filesystem::create_directory(mat_resource_path);
        }
    }

    auto manager = MaterialManager::Get();
    auto mat_template = manager->GetMaterialTemplate("DeferredGPassMaterial");
    auto root_path = std::filesystem::path(path).parent_path().generic_string() + "/";
    auto mat = open_scene->scene->mMaterials[material_index];
    auto material = mat_template->CreateMaterial();
    glm::vec4 color(1.0f,1.0f,1.0f,1.0f);
    if(aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE,(aiColor4D*)glm::value_ptr(color)) == AI_SUCCESS) {
        material->SetParameter("Base_Color", color);
    }
    float roughness_factor = 1.0f;
    if(aiGetMaterialFloat(mat, AI_MATKEY_ROUGHNESS_FACTOR,&roughness_factor) == AI_SUCCESS) {
        material->SetParameter("roughness_gain", roughness_factor);
    }
    aiString texture_path;
    std::string real_path;
    if(aiGetMaterialTexture(mat, aiTextureType::aiTextureType_DIFFUSE,0, &texture_path) == AI_SUCCESS) {
        real_path = root_path + texture_path.C_Str();
        std::replace(real_path.begin(), real_path.end(), '\\', '/');
        auto proxy = std::make_shared<StbiTextureProxy>(real_path, serialize ? mat_resource_path+ "/diffuse.tex" : "");
        material->SetTexture("Color", proxy);
    }
    if(aiGetMaterialTexture(mat, aiTextureType::aiTextureType_NORMALS,0, &texture_path) == AI_SUCCESS) {
        real_path = root_path + texture_path.C_Str();
        std::replace(real_path.begin(), real_path.end(), '\\', '/');
        auto proxy = std::make_shared<StbiTextureProxy>(real_path, serialize ? mat_resource_path + "/normal.tex" : "");
        material->SetTexture("Normal", proxy);
    }
    if(aiGetMaterialTexture(mat, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS,0, &texture_path) == AI_SUCCESS) {
        real_path = root_path + texture_path.C_Str();
        std::replace(real_path.begin(), real_path.end(), '\\', '/');
        auto proxy = std::make_shared<StbiTextureProxy>(real_path, serialize ? mat_resource_path + "/roughness.tex" : "");
        material->SetTexture("Roughness", proxy);
    }

    material->SetMaterialProxy(shared_from_this());

    if(native_file_path.empty()) {
        return material;
    }

    if(serialize) {
        MaterialManager::Get()->SerializeMaterial(FileManager::Get()->GetPath(native_file_path), material);
    }

    return material;
}

const std::string& AssimpMaterialProxy::GetNativeFilePath() {
    if(!native_file_path.empty()) {
        return native_file_path;
    } else  {
        throw std::runtime_error("No native file exists for the material.");
    }
}

AssimpSceneProxy::AssimpOpenScene::~AssimpOpenScene()
{
    delete importer;
}
