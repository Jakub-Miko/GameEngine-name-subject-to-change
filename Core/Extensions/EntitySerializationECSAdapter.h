#pragma once
#include <json.hpp>
#include <World/World.h>
#include <fstream>
#include <stdexcept>

#include "World/Components/SerializableComponent.h"
#include "World/Components/PrefabComponent.h"

/**
 * @brief Used to signalize that a component section in the serialized file corresponding to this component should be skipped while loading.
 */
struct NullComponentType {};

template<typename T>
struct FilterNullComponents {
    static constexpr bool value = !std::is_same_v<T, NullComponentType>;
};

class ECS_Output_Archive {

public:
    ECS_Output_Archive() {
        root = nlohmann::json::array();
    };

    void operator()(std::underlying_type_t<entt::entity> size) {
        if(skip_indices[current_idx] != 0) {
            for(int i = 0; i < skip_indices[current_idx]; ++i) {
                auto array = nlohmann::json::array();
                array.push_back(0); // insert empty entries
                root.push_back(array);
                current_idx++;
            }
        }
        current_idx++;
        int a = 0;
        if (!current.empty()) {
            root.push_back(current);
        }
        current = nlohmann::json::array();
        current.push_back(size);
    }


    void operator()(entt::entity entity) {
        current.push_back((uint32_t)entity);
    }

    template<typename ViewType, typename ... Args>
    void Serialize(World& world, ViewType view, TypeList<Args...>) {
        InitSkipList<Args...>();
        Serialize_impl(world, view, typename TypeFilter<FilterNullComponents, TypeList<Args...>>::type());
    }

    template<typename T>
    void operator()(entt::entity ent, const T& t) {
        current.push_back((uint32_t)ent);
        nlohmann::json json = t;
        current.push_back(json);
    }

    const std::string AsString() {
        if (!current.empty()) {
            root.push_back(current);
        }
        std::string output = root.dump();
        return output;
    }

    const nlohmann::json& AsJson() {
        if (!current.empty()) {
            root.push_back(current);
        }
        return root;
    }

    void SaveAsFile(const std::string filepath) {
        if (!current.empty()) {
            root.push_back(current);
        }
        std::ofstream stream(FileManager::Get()->GetAssetFilePath(filepath));
        if (!stream.is_open()) {
            throw std::runtime_error("File could not be opened: " + filepath);
        }

        stream << root;

        stream.close();
    }

private:
    template<typename ViewType, typename ... Args>
    void Serialize_impl(World& world, ViewType view, TypeList<Args...>) {
        InitSkipList<Args...>();
        entt::snapshot snapshot(world.GetRegistry());
        snapshot.component<Args...>(*this, view.begin(), view.end());
    }

    template<typename ... Args>
    void InitSkipList() {
        skip_indices = std::vector<uint32_t>(sizeof...(Args) + 1, 0);
        InitSkipList_recursive<Args...>(); // mark the null types
        int counter = 0;
        for(int i = sizeof...(Args) - 1; i >= 0; --i) { // reverse prefix sum
            if(skip_indices[i] == 0) {
                counter = 0;
                continue;
            }
            counter++;
            skip_indices[i] = counter;
        }
    }

    template<typename T, typename ... Args>
    void InitSkipList_recursive(int index = 0) {
        skip_indices[index] = std::is_same_v<T, NullComponentType> ? 1 : 0;
        if constexpr (sizeof...(Args) > 0) {
            InitSkipList_recursive<Args...>(index);
        }
    }

    nlohmann::json root;
    nlohmann::json current;
    std::vector<uint32_t> skip_indices;
    int current_idx = 0;
};


class ECS_Input_Archive {
private:
    nlohmann::json root;
    nlohmann::json current;

    uint32_t root_idx = -1;
    uint32_t current_idx = 0;
    std::vector<uint32_t> skip_indices;
public:

    ECS_Input_Archive(const std::string& json_string)
    {
        root = nlohmann::json::parse(json_string);
    };

    ECS_Input_Archive(const nlohmann::json& json)
    {
        root = json;
    };

    ECS_Input_Archive() : root()
    {

    };

    ~ECS_Input_Archive() {

    }

    template<typename ... Args>
    void Deserialize(World& world, TypeList<Args...>) {
        InitSkipList<Args...>();
        Deserialize_impl(world, typename TypeFilter<FilterNullComponents, TypeList<Args...>>::type());
    }


    void LoadFromFile(const std::string& path) {
        std::ifstream stream(FileManager::Get()->GetAssetFilePath(path));
        if (!stream.is_open()) {
            throw std::runtime_error("File could not be opened: " + path);
        }
        stream >> root;
        stream.close();
    }

    bool next_root() {
        root_idx ++;
        root_idx += skip_indices[root_idx]; // skip null entries
        if (root_idx >= root.size()) {
            return false;
        }
        current = root[root_idx];
        current_idx = 0;
        return true;
    }

    void operator()(std::underlying_type_t<entt::entity>& s) {
        if(!next_root()) {
            return;
        }
        int size = current[0].get<int>();
        current_idx++;
        s = (std::underlying_type_t<entt::entity>)size; 
    }

    void operator()(entt::entity& entity) {
        uint32_t ent = current[current_idx].get<uint32_t>();
        entity = entt::entity(ent);
        current_idx++;
    }

    template<typename T>
    void operator()(entt::entity& ent, T& t) {
        nlohmann::json component_data = current[current_idx * 2];

        auto comp = component_data.get<T>();
        t = comp;

        uint32_t _ent = current[current_idx * 2 - 1];
        ent = entt::entity(_ent);
        current_idx++;
    }

private:
    template<typename ... Args>
    void Deserialize_impl(World& world, TypeList<Args...>) {
        entt::snapshot_loader loader(world.GetRegistry());
        loader.component<Args...>(*this);
    }

    template<typename ... Args>
    void InitSkipList() {
        skip_indices = std::vector<uint32_t>(sizeof...(Args) + 1, 0);
        InitSkipList_recursive<Args...>(); // mark the null types
        int counter = 0;
        for(int i = sizeof...(Args) - 1; i >= 0; --i) { // reverse prefix sum
            if(skip_indices[i] == 0) {
                counter = 0;
                continue;
            }
            counter++;
            skip_indices[i] = counter;
        }
    }

    template<typename T, typename ... Args>
    void InitSkipList_recursive(int index = 0) {
        skip_indices[index] = std::is_same_v<T, NullComponentType> ? 1 : 0;
        if constexpr (sizeof...(Args) > 0) {
            InitSkipList_recursive<Args...>(index + 1);
        }
    }
};
