#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>
#include <FileManager.h>
#ifdef EDITOR
#include <Editor/Editor.h>
#endif

class MeshComponent {
	RUNTIME_TAG("MeshComponent")
public:
	MeshComponent() : file_path("Unknown"), mesh(nullptr) {
		mesh = MeshManager::Get()->GetDefaultMesh();
	}
	
	MeshComponent(const std::string& filepath,int index = 0) : file_path("Unknown"), mesh(nullptr){
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		if (mesh->IsSkeletal()) {
			throw std::runtime_error("Cant use skeletal mesh as static mesh");
		}
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	void ChangeMaterial(const std::string& filepath) {
		if (filepath.empty()) {
			material.reset();
			return;
		}
		material = MaterialManager::Get()->GetMaterial(filepath);
	}

	void ResetMesh() {
		mesh = MeshManager::Get()->GetDefaultMesh();
		file_path = "Unknown";
	}

	std::shared_ptr<Mesh> GetMesh() const {
		return mesh;
	}

	const std::string& GetMeshPath() const {
		return file_path;
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

	std::shared_ptr<Material> material = nullptr;
private:
	//This should only be called from World class since other components can rely on it.
	void ChangeMesh(const std::string& filepath) {
		auto import_mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		if (import_mesh->IsSkeletal()) {
#ifdef EDITOR
			Editor::Get()->EditorError("Skeletal mesh cant be used in mesh component");
#endif
			return;
		}
		mesh = import_mesh;
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	friend class MeshManager;
	friend class World;
	friend inline void to_json(nlohmann::json& j, const MeshComponent& p);
	friend inline void from_json(const nlohmann::json& j, MeshComponent& p);
	std::string file_path;
	std::shared_ptr<Mesh> mesh;
};

class LoadingMeshComponent {
	RUNTIME_TAG("LoadingMeshComponent")
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
