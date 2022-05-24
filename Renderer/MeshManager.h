#pragma once
#include <string>
#include <unordered_map>
#include <Renderer/RendererDefines.h>
#include <Renderer/RenderResource.h>
#include <memory>

enum class Mesh_status : char {
	UNINITIALIZED = 0, LOADING = 1, READY = 2
};

class MeshManager;

class Mesh {
public:
	friend MeshManager;
	Mesh(const std::shared_ptr<RenderBufferResource>& vertex_buffer, const std::shared_ptr<RenderBufferResource>& index_buffer, size_t num_of_indicies) 
		: num_of_indicies(num_of_indicies), vertex_buffer(vertex_buffer), index_buffer(index_buffer), status(Mesh_status::READY) {}

	Mesh() = default;

	std::shared_ptr<RenderBufferResource> GetVertexBuffer() const {
		return vertex_buffer;
	}
	
	std::shared_ptr<RenderBufferResource> GetIndexBuffer() const {
		return index_buffer;
	}

	size_t GetIndexCount() const {
		return num_of_indicies;
	}

private:
	Mesh(const std::shared_ptr<RenderBufferResource>& vertex_buffer, const std::shared_ptr<RenderBufferResource>& index_buffer, size_t num_of_indicies,const std::string& path, Mesh_status status = Mesh_status::UNINITIALIZED)
		: num_of_indicies(num_of_indicies), vertex_buffer(vertex_buffer), index_buffer(index_buffer), file_path(path), status(status) {}

	size_t num_of_indicies = 0;
	std::string file_path = "None";
	std::shared_ptr<RenderBufferResource> vertex_buffer = nullptr;
	std::shared_ptr<RenderBufferResource> index_buffer = nullptr;
	Mesh_status status = Mesh_status::UNINITIALIZED;

};

class MeshManager {
public:

	MeshManager(const MeshManager& ref) = delete;
	MeshManager(MeshManager&& ref) = delete;
	MeshManager& operator=(const MeshManager& ref) = delete;
	MeshManager& operator=(MeshManager&& ref) = delete;	

	
	std::shared_ptr<Mesh> LoadMeshFromFile(const std::string& file_path,int mesh_index = 0);
	
	static void Init();
	
	static MeshManager* Get();

	static void Shutdown();
private:
	MeshManager();

	
	

	std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_Map;
	
	
	static MeshManager* instance;
};
