#pragma once
#include <json.hpp>
#include <World/World.h>
#include <fstream>
#include <stdexcept>

class ECS_Output_Archive {
public:
    ECS_Output_Archive() {
        root = nlohmann::json::array();
    };

    void operator()(std::underlying_type_t<entt::entity> size) {
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

private:
    nlohmann::json root;
    nlohmann::json current;
};

class ECS_Input_Archive {
private:
    nlohmann::json root;
    nlohmann::json current;

    int root_idx = -1;
    int current_idx = 0;

public:
    ECS_Input_Archive(const std::string& json_string)
    {
        root = nlohmann::json::parse(json_string);
    };

    ECS_Input_Archive(const std::string& path) : root()
    {
        std::ifstream stream(path);
        if (!stream.is_open()) {
            throw std::runtime_error("File could not be opened: " + path);
        }
        root << stream;
        stream.close();
    };

    ~ECS_Input_Archive() {

    }

    void next_root() {
        root_idx++;
        if (root_idx >= root.size()) {
            throw std::runtime_error("File out of range");
        }
        current = root[root_idx];
        current_idx = 0;
    }

    void operator()(std::underlying_type_t<entt::entity>& s) {
        next_root();
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
};
