#include "MeshManager.h"
#include <Renderer/Renderer3D/Animations/AnimationManager.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <Renderer/RenderResourceManager.h>
#include <assimp/postprocess.h>
#include <Application.h>
#include <FileManager.h>

MeshManager* MeshManager::instance = nullptr;

Mesh MeshManager::LoadMeshFromFileImpl(const std::string& file_path)
{

    auto import_data = Fetch_Native_Data(file_path);

    Mesh mesh;
    mesh.status = Mesh_status::READY;
    mesh.bounding_box = import_data.bounding_box;
    if (import_data.skeleton != nullptr) {
        mesh.skeleton.reset(import_data.skeleton.get());
        import_data.skeleton.release();
    }
    auto command_list = Renderer::Get()->GetRenderCommandList(); //TODO:Make sure you dont use too many command lists.
    auto command_queue = Renderer::Get()->GetCommandQueue();

    mesh.num_of_indicies = import_data.index_count; //TODO: make sure the mesh is triangulated
    RenderBufferDescriptor index_buffer_desc(sizeof(unsigned int) * mesh.num_of_indicies, RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);

    RenderBufferDescriptor vertex_buffer_desc(import_data.vertex_size * import_data.vertex_count, RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);

    mesh.vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_buffer_desc);
    RenderResourceManager::Get()->UploadDataToBuffer(command_list, mesh.vertex_buffer, import_data.vertex_buffer, import_data.vertex_size * import_data.vertex_count, 0);

    mesh.index_buffer = RenderResourceManager::Get()->CreateBuffer(index_buffer_desc);
    RenderResourceManager::Get()->UploadDataToBuffer(command_list, mesh.index_buffer, import_data.index_buffer, sizeof(unsigned int) * import_data.index_count, 0);

    command_queue->ExecuteRenderCommandList(command_list);

    import_data.clear();
    return mesh;
}

void MeshManager::MakeMeshFromObjectFile(const std::string& in_file_path, const std::string& out_file_path, const VertexLayout& normal_layout, const VertexLayout& skeletal_mesh_layout, int mesh_index) {
    mesh_vertex_props props;
    VertexLayout layout = normal_layout;
    props.has_normal = layout.has_normal();
    props.has_position = layout.has_position();
    props.num_of_uv_channels = layout.GetUvCount();
    props.has_tangent = layout.has_tangent();
    mesh_assimp_input_data input_data = Fetch_Assimp_Data(props, in_file_path, mesh_index);
    bool has_bones = input_data.bone_Indicies != nullptr;
    if (has_bones) {
        if ((layout.has_normal() != skeletal_mesh_layout.has_normal()) ||
            (layout.has_position() != skeletal_mesh_layout.has_position()) ||
            (layout.GetUvCount() != skeletal_mesh_layout.GetUvCount()) ||
            (layout.has_tangent() != skeletal_mesh_layout.has_tangent())) {
            throw std::runtime_error("normal_mesh_layout and skeletal_mesh_layout must define the same basic properties besides bone_ids and bone_weights");
        }
        layout = skeletal_mesh_layout;
    }

    size_t num_vertecies = input_data.num_of_verticies;
    char* vertex_buffer = new char[num_vertecies * layout.stride];
    auto pos_element = layout.GetElement("position");
    auto normal_element = layout.GetElement("normal");
    auto tangent_element = layout.GetElement("tangent");
    VertexLayoutElement* uv_elements = new VertexLayoutElement[props.num_of_uv_channels];
    for (int i = 0; i < props.num_of_uv_channels; i++) {
        uv_elements[i] = layout.GetElement("uv" + std::to_string(i));
    }

    if (props.has_position) {
        switch (pos_element.size) {
        case 3:
            for (int i = 0; i < input_data.num_of_verticies; i++) {
                glm::vec3 pos_data = input_data.position[i];
                void* data = (void*)(vertex_buffer + ((layout.stride * i) + pos_element.offset));
                std::memcpy(data, &pos_data, sizeof(glm::vec3));
            }
            break;
        case 4:
            for (int i = 0; i < input_data.num_of_verticies; i++) {
                glm::vec4 pos_data = glm::vec4(input_data.position[i], 1.0f);
                void* data = (void*)(vertex_buffer + ((layout.stride * i) + pos_element.offset));
                std::memcpy(data, &pos_data, sizeof(glm::vec4));
            }
        }
    }

    if (props.has_normal) {
        switch (normal_element.size) {
        case 3:
            for (int i = 0; i < input_data.num_of_verticies; i++) {
                glm::vec3 normal_data = input_data.normal[i];
                void* data = (void*)(vertex_buffer + ((layout.stride * i) + normal_element.offset));
                std::memcpy(data, &normal_data, sizeof(glm::vec3));
            }
            break;
        case 4:
            for (int i = 0; i < input_data.num_of_verticies; i++) {
                glm::vec4 normal_data = glm::vec4(input_data.normal[i], 1.0f);
                void* data = (void*)(vertex_buffer + ((layout.stride * i) + normal_element.offset));
                std::memcpy(data, &normal_data, sizeof(glm::vec4));
            }
        }
    }

    if (props.has_tangent) {
        switch (tangent_element.size) {
        case 3:
            for (int i = 0; i < input_data.num_of_verticies; i++) {
                glm::vec3 tangent_data = input_data.tangent[i];
                void* data = (void*)(vertex_buffer + ((layout.stride * i) + tangent_element.offset));
                std::memcpy(data, &tangent_data, sizeof(glm::vec3));
            }
            break;
        case 4:
            for (int i = 0; i < input_data.num_of_verticies; i++) {
                glm::vec4 tangent_data = glm::vec4(input_data.normal[i], 1.0f);
                void* data = (void*)(vertex_buffer + ((layout.stride * i) + tangent_element.offset));
                std::memcpy(data, &tangent_data, sizeof(glm::vec4));
            }
        }
    }

    if (has_bones) {
        auto bone_ids_element = layout.GetElement("bone_ids");
        auto bone_weights_element = layout.GetElement("bone_weights");
        for (int i = 0; i < input_data.num_of_verticies; i++) {
            glm::uvec4 bone_ids = input_data.bone_Indicies[i];
            void* data = (void*)(vertex_buffer + ((layout.stride * i) + bone_ids_element.offset));
            std::memcpy(data, &bone_ids, sizeof(glm::uvec4));
        }
        for (int i = 0; i < input_data.num_of_verticies; i++) {
            glm::vec4 bone_weights = input_data.bone_weigths[i];
            void* data = (void*)(vertex_buffer + ((layout.stride * i) + bone_weights_element.offset));
            std::memcpy(data, &bone_weights, sizeof(glm::vec4));
        }
    }

    for (int x = 0; x < props.num_of_uv_channels; x++) {
        for (int i = 0; i < input_data.num_of_verticies; i++) {
            glm::vec2 uv_data = input_data.uvs[x][i];
            void* data = (void*)(vertex_buffer + ((layout.stride * i) + uv_elements[x].offset));
            std::memcpy(data, &uv_data, sizeof(glm::vec2));
        }
    }


    Write_assimp_processed_data((void*)vertex_buffer, layout.stride, input_data, out_file_path, layout);
    if (input_data.skeleton) {
        if (((const aiScene*)(input_data.imported_scene))->HasAnimations()) {
            auto animation_dir = std::filesystem::path(out_file_path).parent_path().generic_string() + "/" + std::filesystem::path(out_file_path).filename().generic_string() + "_animations/";
            std::filesystem::create_directory(animation_dir);
            AnimationManager::Get()->MakeAnimations(*input_data.skeleton, (aiScene*)input_data.imported_scene, animation_dir);
        }
    }
    input_data.clear();
    delete[] vertex_buffer;
    delete[] uv_elements;
}

std::shared_ptr<Mesh> MeshManager::LoadMeshFromFileAsync(const std::string& file_path)
{
    using namespace std::filesystem;
    std::string absolute_path = absolute(path(file_path)).generic_string();
    std::string relative_path = FileManager::Get()->GetRelativeFilePath(absolute_path);


    std::unique_lock<std::mutex> lock(mesh_Map_mutex);
    auto fnd = mesh_Map.find(relative_path); //TODO: Make sure the file path is in an appropriate format
    if (fnd != mesh_Map.end()) {
        return fnd->second;
    }

    Mesh mesh;

    mesh.status = Mesh_status::LOADING;
    mesh.num_of_indicies = GetDefaultMesh()->num_of_indicies;
    mesh.index_buffer = GetDefaultMesh()->index_buffer;
    mesh.vertex_buffer = GetDefaultMesh()->vertex_buffer;
    if (std::filesystem::path(absolute_path).extension().generic_string() == ".skel") {
        mesh.skeleton.reset(new Skeleton);
    }

    auto mesh_final = RegisterMesh(std::make_unique<Mesh>(mesh), relative_path);


    auto async_queue = Application::GetAsyncDispather();

    auto task = async_queue->CreateTask<Mesh>([absolute_path, this]() -> Mesh {
            return LoadMeshFromFileImpl(absolute_path);
        });

    async_queue->Submit(task);

    mesh_load_future future;
    future.future = task->GetFuture();
    future.mesh = mesh_final;
    future.path = relative_path;

    std::lock_guard<std::mutex> lock2(mesh_Load_queue_mutex);
    mesh_Load_queue.push_back(future);

    return mesh_final; // Handle return types differently 
}

void MeshManager::UpdateLoadedMeshes()
{
    std::lock_guard<std::mutex> lock(mesh_Load_queue_mutex);
    for (auto& loaded_mesh : mesh_Load_queue) {
        if (!loaded_mesh.future.IsAvailable() || loaded_mesh.destroyed) continue;
        try {
            *(loaded_mesh.mesh) = std::move(loaded_mesh.future.GetValue());
            
            loaded_mesh.destroyed = true;
        }
        catch (...) {
            loaded_mesh.mesh->status = Mesh_status::ERROR;
            std::lock_guard<std::mutex> lock(mesh_Map_mutex);
            mesh_Map.erase(loaded_mesh.path);
            loaded_mesh.destroyed = true;
        }
    }
   
    
    while (!mesh_Load_queue.empty() && mesh_Load_queue.front().destroyed) {
        mesh_Load_queue.pop_front();
    }

}

void MeshManager::Init()
{
	if (!instance) {
		instance = new MeshManager;
	}
}

MeshManager* MeshManager::Get()
{
	return instance;
}

void MeshManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

MeshManager::MeshManager() : mesh_Map(), mesh_Map_mutex(), mesh_Load_queue(), mesh_Load_queue_mutex()
{
    RenderBufferDescriptor desc_v(0,RenderBufferType::DEFAULT,RenderBufferUsage::VERTEX_BUFFER);
    auto vertex_buf = RenderResourceManager::Get()->CreateBuffer(desc_v);
    
    
    RenderBufferDescriptor desc_i(0, RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
    auto index_buf = RenderResourceManager::Get()->CreateBuffer(desc_i);

    default_mesh = std::make_unique<Mesh>(vertex_buf, index_buf, 0);

}

void MeshManager::ClearMeshCache()
{
    std::lock_guard<std::mutex> lock1(mesh_Load_queue_mutex);
    std::lock_guard<std::mutex> lock2(mesh_Map_mutex);
    mesh_Map.clear();

}
static bool CheckBoneRoot(Skeleton& skeleton, aiNode* node, aiNode** root_node);

MeshManager::mesh_assimp_input_data MeshManager::Fetch_Assimp_Data(const mesh_vertex_props& props, const std::string& in_file_path, int mesh_index) {
    Assimp::Importer* importer = new Assimp::Importer;
    mesh_assimp_input_data data;
    data.importer = importer;

    Mesh mesh;
    unsigned int flags = 0;
    flags |= aiProcess_GenBoundingBoxes | (props.has_normal ? aiProcess_GenNormals : 0);
    flags |= aiProcess_GenBoundingBoxes | (props.has_tangent ? aiProcess_CalcTangentSpace : 0);
    const aiScene* scene = importer->ReadFile(in_file_path, flags); //TODO: Handle bounding boxes somehow
    aiMesh* imported_mesh = scene->mMeshes[mesh_index];
    bool has_skeleton = imported_mesh->HasBones();
    auto& aabb = imported_mesh->mAABB;

    auto size = aabb.mMax - aabb.mMin;
    auto pos = (aabb.mMax + aabb.mMin) / aiVector3D(2.0);
    data.bounding_box = BoundingBox(*reinterpret_cast<glm::vec3*>(&size), *reinterpret_cast<glm::vec3*>(&pos));


    if (props.has_position) {
        if (imported_mesh->HasPositions()) {
            data.position = reinterpret_cast<glm::vec3*>(imported_mesh->mVertices);
            data.num_of_verticies = imported_mesh->mNumVertices;
        }
        else {
            throw std::runtime_error("Positions could not be generated.");
        }
    }

    if (props.has_normal && imported_mesh->HasNormals()) {
        if (imported_mesh->HasNormals()) {
            data.normal = reinterpret_cast<glm::vec3*>(imported_mesh->mNormals);
        }
        else {
            throw std::runtime_error("Normals could not be generated.");
        }
    }

    if (props.has_tangent && imported_mesh->HasTangentsAndBitangents()) {
        if (imported_mesh->HasNormals()) {
            data.tangent = reinterpret_cast<glm::vec3*>(imported_mesh->mTangents);
        }
        else {
            throw std::runtime_error("Normals could not be generated.");
        }
    }

    if (props.num_of_uv_channels > 0) {
        if (imported_mesh->GetNumUVChannels() >= props.num_of_uv_channels) {
            int num_of_channels = props.num_of_uv_channels;
            data.uvs = new glm::vec3 * [num_of_channels];
            data.num_of_uv_channels = props.num_of_uv_channels;
            for (int i = 0; i < num_of_channels; i++) {
                data.uvs[i] = reinterpret_cast<glm::vec3*>(imported_mesh->mTextureCoords[i]);
            }
        }
        else {
            throw std::runtime_error("UVs could not be generated.");
        }
    }

    data.num_of_indicies = (size_t)imported_mesh->mNumFaces * 3; //TODO: make sure the mesh is triangulated
    unsigned int* indicies = new unsigned int[data.num_of_indicies];
    for (int i = 0; i < data.num_of_indicies / 3; i++) {
        indicies[i * 3] = imported_mesh->mFaces[i].mIndices[0];
        indicies[i * 3 + 1] = imported_mesh->mFaces[i].mIndices[1];
        indicies[i * 3 + 2] = imported_mesh->mFaces[i].mIndices[2];
    }
    data.indicies = indicies;

    if (has_skeleton) {
        Skeleton* skeleton = new Skeleton;
        for (int i = 0; i < imported_mesh->mNumBones; i++) {
            skeleton->bone_hashmap.insert(std::make_pair(std::string(imported_mesh->mBones[i]->mName.C_Str()), Skeleton::bone_hashmap_entry{(uint16_t)-1,(uint16_t)i}));
        }

        data.bone_Indicies = new glm::uvec4[data.num_of_verticies];
        memset(data.bone_Indicies, 255, sizeof(glm::uvec4) * data.num_of_verticies);
        data.bone_weigths = new glm::vec4[data.num_of_verticies];
        aiNode* root = nullptr;
        CheckBoneRoot(*skeleton, scene->mRootNode, &root);
        if (!root) {
            throw std::runtime_error("Root bone could not be found");
        }
        skeleton->parent_bone_array.reserve(imported_mesh->mNumBones);
        BuildBoneHierarchy(*skeleton, root, -1, imported_mesh, &data);
        data.skeleton.reset(skeleton);
    }
    data.imported_scene = scene;
    return data;
}

void MeshManager::BuildBoneHierarchy(Skeleton& skeleton, aiNode* node, uint16_t parent_index, aiMesh* mesh, mesh_assimp_input_data* data) {
    if (!skeleton.BoneExists(node->mName.C_Str())) return;
    Bone current_bone;
    current_bone.name = node->mName.C_Str();
    current_bone.parent_index = parent_index;
    Skeleton::bone_hashmap_entry& entry = skeleton.GetBoneEntryReferenceByName(current_bone.name);
    aiBone* bone = mesh->mBones[entry.animation_file_entry];
    current_bone.offset_matrix = glm::transpose(glm::make_mat4(&(bone->mOffsetMatrix).a1));
    skeleton.parent_bone_array.push_back(current_bone);
    uint16_t current_index = skeleton.parent_bone_array.size() - 1;
    entry.array_entry = current_index;

    for (uint32_t i = 0; i < bone->mNumWeights; i++) {
        glm::uvec4* index = &data->bone_Indicies[bone->mWeights[i].mVertexId];
        glm::vec4* weight = &data->bone_weigths[bone->mWeights[i].mVertexId];
        for (int x = 0; x < 4; x++) {
            if ((*index)[x] == -1) {
                (*index)[x] = current_index;
                (*weight)[x] = bone->mWeights[i].mWeight;
                break;
            }
        }
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        BuildBoneHierarchy(skeleton, node->mChildren[i], current_index, mesh,data);
    }

}

static bool CheckBoneRoot(Skeleton& skeleton, aiNode* node, aiNode** root_node) {
    bool is_bone = skeleton.BoneExists(node->mName.C_Str());
    if (is_bone) {
        *root_node = node;
        return true;
    }
    for (int i = 0; i < node->mNumChildren; i++) {
        if (CheckBoneRoot(skeleton, node->mChildren[i], root_node)) return true;
    }
    return false;
}

void MeshManager::Write_assimp_processed_data(void* vertex_data, size_t vertex_size, const MeshManager::mesh_assimp_input_data& import_data, const std::string& out_data_path, const VertexLayout& layout)
{
    static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<float>::is_iec559, "This pc doesn't comply to IEEE 754 and thus isn't supported");
    
    bool has_bones = import_data.bone_Indicies != nullptr;

    std::string out_data = out_data_path + (has_bones ? ".skel" : ".mesh");

    std::ofstream output_file(out_data , std::ios_base::binary);
    if (!output_file.is_open()) {
        throw std::runtime_error("File " + out_data + " could not be created");
    }

    output_file << "mesh_info\n";
    output_file << import_data.num_of_indicies << "\n";
    output_file << import_data.num_of_verticies << "\n";
    output_file << import_data.num_of_uv_channels << "\n";
    output_file << (has_bones ? "skeletal_mesh" : "normal_mesh") << "\n";
    output_file << "vertex_layout\n";
    output_file << layout.layout.size() << "\n";
    for (auto& output_element : layout.layout) {
        output_file << " " << output_element.name << " " << output_element.size << " " << (int)output_element.type << "\n";
    }
    output_file << "vertex_buffer\n";
    output_file.write((const char*)vertex_data, vertex_size * import_data.num_of_verticies);
    output_file << "\nindex_buffer\n";
    output_file.write((const char*)import_data.indicies, sizeof(unsigned int) * import_data.num_of_indicies);
    output_file << "\nbounding_box\n";
    output_file << import_data.bounding_box.GetBoxSize().x << " " << import_data.bounding_box.GetBoxSize().y << " " << import_data.bounding_box.GetBoxSize().z << "\n";
    output_file << import_data.bounding_box.GetBoxOffset().x << " " << import_data.bounding_box.GetBoxOffset().y << " " << import_data.bounding_box.GetBoxOffset().z << "\n";
    if (has_bones) {
        output_file << "skeleton" << "\n";
        output_file << import_data.skeleton->parent_bone_array.size() << "\n";
        auto& skelton_bone_array = import_data.skeleton->parent_bone_array;
        for (auto& bone : skelton_bone_array) {
            auto offset_mat = glm::value_ptr(bone.offset_matrix);
            output_file << bone.name << " " << bone.parent_index << " " << import_data.skeleton->GetBoneEntryByName(bone.name).animation_file_entry << " ";
            output_file.write((const char*)offset_mat, sizeof(glm::mat4));
            output_file << "\n";
        }
    }

    output_file << "\nend";

    output_file.close();
}

void MeshManager::mesh_assimp_input_data::clear()
{
    delete[] uvs;
    delete[] indicies;
    if (bone_Indicies) {
        delete[] bone_Indicies;
    }
    if (bone_weigths) {
        delete[] bone_weigths;
    }

    delete (Assimp::Importer*)importer;
}

std::shared_ptr<Mesh> MeshManager::RegisterMesh(std::shared_ptr<Mesh> mesh_to_register, const std::string& file_path)
{
    auto mesh_final = mesh_Map.insert(std::make_pair(file_path,mesh_to_register));
    return mesh_final.first->second;
}

MeshManager::mesh_native_input_data MeshManager::Fetch_Native_Data(const std::string& in_file_path)
{
    std::ifstream input_file;
    try {
        input_file.open(in_file_path, std::ios_base::binary | std::ios_base::in);
    }
    catch(...) {
        throw std::runtime_error("File " + in_file_path + " could not be opened");
    }
    
    if (!input_file.is_open()) {
        throw std::runtime_error("File " + in_file_path + " could not be opened");
    }

    std::string check;
    mesh_native_input_data data;

    int num_of_indicies;
    int num_of_verticies;
    bool has_bones = false;

    input_file >> check;
    if (check != "mesh_info") throw std::runtime_error("Invalid Native Mesh Format");
    input_file >> num_of_indicies;
    input_file >> num_of_verticies;
    input_file >> check; // skip this since we dont need num_of_uv_channels
    //mesh_type
    input_file >> check;
    if (check == "skeletal_mesh") has_bones = true;

    input_file >> check;
    if (check != "vertex_layout") throw std::runtime_error("Invalid Native Mesh Format");
    int num_of_elements;
    input_file >> num_of_elements;
    std::vector<VertexLayoutElement> element_list;
    element_list.reserve(num_of_elements);
    for (int i = 0; i < num_of_elements; i++) {
        VertexLayoutElement element;
        int type;
        input_file >> element.name >> element.size >> type;
        element.type = (RenderPrimitiveType)type;
        element_list.push_back(element);
    }
    VertexLayout layout(element_list);

    input_file >> check;
    if (check != "vertex_buffer") throw std::runtime_error("Invalid Native Mesh Format");
    
    uint32_t size_of_vertex_buffer = num_of_verticies * layout.stride;
    char* vertex_buffer = new char[size_of_vertex_buffer];
    input_file.get();
    input_file.read(vertex_buffer, size_of_vertex_buffer);

    input_file >> check;
    if (check != "index_buffer") throw std::runtime_error("Invalid Native Mesh Format");
    input_file.get();
    unsigned int* index_buffer = new unsigned int[num_of_indicies];
    input_file.read((char*)index_buffer, sizeof(unsigned int) * num_of_indicies);
    input_file >> check;
    if (check == "bounding_box") {
        glm::vec3 size_aabb;
        glm::vec3 pos_aabb;
        input_file >> size_aabb.x >> size_aabb.y >> size_aabb.z;
        input_file >> pos_aabb.x >> pos_aabb.y >> pos_aabb.z;
        data.bounding_box = BoundingBox (size_aabb, pos_aabb);
        input_file >> check;
    }
    
    if (has_bones) {
        if(check != "skeleton") throw std::runtime_error("Invalid Native Mesh Format skeleton not found");
        int num_of_bones;
        input_file >> num_of_bones;
        data.skeleton.reset(new Skeleton);
        Skeleton& skel = *data.skeleton;
        for (int i = 0; i < num_of_bones; i++) {
            Bone bone;
            uint16_t anim_file_entry;
            input_file >> bone.name >> bone.parent_index >> anim_file_entry;
            input_file.get();
            input_file.read((char*)glm::value_ptr(bone.offset_matrix), sizeof(glm::mat4));
            input_file.get();
            skel.parent_bone_array.push_back(bone);
            skel.bone_hashmap.insert(std::make_pair(bone.name, Skeleton::bone_hashmap_entry{ (uint16_t)i, anim_file_entry }));
        }
        input_file >> check;


    }


    
    if (check != "end") throw std::runtime_error("Invalid Native Mesh Format");

    data.index_buffer = index_buffer;
    data.vertex_buffer = vertex_buffer;
    data.index_count = num_of_indicies;
    data.vertex_count = num_of_verticies;
    data.vertex_size = layout.stride;
    data.layout = layout;

    return data;

}

void MeshManager::mesh_native_input_data::clear()
{
    delete[] index_buffer;
    delete[](char*)vertex_buffer;
}
