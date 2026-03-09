#pragma once
#include <string>
#include <Core/RuntimeTag.h>
#include <Core/UnitConverter.h>

class World;

template <typename T>
class ComponentInitProxy;

enum class LightType : char {
    DIRECTIONAL = 0, POINT = 1, SPOTLIGHT = 2
};

class LightComponent;

template <>
class ComponentInitProxy<LightComponent> {
public:
    static void OnCreate(World& world, Entity entity);

    static void OnDestroy(World& world, Entity entity);

    static constexpr bool can_copy = true;
};


class LightComponent {
    RUNTIME_TAG("LightComponent")

public:
    LightComponent(LightType type = LightType::DIRECTIONAL,
                   const glm::vec4& color = glm::vec4(1.0f)) : color(color), type(type) {}

    LightComponent(float range, const glm::vec4& color = glm::vec4(1.0f)) : color(color), type(LightType::POINT),
                                                                            range(range) {}

    LightComponent(const LightComponent& other) : color(other.color), range(other.range), type(other.type) {}
    static void ChangeType(LightType type, Entity ent);
    static void SetRange(float new_range, Entity ent);
    static void SetLightColor(glm::vec4 color, Entity ent);

    float GetLightRange() const {
        return range;
    }

    glm::vec4 GetLightColor() const {
        return color;
    }

    LightType type;

private:
    glm::vec4 color;
    float range = 10.0f;

    friend void to_json(nlohmann::json& nlohmann_json_j, const LightComponent& nlohmann_json_t) {
        nlohmann_json_j["color"] = nlohmann_json_t.color;
        nlohmann_json_j["type"] = nlohmann_json_t.type;
        nlohmann_json_j["range"] = nlohmann_json_t.range;
    }

    friend void from_json(const nlohmann::json& nlohmann_json_j, LightComponent& nlohmann_json_t) {
        nlohmann_json_j.at("color").get_to(nlohmann_json_t.color);
        nlohmann_json_j.at("type").get_to(nlohmann_json_t.type);
        if(nlohmann_json_j.contains("range")) {
            nlohmann_json_j.at("range").get_to(nlohmann_json_t.range);
        } else {
            nlohmann_json_t.range = std::sqrt(nlohmann_json_t.color.w / 0.005);
        }
    }
};
