#pragma once
#include <World/SceneProxy.h>
#include <Renderer/MeshManager.h>
#include <Renderer/MaterialManager.h>

#include "FileManager.h"

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMetadata;
struct aiMetadataEntry;
namespace Assimp {
    class Importer;
}

class AssimpSceneProxy : public SceneProxy {
public:
    AssimpSceneProxy(const std::string& path, float scale_factor = 1.0f, const std::string& scene_save_path = "") : path(path), scale_factor(scale_factor), scene_save_path(scene_save_path) {}

    virtual LoadInfo LoadScene(World& world) override;
	virtual const std::string& GetFilePath() const override {
        return path;
    }

    virtual const std::string& GetNativeFilePath() const override {
	    return scene_save_path;
	};
	virtual ~AssimpSceneProxy() {}

    /**
     * @brief Handle for all other proxies created while Loading the scene to ensure all of them don't have to load the assimp file again.
     */
    struct AssimpOpenScene {
        Assimp::Importer* importer;
        const aiScene* scene;
        ~AssimpOpenScene();
    };

private:

    struct LoadState {
        struct imported_entity {
            Entity entity;
            aiNode* entity_node;
        };
        World& world;
        std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene;
        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<Material>> materials;
        std::unordered_map<std::string, imported_entity> name_map;
        aiMetadata* gltf_light_meta = nullptr;
        std::string scene_resource_directory = "";
        bool serialize; // Whether to serialize imported resources into a native format;
    };
    void LoadMeshes(LoadState& state);
    void LoadLights(LoadState& state);
    void LoadCameras(LoadState& state);
    void LoadAnimations(LoadState& state);
    void ProcessNode(LoadState& state, aiNode* node, aiNode* parent, Entity parent_entity);
    void LoadMaterials(LoadState& state);

    void Inspect_Metadata(AssimpSceneProxy::LoadState& state, aiMetadata* data, int level = 0);
    aiMetadataEntry* GetNestedMetadata(const std::vector<std::string>& path, aiMetadata* parent);
    void ProbeSceneMetadata(LoadState &state);

    std::string path;
    std::string scene_save_path;
    float scale_factor = 1.0f;
};

class AssimpMeshProxy : public MeshProxy {
public:
    AssimpMeshProxy(const std::string& path, uint32_t mesh_index, std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene, std::string native_path = "")
        : path(path), mesh_index(mesh_index), open_scene(open_scene), native_file_path(native_path) {}
    virtual MeshSourceData LoadMesh() override;
    virtual const std::string& GetFilePath() override {
        return path;
    }
    virtual const std::string& GetNativeFilePath() override;
	virtual bool IsSkeletal() override;
	virtual ~AssimpMeshProxy() {}
public:
    std::string path;
    std::string native_file_path = "";
    uint32_t mesh_index;
    std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene = nullptr; ///< if not nullptr this scene is used to load the mesh instead of reopening it
};

class AssimpMaterialProxy : public MaterialProxy, public std::enable_shared_from_this<AssimpMaterialProxy> {
public:
    AssimpMaterialProxy(const std::string& path, uint32_t material_index, std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene, std::string native_path = "")
        : path(path), material_index(material_index), open_scene(open_scene), native_file_path(native_path) {}

    virtual std::shared_ptr<Material> LoadMaterial();

    virtual const std::string& GetFilePath() override {
        return path;
    }

    virtual const std::string& GetNativeFilePath() override;

    virtual ~AssimpMaterialProxy() {}
public:
    std::string path;
    std::string native_file_path;
    uint32_t material_index;
    std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene = nullptr; ///< if not nullptr this scene is used to load the mesh instead of reopening it
};