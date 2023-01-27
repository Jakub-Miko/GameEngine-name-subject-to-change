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

template<typename T>
class ComponentInitProxy;

class MeshComponent {
	RUNTIME_TAG("MeshComponent")
public:
	MeshComponent() : file_path("Unknown"), mesh(nullptr) {
		mesh = MeshManager::Get()->GetDefaultMesh();
		compare_status = Mesh_status::READY;
	}
	
	MeshComponent(const MeshComponent& other) : compare_status(other.compare_status), mesh(other.mesh), file_path(other.file_path), material(other.material), visible(other.visible) {}

	MeshComponent(const std::string& filepath,int index = 0) : file_path("Unknown"), mesh(nullptr){
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		if (mesh->IsSkeletal()) {
			throw std::runtime_error("Cant use skeletal mesh as static mesh");
		}
		compare_status = mesh->GetMeshStatus();
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
		compare_status = Mesh_status::READY;
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

	bool GetVisibility() const {
		return visible;
	}

	void SetVisibility(bool visible) {
		this->visible = visible;
	}

	std::shared_ptr<Material> material = nullptr;
private:
	//Check if the mesh object has changed since last time this component was used
	bool UpdateState() {
		if (compare_status != mesh->GetMeshStatus()) {
			compare_status = mesh->GetMeshStatus();
			return true;
		} 
		return false;
	}
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
		compare_status = mesh->GetMeshStatus();
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	friend class MeshManager;
	friend class World;
	friend inline void to_json(nlohmann::json& j, const MeshComponent& p);
	friend inline void from_json(const nlohmann::json& j, MeshComponent& p);
	std::string file_path;
	std::shared_ptr<Mesh> mesh;
	//to check if the resource transitioned into a loaded state and act accordingly
	Mesh_status compare_status = Mesh_status::UNINITIALIZED;;
	bool visible = true;
};

template<>
class ComponentInitProxy<MeshComponent> {
public:
	static constexpr bool can_copy = true;

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
	j["visible"] = p.visible;
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
	if (j.contains("visible")) {
		p.visible = j["visible"].get<bool>();
	}
}

#pragma endregion
