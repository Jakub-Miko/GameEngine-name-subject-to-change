#pragma once
#include <string>
#include <glm/glm.hpp>
#include <TypeId.h>


NonIntrusiveRuntimeTag(int, "int")
NonIntrusiveRuntimeTag(std::string, "string")
NonIntrusiveRuntimeTag(float, "float")
NonIntrusiveRuntimeTag(double, "double")
NonIntrusiveRuntimeTag(glm::vec3, "vec3")
NonIntrusiveRuntimeTag(glm::vec4, "vec4")
NonIntrusiveRuntimeTag(glm::vec2, "vec2")

