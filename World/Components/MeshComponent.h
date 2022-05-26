#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>

class MeshComponent {
	RuntimeTag("MeshComponent")
public:
	MeshComponent(std::string filepath,int index = 0) : file_path(filepath), index(index), mesh(nullptr){}

	std::string file_path;
	std::shared_ptr<Mesh> mesh;
	Mesh_status status = Mesh_status::UNINITIALIZED;
	int index = 0;
};

class LoadingMeshComponent {
	RuntimeTag("LoadingMeshComponent")
public:
	LoadingMeshComponent(const Future<std::shared_ptr<Mesh>>& mesh_future) : mesh_future(mesh_future) {}

	Future<std::shared_ptr<Mesh>> mesh_future;
};


template<>
class ComponentInitProxy<MeshComponent> {
public:

	static void OnCreate(World& world, Entity entity) {
		auto& mesh = world.GetComponent<MeshComponent>(entity);
		mesh.mesh = MeshManager::Get()->GetDefaultMesh();
		mesh.status = Mesh_status::LOADING;
		world.SetComponent<LoadingMeshComponent>(entity, MeshManager::Get()->LoadMeshFromFileAsync(mesh.file_path, mesh.index));
	}

};