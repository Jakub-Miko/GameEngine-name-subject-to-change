#include "MeshManager.h"
#include "MeshManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <Renderer/RenderResourceManager.h>
#include <assimp/postprocess.h>
#include <Application.h>

MeshManager* MeshManager::instance = nullptr;

std::shared_ptr<Mesh> MeshManager::LoadMeshFromFile(const std::string& file_path, int mesh_index )
{
    std::unique_lock<std::mutex> lock(mesh_Map_mutex);
    auto fnd = mesh_Map.find(file_path); //TODO: Make sure the file path is in an appropriate format
    if (fnd != mesh_Map.end()) {
        return fnd->second;
    }
    lock.unlock();
    
    Assimp::Importer importer;

    Mesh mesh;
	const aiScene* scene = importer.ReadFile(file_path, aiProcess_GenBoundingBoxes); //TODO: Handle bounding boxes somehow
	aiMesh* imported_mesh = scene->mMeshes[mesh_index];

    auto command_list = Renderer::Get()->GetRenderCommandList(); //TODO:Make sure you dont use too many command lists.
    auto command_queue = Renderer::Get()->GetCommandQueue();

    mesh.num_of_indicies = (size_t)imported_mesh->mNumFaces * 3; //TODO: make sure the mesh is triangulated
    RenderBufferDescriptor index_buffer_desc(sizeof(unsigned int) * mesh.num_of_indicies, RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);

    struct Vertex { glm::vec3 pos; glm::vec3 normal; }; //TODO: give use more control over vertex buffer format

    size_t num_vertecies = imported_mesh->mNumVertices;
    Vertex* vertecies = new Vertex[num_vertecies];
    for (int i = 0; i < num_vertecies; i++) {
        vertecies[i] = Vertex{ *reinterpret_cast<glm::vec3*>(&imported_mesh->mVertices[i]),*reinterpret_cast<glm::vec3*>(&imported_mesh->mNormals[i]) };
    }

    RenderBufferDescriptor vertex_buffer_desc(sizeof(Vertex) * num_vertecies, RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);

    mesh.vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_buffer_desc);
    RenderResourceManager::Get()->UploadDataToBuffer(command_list, mesh.vertex_buffer, vertecies, sizeof(Vertex) * num_vertecies, 0);


    unsigned int* indicies = new unsigned int[mesh.num_of_indicies];
    for (int i = 0; i < mesh.num_of_indicies / 3; i++) {
        indicies[i * 3] = imported_mesh->mFaces[i].mIndices[0];
        indicies[i * 3 + 1] = imported_mesh->mFaces[i].mIndices[1];
        indicies[i * 3 + 2] = imported_mesh->mFaces[i].mIndices[2];
    }

    mesh.index_buffer = RenderResourceManager::Get()->CreateBuffer(index_buffer_desc);
    RenderResourceManager::Get()->UploadDataToBuffer(command_list, mesh.index_buffer, indicies, sizeof(unsigned int) * mesh.num_of_indicies, 0);

    command_queue->ExecuteRenderCommandList(command_list);

    lock.lock();

    auto mesh_final = mesh_Map.insert(std::make_pair(file_path, std::make_shared<Mesh>(mesh)));
    delete[] indicies;
    delete[] vertecies;

    return mesh_final.first->second; // Handle return types differently 

}

Future<std::shared_ptr<Mesh>> MeshManager::LoadMeshFromFileAsync(const std::string& file_path, int mesh_index)
{
    auto async_queue = Application::GetAsyncDispather();
    
    auto task = async_queue->CreateTask<std::shared_ptr<Mesh>>([file_path,mesh_index,this]() -> std::shared_ptr<Mesh> {
        PROFILE("LOADING MESH");
        std::unique_lock<std::mutex> lock(mesh_Map_mutex);
        auto fnd = mesh_Map.find(file_path); //TODO: Make sure the file path is in an appropriate format
        if (fnd != mesh_Map.end()) {
            return fnd->second;
        }
        lock.unlock();

        Assimp::Importer importer;

        Mesh mesh;
        const aiScene* scene = importer.ReadFile(file_path, aiProcess_GenBoundingBoxes); //TODO: Handle bounding boxes somehow
        aiMesh* imported_mesh = scene->mMeshes[mesh_index];

        auto command_list = Renderer::Get()->GetRenderCommandList(); //TODO:Make sure you dont use too many command lists.
        auto command_queue = Renderer::Get()->GetCommandQueue();

        mesh.num_of_indicies = (size_t)imported_mesh->mNumFaces * 3; //TODO: make sure the mesh is triangulated
        RenderBufferDescriptor index_buffer_desc(sizeof(unsigned int) * mesh.num_of_indicies, RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);

        struct Vertex { glm::vec3 pos; glm::vec3 normal; }; //TODO: give use more control over vertex buffer format

        size_t num_vertecies = imported_mesh->mNumVertices;
        Vertex* vertecies = new Vertex[num_vertecies];
        for (int i = 0; i < num_vertecies; i++) {
            vertecies[i] = Vertex{ *reinterpret_cast<glm::vec3*>(&imported_mesh->mVertices[i]),*reinterpret_cast<glm::vec3*>(&imported_mesh->mNormals[i]) };
        }

        RenderBufferDescriptor vertex_buffer_desc(sizeof(Vertex) * num_vertecies, RenderBufferType::DEFAULT, RenderBufferUsage::VERTEX_BUFFER);

        mesh.vertex_buffer = RenderResourceManager::Get()->CreateBuffer(vertex_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, mesh.vertex_buffer, vertecies, sizeof(Vertex) * num_vertecies, 0);


        unsigned int* indicies = new unsigned int[mesh.num_of_indicies];
        for (int i = 0; i < mesh.num_of_indicies / 3; i++) {
            indicies[i * 3] = imported_mesh->mFaces[i].mIndices[0];
            indicies[i * 3 + 1] = imported_mesh->mFaces[i].mIndices[1];
            indicies[i * 3 + 2] = imported_mesh->mFaces[i].mIndices[2];
        }

        mesh.index_buffer = RenderResourceManager::Get()->CreateBuffer(index_buffer_desc);
        RenderResourceManager::Get()->UploadDataToBuffer(command_list, mesh.index_buffer, indicies, sizeof(unsigned int) * mesh.num_of_indicies, 0);

        command_queue->ExecuteRenderCommandList(command_list);

        lock.lock();

        auto mesh_final = mesh_Map.insert(std::make_pair(file_path, std::make_shared<Mesh>(mesh)));
        delete[] indicies;
        delete[] vertecies;

        return mesh_final.first->second;
        });

    async_queue->Submit(task);
    
    return task->GetFuture();

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

MeshManager::MeshManager() : mesh_Map(), mesh_Map_mutex()
{
    RenderBufferDescriptor desc_v(0,RenderBufferType::DEFAULT,RenderBufferUsage::VERTEX_BUFFER);
    auto vertex_buf = RenderResourceManager::Get()->CreateBuffer(desc_v);
    
    
    RenderBufferDescriptor desc_i(0, RenderBufferType::DEFAULT, RenderBufferUsage::INDEX_BUFFER);
    auto index_buf = RenderResourceManager::Get()->CreateBuffer(desc_i);

    default_mesh = std::make_unique<Mesh>(vertex_buf, index_buf, 0);

}
