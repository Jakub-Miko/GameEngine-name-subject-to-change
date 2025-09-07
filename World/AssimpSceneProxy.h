#pragma once
#include <World/SceneProxy.h>
#include <Renderer/MeshManager.h>
#include <Renderer/MaterialManager.h>

class aiNode;
class aiScene;
class aiMesh;
class aiMetadata;
class aiMetadataEntry;
namespace Assimp {
    class Importer;
}

class AssimpSceneProxy : public SceneProxy {
public:
    AssimpSceneProxy(const std::string& path, float scale_factor = 1.0f) : path(path), scale_factor(scale_factor) {}

    virtual LoadInfo LoadScene(World& world) override;
	virtual const std::string& GetFilePath() const override {
        return path;
    }
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
        std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene;
        World& world;
        std::vector<std::shared_ptr<Mesh>> meshes;
        std::vector<std::shared_ptr<Material>> materials;
        std::unordered_map<std::string, imported_entity> name_map; 
        aiMetadata* gltf_light_meta = nullptr;
    };
    void LoadMeshes(LoadState& state);
    void LoadLights(LoadState& state);
    void ProcessNode(LoadState& state, aiNode* node, aiNode* parent, Entity parent_entity);
    void LoadMaterials(LoadState& state);

    void Inspect_Metadata(AssimpSceneProxy::LoadState& state, aiMetadata* data, int level = 0);
    aiMetadataEntry* GetNestedMetadata(const std::vector<std::string>& path, aiMetadata* parent);
    void ProbeSceneMetadata(LoadState &state);

    std::string path;
    float scale_factor = 1.0f;
};

class AssimpMeshProxy : public MeshProxy {
public:
    AssimpMeshProxy(const std::string& path, uint32_t mesh_index, std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene = nullptr)
        : path(path), mesh_index(mesh_index), open_scene(open_scene) {}
    virtual MeshSourceData LoadMesh() override;
	virtual bool IsSkeletal() override;
	virtual ~AssimpMeshProxy() {}
public:
    std::string path;
    uint32_t mesh_index;
    std::shared_ptr<AssimpSceneProxy::AssimpOpenScene> open_scene = nullptr; ///< if not nullptr this scene is used to load the mesh instead of reopening it
};