#pragma once
#include <memory>
#include <Renderer/MeshManager.h>
#include <Core/UnitConverter.h>
#include <Core/RuntimeTag.h>
#include <FileManager.h>

class MeshComponent {
	RuntimeTag("MeshComponent")
public:
	MeshComponent(const std::string& filepath,int index = 0) : file_path("Unknown"), mesh(nullptr){
		mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
	}

	void ChangeMesh(const std::string& filepath) {
		auto import_mesh = MeshManager::Get()->LoadMeshFromFileAsync(filepath);
		mesh = import_mesh;
		file_path = FileManager::Get()->GetRelativeFilePath(filepath);
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

