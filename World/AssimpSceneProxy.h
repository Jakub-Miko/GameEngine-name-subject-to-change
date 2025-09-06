#pragma once
#include <World/SceneProxy.h>
#include <Renderer/MeshManager.h>

class aiNode;
class aiScene;
class aiMesh;

class AssimpSceneProxy : public SceneProxy {
public:
    AssimpSceneProxy(const std::string path) : path(path) {}

    virtual LoadInfo LoadScene(World& world) override;
	virtual const std::string& GetFilePath() const override {
        return path;
    }
	virtual ~AssimpSceneProxy() {}

private:

    struct LoadState {
        const aiScene* scene;
        World& world;
        std::vector<std::shared_ptr<Mesh>> meshes;
    };
    void LoadMeshes(LoadState& state);
    void ProcessNode(LoadState& state, aiNode* node, aiNode* parent, Entity parent_entity);

    std::shared_ptr<Mesh> LoadMesh(aiMesh* mesh);

    std::string path;
};