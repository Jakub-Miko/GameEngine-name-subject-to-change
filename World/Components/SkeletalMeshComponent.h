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
	SkeletalMeshComponent() : file_path("Unknown"), mesh(nullptr), animation_plaback(), default_animation_path("") {
		mesh = MeshManager::Get()->GetDefaultSkeletalMesh();
		compare_status = Mesh_status::READY;
	}
	
	SkeletalMeshComponent(const std::string& filepath,const std::string& default_anim_path = "", int index = 0) : file_path("Unknown"), mesh(nullptr), animation_plaback(), default_animation_path(default_anim_path){
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		if (!mesh->IsSkeletal()) {
			throw std::runtime_error("Skeletal mesh needs to be used in skeletal mesh component");
		}
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
		compare_status = mesh->GetMeshStatus();
		if (!default_animation_path.empty()) {
			SetDefaultAnimationPath(default_anim_path);
		}
	}

	SkeletalMeshComponent(const SkeletalMeshComponent& other) : file_path(other.file_path), mesh(other.mesh), animation_plaback(), default_animation_path(other.default_animation_path), material(other.material), compare_status(other.compare_status) {
		if (!default_animation_path.empty()) {
			SetDefaultAnimationPath(default_animation_path);
		}
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
		compare_status = Mesh_status::READY;
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

	std::string GetDefaultAnimationPath() const {
		return default_animation_path;
	}

	AnimationPlayback& GetAnimation() {
		return animation_plaback;
	}

	void SetDefaultAnimationPath(const std::string& default_anim_path, bool force = false) {
		if (default_anim_path.empty()) {
			default_animation_path = "";
			SetAnimation(AnimationManager::Get()->GetDefaultAnimation());
			return;
		}
		default_animation_path = default_anim_path;
		if (!animation_plaback.IsValidAnim() || force) {
			SetAnimation(AnimationPlayback(AnimationManager::Get()->LoadAnimationAsync(FileManager::Get()->GetPath(default_anim_path))));
		}
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
		if (!import_mesh->IsSkeletal()) {
#ifdef EDITOR
			Editor::Get()->EditorError("Skeletal mesh needs to be used in skeletal mesh component");
#endif
			return;
		}
		mesh = import_mesh;
		compare_status = mesh->GetMeshStatus();
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	friend class MeshManager;
	friend class World;
	friend inline void to_json(nlohmann::json& j, const SkeletalMeshComponent& p);
	friend inline void from_json(const nlohmann::json& j, SkeletalMeshComponent& p);
	std::string file_path;
	std::string default_animation_path;
	std::shared_ptr<Mesh> mesh;
	//to check if the resource transitioned into a loaded state and act accordingly
	Mesh_status compare_status;
	AnimationPlayback animation_plaback;
};

template<>
class ComponentInitProxy<SkeletalMeshComponent> {
public:
	static constexpr bool can_copy = true;

};

#pragma region Json_Serialization

inline void to_json(nlohmann::json& j, const SkeletalMeshComponent& p) {
	j["path"] = p.file_path;
	if (p.material != nullptr) {
		j["material_path"] = p.GetMaterialPath();
	}
	if (!p.default_animation_path.empty()) {
		j["animation_path"] = p.default_animation_path;
	}

}

inline void from_json(const nlohmann::json& j, SkeletalMeshComponent& p) {
	p.ChangeMesh(FileManager::Get()->GetPath(j["path"].get<std::string>()));
	if (j.contains("material_path")) {
		auto material_path = j["material_path"].get<std::string>();
		p.ChangeMaterial(j["material_path"].get<std::string>());
	}
	if (j.contains("animation_path")) {
		auto material_path = j["animation_path"].get<std::string>();
		p.SetDefaultAnimationPath(j["animation_path"].get<std::string>());
	}
}

#pragma endregion
