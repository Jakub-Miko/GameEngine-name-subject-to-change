#pragma once
#include <string>
#include <glm/glm.hpp>
#include <TypeId.h>
#include <Renderer/RenderResource.h>

NonIntrusiveRuntimeTag(int, "int")
NonIntrusiveRuntimeTag(std::string, "string")
NonIntrusiveRuntimeTag(float, "float")
NonIntrusiveRuntimeTag(double, "double")
NonIntrusiveRuntimeTag(glm::vec3, "vec3")
NonIntrusiveRuntimeTag(glm::vec4, "vec4")
NonIntrusiveRuntimeTag(glm::vec2, "vec2")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderResource>, "RenderResource")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderBufferResource>, "RenderBufferResource")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderTexture2DResource>, "RenderTexture2DResource")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderFrameBufferResource>, "RenderFrameBufferResource")

