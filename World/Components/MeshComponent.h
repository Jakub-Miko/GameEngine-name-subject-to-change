#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/MaterialManager.h>
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

	void ChangeMaterial(const std::string& filepath) {
		if (filepath.empty()) {
			material.reset();
			return;
		}
		material = MaterialManager::Get()->GetMaterial(filepath);
	}

	std::string GetMaterialPath() const {
		if (material) {
			return material->GetFilePath();
		}
		else {
			return "";
		}
	}

	Material::Material_status GetMaterialStatus() {
		if (material) {
			return material->GetStatus();
		}
		else {
			return Material::Material_status::UNINITIALIZED;
		}
	}

	std::string file_path;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material = nullptr;
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
	if (p.material != nullptr) {
		j["material_path"] = p.GetMaterialPath();
	}

}

inline void from_json(const nlohmann::json& j, MeshComponent& p) {
	p.ChangeMesh(FileManager::Get()->GetPath(j["path"].get<std::string>()));
	if (j.contains("material_path")) {
		auto material_path = j["material_path"].get<std::string>();
		p.ChangeMaterial(j["material_path"].get<std::string>());
	}
}

#pragma endregion
