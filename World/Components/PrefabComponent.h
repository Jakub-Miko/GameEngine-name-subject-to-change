#pragma once 
#include <Core/RuntimeTag.h>
#include <World/Entity.h>
#include <FileManager.h>

enum class PrefabStatus : char {
	OK = 0, ERROR = 1, UNINITIALIZED = 2
};

template<typename T>
class ComponentInitProxy;

struct PrefabComponent {
	RUNTIME_TAG("PrefabComponent");
	PrefabComponent(const std::string& file_path = "Undefined", Entity first_child = Entity()) : file_path(file_path) {}
	Entity first_child = Entity();
	PrefabStatus status = PrefabStatus::OK;

	PrefabComponent(const PrefabComponent& other) : file_path(other.file_path) {}

	const std::string& GetFilePath() const {
		return file_path;
	}

private:
	friend class World;
	friend void from_json(const nlohmann::json& nlohmann_json_j, PrefabComponent& nlohmann_json_t);
	std::string file_path;
};

template<>
class ComponentInitProxy<PrefabComponent> {
public:
	static constexpr bool can_copy = true;

	static void OnCreate(World& world, Entity entity);

	static void OnDestroy(World& world, Entity entity);

};

inline void to_json(nlohmann::json& nlohmann_json_j, const PrefabComponent& nlohmann_json_t) { 
	nlohmann_json_j["file_path"] = nlohmann_json_t.GetFilePath();
}

inline void from_json(const nlohmann::json& nlohmann_json_j, PrefabComponent& nlohmann_json_t) { 
	nlohmann_json_t.file_path = (nlohmann_json_j["file_path"].get<std::string>());
}