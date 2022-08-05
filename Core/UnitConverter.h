#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <json.hpp>
#include <Renderer/RendererDefines.h>
#include <variant>
#define JSON_SERIALIZABLE(Type, ...) NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Type, __VA_ARGS__) 

//Needed for serializing glm object to json
namespace nlohmann {
    template <>
    struct adl_serializer<glm::vec3> {
        static void to_json(json& j, const glm::vec3& vec) {
			j = json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
        }

        static void from_json(const json& j, glm::vec3& vec) {
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
			j.at("z").get_to(vec.z);
        }
    };

	template <>
	struct adl_serializer<glm::vec2> {
		static void to_json(json& j, const glm::vec2& vec) {
			j = json{ {"x", vec.x}, {"y", vec.y}};
		}

		static void from_json(const json& j, glm::vec2& vec) {
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
		}
	};

	template <>
	struct adl_serializer<glm::quat> {
		static void to_json(json& j, const glm::quat& quat) {
			j = json{ {"x", quat.x}, {"y", quat.y}, {"z", quat.z}, {"w", quat.w} };
		}

		static void from_json(const json& j, glm::quat& quat) {
			j.at("x").get_to(quat.x);
			j.at("y").get_to(quat.y);
			j.at("z").get_to(quat.z);
			j.at("w").get_to(quat.w);
		}
	};

	template <>
	struct adl_serializer<glm::vec4> {
		static void to_json(json& j, const glm::vec4& vec) {
			j = json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w} };
		}

		static void from_json(const json& j, glm::vec4& vec) {
			j.at("x").get_to(vec.x);
			j.at("y").get_to(vec.y);
			j.at("z").get_to(vec.z);
			j.at("w").get_to(vec.w);
		}
	};

	template <>
	struct adl_serializer<glm::mat4> {
		static void to_json(json& j, const glm::mat4& mat) {
			const float* mat4_rep = glm::value_ptr(mat);
			for (int i = 0; i < 16; i++) {
				j.push_back(mat4_rep[i]);
			}
		}

		static void from_json(const json& j, glm::mat4& mat) {
			float* mat4_rep = glm::value_ptr(mat);
			for (int i = 0; i < 16; i++) {
				mat4_rep[i] = j[i].get<float>();
			}
		}
	};

}

class UnitConverter {
public:

	static glm::vec2 ScreenSpaceToNDC(glm::vec2 screen_space);

};

NLOHMANN_JSON_SERIALIZE_ENUM(RenderPrimitiveType, {
	{RenderPrimitiveType::CHAR, "CHAR"},
	{RenderPrimitiveType::FLOAT, "FLOAT"},
	{RenderPrimitiveType::INT, "INT"},
	{RenderPrimitiveType::MAT3, "MAT3"},
	{RenderPrimitiveType::MAT4, "MAT4"},
	{RenderPrimitiveType::UNKNOWN, nullptr},
	{RenderPrimitiveType::UNSIGNED_CHAR, "UNSIGNED_CHAR"},
	{RenderPrimitiveType::UNSIGNED_INT, "UNSIGNED_INT"},
	{RenderPrimitiveType::VEC2, "VEC2"},
	{RenderPrimitiveType::VEC3, "VEC3"},
	{RenderPrimitiveType::VEC4, "VEC4"}
	})
