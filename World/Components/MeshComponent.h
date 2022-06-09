#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>

class MeshComponent {
	RuntimeTag("MeshComponent")
public:
	MeshComponent(std::string filepath,int index = 0) : file_path(filepath), mesh(nullptr){
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
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

