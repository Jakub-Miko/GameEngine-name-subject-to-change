#pragma once
#include <Renderer/Renderer3D/Animations/Skeleton.h>
#include <string>
#include <Events/SubjectObserver.h>
#include <cstring>
#include <unordered_map>
#include <Core/BoundingVolumes.h>
#include <Renderer/RendererDefines.h>
#include <Renderer/RenderResource.h>
#include <AsyncTaskDispatcher.h>
#include <mutex>
#include <memory>


enum class Mesh_status : char {
	UNINITIALIZED = 0, LOADING = 1, READY = 2, ERROR = 3
};

class MeshManager;
struct aiNode;
struct aiMesh;

class Mesh {
public:
	friend MeshManager;
	Mesh(const std::shared_ptr<RenderBufferResource>& vertex_buffer, const std::shared_ptr<RenderBufferResource>& index_buffer, size_t num_of_indicies)
		: num_of_indicies(num_of_indicies), vertex_buffer(vertex_buffer), index_buffer(index_buffer){}
	Mesh() = default;

	Mesh(const Mesh& other) : index_buffer(other.index_buffer), vertex_buffer(other.vertex_buffer), num_of_indicies(other.num_of_indicies),
		status(other.status), bounding_box(other.bounding_box), skeleton(nullptr) {
		if (other.skeleton != nullptr) {
			skeleton.reset(new Skeleton(*other.skeleton.get()));
		}
	};

	Mesh(Mesh&& other) : index_buffer(other.index_buffer), vertex_buffer(other.vertex_buffer), num_of_indicies(other.num_of_indicies), 
		status(other.status), bounding_box(other.bounding_box), skeleton(other.skeleton.get()) {
		other.skeleton.release();
		other.status = Mesh_status::UNINITIALIZED;
	};
	Mesh& operator=(Mesh&& other) {
		index_buffer = other.index_buffer;
		vertex_buffer = other.vertex_buffer;
		num_of_indicies = other.num_of_indicies;
		status = other.status;
		bounding_box = other.bounding_box;
		skeleton.reset(other.skeleton.get());
		other.skeleton.release();
		other.status = Mesh_status::UNINITIALIZED;
		return *this;
	}

	Mesh& operator=(const Mesh& other) {
		index_buffer = other.index_buffer;
		vertex_buffer = other.vertex_buffer;
		num_of_indicies = other.num_of_indicies;
		status = other.status;
		bounding_box = other.bounding_box;
		if (other.skeleton != nullptr) {
			skeleton.reset(new Skeleton(*other.skeleton.get()));
		}
		return *this;
	}


	std::shared_ptr<RenderBufferResource> GetVertexBuffer() const {
		return vertex_buffer;
	}
	
	std::shared_ptr<RenderBufferResource> GetIndexBuffer() const {
		return index_buffer;
	}

	Mesh_status GetMeshStatus() const {
		return status;
	}

	size_t GetIndexCount() const {
		return num_of_indicies;
	}

	const BoundingBox& GetBoundingBox() const {
		return bounding_box;
	}

	bool IsSkeletal() const {
		return skeleton != nullptr;
	}

	const Skeleton& GetSkeleton() {
		if (!IsSkeletal()) {
			throw std::runtime_error("Can't get a skeleton of a non skeletal object");
		}
		return *skeleton.get();
	}

private:

	size_t num_of_indicies = 0;
	BoundingBox bounding_box = BoundingBox();
	Mesh_status status = Mesh_status::UNINITIALIZED;
	std::shared_ptr<RenderBufferResource> vertex_buffer = nullptr;
	std::shared_ptr<RenderBufferResource> index_buffer = nullptr;
	std::unique_ptr<Skeleton> skeleton = nullptr;
};

class MeshManager {
public:

	struct mesh_native_input_data {
		BoundingBox bounding_box = BoundingBox();
		void* vertex_buffer = nullptr;
		uint32_t vertex_count = 0;
		unsigned int* index_buffer = nullptr;
		uint32_t index_count = 0;
		int vertex_size = 0;
		VertexLayout layout;
		std::unique_ptr<Skeleton> skeleton = nullptr;

		void clear();
	};


	MeshManager(const MeshManager& ref) = delete;
	MeshManager(MeshManager&& ref) = delete;
	MeshManager& operator=(const MeshManager& ref) = delete;
	MeshManager& operator=(MeshManager&& ref) = delete;	

	void MakeMeshFromObjectFile(const std::string& in_file_path, const std::string& out_file_path, const VertexLayout& normal_layout, const VertexLayout& skeletal_mesh_layout, int mesh_index = 0);

	std::shared_ptr<Mesh> LoadMeshFromFileAsync(const std::string& file_path);
	
	std::shared_ptr<Mesh> GetDefaultMesh() const {
		return default_mesh;
	}

	std::shared_ptr<Mesh> GetDefaultSkeletalMesh() const {
		return default_skeletal_mesh;
	}

	mesh_native_input_data Fetch_Native_Data(const std::string& in_file_path);

	//Only call when meshes are actively being used
	void UpdateLoadedMeshes();

	static void Init();
	
	static MeshManager* Get();

	static void Shutdown();
private:
	friend class MeshComponent;

	MeshManager();
	
	friend class World;

	void ClearMeshCache();


	struct mesh_vertex_props {
		uint32_t num_of_uv_channels = 0;
		bool has_position = true;
		bool has_normal = true;
		bool has_tangent = true;
	};

	struct mesh_assimp_input_data {
		BoundingBox bounding_box = BoundingBox();
		glm::vec3* position = nullptr;
		glm::vec3* normal = nullptr;
		glm::vec3* tangent = nullptr;
		glm::vec3** uvs = nullptr;
		glm::uvec4* bone_Indicies = nullptr;
		glm::vec4* bone_weigths = nullptr;
		std::unique_ptr<Skeleton> skeleton = nullptr;
		unsigned int *indicies = nullptr;
		int num_of_indicies;
		uint32_t num_of_verticies = 0;
		uint32_t num_of_uv_channels = 0;
		void* importer = nullptr;
		const void* imported_scene = nullptr;
		 
		void clear();

	};

	void BuildBoneHierarchy(Skeleton& skeleton, aiNode* node, uint16_t parent_index, aiMesh* mesh, mesh_assimp_input_data* data);

	struct mesh_load_future {
		std::shared_ptr<Mesh> mesh;
		Future<Mesh> future;
		bool destroyed = false;
		std::string path;
	};

	Mesh LoadMeshFromFileImpl(const std::string& file_path);

	std::shared_ptr<Mesh> RegisterMesh(std::shared_ptr<Mesh> file_path, const std::string& name);


	mesh_assimp_input_data Fetch_Assimp_Data(const mesh_vertex_props& props, const std::string& in_file_path, int mesh_index = 0);

	void Write_assimp_processed_data(void* vertex_data, size_t vertex_size, const mesh_assimp_input_data& import_data,const std::string& out_data, const VertexLayout& layout);
	std::shared_ptr<Mesh> default_mesh;
	std::shared_ptr<Mesh> default_skeletal_mesh;
	std::mutex mesh_Map_mutex;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_Map;
	std::mutex mesh_Load_queue_mutex;
	std::deque<mesh_load_future> mesh_Load_queue;
	
	static MeshManager* instance;
};

NonIntrusiveRuntimeTag(std::shared_ptr<Mesh>, "Mesh")
