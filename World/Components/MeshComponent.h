#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>
#include <FileManager.h>

class MeshComponent {
	RuntimeTag("MeshComponent")
public:
	MeshComponent() : file_path("Unknown"), mesh(nullptr) {
		mesh = MeshManager::Get()->GetDefaultMesh();
	}
	
	MeshComponent(const std::string& filepath,int index = 0) : file_path("Unknown"), mesh(nullptr){
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	void ChangeMesh(const std::string& filepath) {
		auto import_mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		mesh = import_mesh;
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	std::string file_path;
	std::shared_ptr<Mesh> mesh;
};

class LoadingMeshComponent {
	RuntimeTag("LoadingMeshComponent")
public:
	LoadingMeshComponent(const Future<std::shared_ptr<Mesh>>& mesh_future) : mesh_future(mesh_future) {}

	Future<std::shared_ptr<Mesh>> mesh_future;
};


#pragma region Json_Serialization

inline void to_json(nlohmann::json& j, const MeshComponent& p) {
	j["path"] = p.file_path;

}

inline void from_json(const nlohmann::json& j, MeshComponent& p) {
	p.ChangeMesh(FileManager::Get()->GetPath(j["path"].get<std::string>()));
}

#pragma endregion
