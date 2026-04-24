#include "SceneProxy.h"
#include <json.hpp>
#include <fstream>
#include <stdexcept>
#include <FileManager.h>
#include <ConfigManager.h>
#include <World/World.h>
#include <Core/Extensions/EntitySerializationECSAdapter.h>
#include <World/ComponentTypes.h>

NativeSceneProxy::NativeSceneProxy()
{
	path = ConfigManager::Get()->GetString("default_scene");
}

SceneProxy::LoadInfo NativeSceneProxy::LoadScene(World &world)
{
	SectionList sections;
	std::string file = FileManager::Get()->OpenFileRaw(FileManager::Get()->GetPath(path), &sections);

	LoadInfo load_info = {};

	if (sections.find("Script") != sections.end())
	{
		load_info.script = FileManager::Get()->GetFileSectionFromString(file, "Script");
		load_info.has_script = true;
	}
	else
	{
		load_info.has_script = false;
	}

	if (sections.size() != 0)
	{
		file = FileManager::Get()->GetFileSection(FileManager::Get()->GetPath(path), "Root");
	}

	nlohmann::json json = nlohmann::json::parse(file);

	Entity primary;
	if (json.find("primary_entity") != json.end())
	{
		primary = json["primary_entity"].get<Entity>();
	}

	auto typelist = TypeList<TransformComponent, PrefabComponent, DynamicPropertiesComponent, LabelComponent, MeshComponent, CameraComponent,
	LightComponent, ShadowCasterComponent, NullComponentType, SkeletalMeshComponent, NullComponentType, UITextComponent, SkylightComponent, AnimationComponent>();


	ECS_Input_Archive archive(json["Entities"]);
	archive.Deserialize(world, typelist);

	world.GetSceneGraph()->Deserialize(json);

	world.SetPrimaryEntity(primary);

	return load_info;
}
