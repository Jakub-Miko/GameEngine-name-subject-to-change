#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/MaterialManager.h>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>
#include <FileManager.h>
#include <Renderer/Renderer3D/Animations/Animation.h>
#include <Renderer/Renderer3D/Animations/AnimationManager.h>
#ifdef EDITOR
#include <Editor/Editor.h>
#endif

class SkeletalMeshComponent {
	RUNTIME_TAG("SkeletalMeshComponent")
public:
	SkeletalMeshComponent() : file_path("Unknown"), mesh(nullptr), animation_plaback() {
		mesh = MeshManager::Get()->GetDefaultSkeletalMesh();
	}
	
	SkeletalMeshComponent(const std::string& filepath,int index = 0) : file_path("Unknown"), mesh(nullptr), animation_plaback() {
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		if (!mesh->IsSkeletal()) {
			throw std::runtime_error("Skeletal mesh needs to be used in skeletal mesh component");
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

	AnimationPlayback& GetAnimation() {
		return animation_plaback;
	}

	void SetAnimation(const AnimationPlayback& animation) {
		animation_plaback = animation;
	}

	void SetAnimationTime(float time) {
		animation_plaback.SetTime(time);
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
		if (!import_mesh->IsSkeletal()) {
#ifdef EDITOR
			Editor::Get()->EditorError("Skeletal mesh needs to be used in skeletal mesh component");
#endif
			return;
		}
		mesh = import_mesh;
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	friend class MeshManager;
	friend class World;
	friend inline void to_json(nlohmann::json& j, const SkeletalMeshComponent& p);
	friend inline void from_json(const nlohmann::json& j, SkeletalMeshComponent& p);
	std::string file_path;
	std::shared_ptr<Mesh> mesh;
	AnimationPlayback animation_plaback;
};

#pragma region Json_Serialization

inline void to_json(nlohmann::json& j, const SkeletalMeshComponent& p) {
	j["path"] = p.file_path;
	if (p.material != nullptr) {
		j["material_path"] = p.GetMaterialPath();
	}

}

inline void from_json(const nlohmann::json& j, SkeletalMeshComponent& p) {
	p.ChangeMesh(FileManager::Get()->GetPath(j["path"].get<std::string>()));
	if (j.contains("material_path")) {
		auto material_path = j["material_path"].get<std::string>();
		p.ChangeMaterial(j["material_path"].get<std::string>());
	}
}

#pragma endregion
