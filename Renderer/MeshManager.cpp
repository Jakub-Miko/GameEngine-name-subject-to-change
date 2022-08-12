#include "MeshManager.h"
#include "MeshManager.h"
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
            auto mesh_l = loaded_mesh.future.GetValue();
            loaded_mesh.mesh->index_buffer = mesh_l.index_buffer;
            loaded_mesh.mesh->vertex_buffer = mesh_l.vertex_buffer;
            loaded_mesh.mesh->num_of_indicies = mesh_l.num_of_indicies;
            loaded_mesh.mesh->status = Mesh_status::READY;
            loaded_mesh.mesh->bounding_box = mesh_l.bounding_box;
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

    return data;
}

void MeshManager::Write_assimp_processed_data(void* vertex_data, size_t vertex_size, const MeshManager::mesh_assimp_input_data& import_data, const std::string& out_data, const VertexLayout& layout)
{
    static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<float>::is_iec559, "This pc doesn't comply to IEEE 754 and thus isn't supported");
    
    std::ofstream output_file(out_data , std::ios_base::binary);
    if (!output_file.is_open()) {
        throw std::runtime_error("File " + out_data + " could not be created");
    }

    output_file << "mesh_info\n";
    output_file << import_data.num_of_indicies << "\n";
    output_file << import_data.num_of_verticies << "\n";
    output_file << import_data.num_of_uv_channels << "\n";
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
    output_file << "\nend";

    output_file.close();
}

void MeshManager::mesh_assimp_input_data::clear()
{
    delete[] uvs;
    delete[] indicies;
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

    input_file >> check;
    if (check != "mesh_info") throw std::runtime_error("Invalid Native Mesh Format");
    input_file >> num_of_indicies;
    input_file >> num_of_verticies;
    input_file >> check; // skip this since we dont need num_of_uv_channels

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
