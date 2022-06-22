#pragma once
#include <string>
#include <cstring>
#include <unordered_map>
#include <Renderer/RendererDefines.h>
#include <Renderer/RenderResource.h>
#include <AsyncTaskDispatcher.h>
#include <mutex>
#include <memory>

enum class Mesh_status : char {
	UNINITIALIZED = 0, LOADING = 1, READY = 2, ERROR = 3
};

class MeshManager;

class Mesh {
public:
	friend MeshManager;
	Mesh(const std::shared_ptr<RenderBufferResource>& vertex_buffer, const std::shared_ptr<RenderBufferResource>& index_buffer, size_t num_of_indicies)
		: num_of_indicies(num_of_indicies), vertex_buffer(vertex_buffer), index_buffer(index_buffer) {}
	Mesh() = default;

	Mesh(const Mesh& other) = default;

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

private:



	size_t num_of_indicies = 0;
	Mesh_status status = Mesh_status::UNINITIALIZED;
	std::shared_ptr<RenderBufferResource> vertex_buffer = nullptr;
	std::shared_ptr<RenderBufferResource> index_buffer = nullptr;
};

class MeshManager {
public:

	MeshManager(const MeshManager& ref) = delete;
	MeshManager(MeshManager&& ref) = delete;
	MeshManager& operator=(const MeshManager& ref) = delete;
	MeshManager& operator=(MeshManager&& ref) = delete;	

	void MakeMeshFromObjectFile(const std::string& in_file_path, const std::string& out_file_path,const VertexLayout& layout, int mesh_index = 0) {
		mesh_vertex_props props;
		props.has_normal = layout.has_normal();
		props.has_position= layout.has_position();
		props.num_of_uv_channels = layout.GetUvCount();
		mesh_assimp_input_data input_data = Fetch_Assimp_Data(props, in_file_path, mesh_index);

		size_t num_vertecies = input_data.num_of_verticies;
		char* vertex_buffer = new char[num_vertecies * layout.stride];
		auto pos_element = layout.GetElement("position");
		auto normal_element = layout.GetElement("normal");
		VertexLayoutElement* uv_elements = new VertexLayoutElement[props.num_of_uv_channels];
		for (int i = 0; i < props.num_of_uv_channels; i++) {
			uv_elements[i] = layout.GetElement("uv" + std::to_string(i));
		}

		if (props.has_position) {
			switch (pos_element.size) {
			case 3:
				for (int i = 0; i < input_data.num_of_verticies; i++) {
					glm::vec3 pos_data = input_data.position[i];
					void* data = (void*)(vertex_buffer +((layout.stride * i) + pos_element.offset));
					std::memcpy(data, &pos_data, sizeof(glm::vec3));
				}
				break;
			case 4:
				for (int i = 0; i < input_data.num_of_verticies; i++) {
					glm::vec4 pos_data = glm::vec4(input_data.position[i],1.0f);
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

		for (int x = 0; x < props.num_of_uv_channels; x++) {
			for (int i = 0; i < input_data.num_of_verticies; i++) {
				glm::vec2 uv_data = input_data.uvs[x][i];
				void* data = (void*)(vertex_buffer + ((layout.stride * i) + uv_elements[x].offset));
				std::memcpy(data,&uv_data, sizeof(glm::vec3));
			}
		}


		Write_assimp_processed_data((void*)vertex_buffer, layout.stride, input_data, out_file_path, layout);
		input_data.clear();
		delete[] vertex_buffer;
		delete[] uv_elements;
	}

	std::shared_ptr<Mesh> LoadMeshFromFileAsync(const std::string& file_path);
	
	std::shared_ptr<Mesh> GetDefaultMesh() const {
		return default_mesh;
	}

	//Only call when meshes are actively being used
	void UpdateLoadedMeshes();

	static void Init();
	
	static MeshManager* Get();

	static void Shutdown();
private:
	MeshManager();

	struct mesh_vertex_props {
		uint32_t num_of_uv_channels = 0;
		bool has_position = true;
		bool has_normal = true;
	};

	struct mesh_assimp_input_data {
		glm::vec3* position = nullptr;
		glm::vec3* normal = nullptr;
		glm::vec3** uvs = nullptr;
		unsigned int *indicies = nullptr;
		int num_of_indicies;
		uint32_t num_of_verticies = 0;
		uint32_t num_of_uv_channels = 0;
		void* importer = nullptr;

		void clear();

	};

	
	struct mesh_native_input_data {
		void* vertex_buffer = nullptr;
		uint32_t vertex_count = 0;
		unsigned int* index_buffer = nullptr;
		uint32_t index_count = 0;
		int vertex_size = 0;
		VertexLayout layout;

		void clear();
	};

	struct mesh_load_future {
		std::shared_ptr<Mesh> mesh;
		Future<Mesh> future;
		bool destroyed = false;
		std::string path;
	};


	Mesh LoadMeshFromFileImpl(const std::string& file_path);

	std::shared_ptr<Mesh> RegisterMesh(std::shared_ptr<Mesh> file_path, const std::string& name);

	mesh_native_input_data Fetch_Native_Data(const std::string& in_file_path);

	mesh_assimp_input_data Fetch_Assimp_Data(const mesh_vertex_props& props, const std::string& in_file_path, int mesh_index = 0);

	void Write_assimp_processed_data(void* vertex_data, size_t vertex_size, const mesh_assimp_input_data& import_data,const std::string& out_data, const VertexLayout& layout);

	std::shared_ptr<Mesh> default_mesh;
	std::mutex mesh_Map_mutex;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_Map;
	std::mutex mesh_Load_queue_mutex;
	std::deque<mesh_load_future> mesh_Load_queue;
	
	static MeshManager* instance;
};
