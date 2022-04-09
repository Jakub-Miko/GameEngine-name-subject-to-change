#pragma once
#include <string>
#include <glm/glm.hpp>
#include <type_traits>
#define RuntimeTag(name) public: inline static constexpr std::string_view type_name = name;
#define NonIntrusiveRuntimeTag(type, name) template<> struct RuntimeTag<##type, void> { static constexpr std::string_view GetName() { return name; } };

template<typename T, typename dummy = void>
struct RuntimeTag {

	static constexpr std::string_view GetName() {
		return "Unidentified";
	}

};

template<typename T>
struct RuntimeTag<T, std::void_t<decltype(T::type_name)>> {

	static constexpr std::string_view GetName() {
		return T::type_name;
	}

};


NonIntrusiveRuntimeTag(int, "int")
NonIntrusiveRuntimeTag(std::string, "string")
NonIntrusiveRuntimeTag(float, "float")
NonIntrusiveRuntimeTag(double, "double")
NonIntrusiveRuntimeTag(glm::vec3, "vec3")
NonIntrusiveRuntimeTag(glm::vec4, "vec4")
NonIntrusiveRuntimeTag(glm::vec2, "vec2")

