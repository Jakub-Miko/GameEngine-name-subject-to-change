#pragma once
#include <string>
#include <glm/glm.hpp>
#include <TypeId.h>
#include <Renderer/RenderResource.h>
#include <World/Entity.h>

NonIntrusiveRuntimeTag(int, "int")
NonIntrusiveRuntimeTag(Entity, "entity")
NonIntrusiveRuntimeTag(std::string, "string")
NonIntrusiveRuntimeTag(float, "float")
NonIntrusiveRuntimeTag(double, "double")
NonIntrusiveRuntimeTag(glm::vec3, "vec3")
NonIntrusiveRuntimeTag(glm::vec4, "vec4")
NonIntrusiveRuntimeTag(glm::vec2, "vec2")
NonIntrusiveRuntimeTag(glm::mat4, "mat4")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderResource>, "RenderResource")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderBufferResource>, "RenderBufferResource")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderTexture2DResource>, "RenderTexture2DResource")
NonIntrusiveRuntimeTag(std::shared_ptr<RenderFrameBufferResource>, "RenderFrameBufferResource")

template<typename T>
struct RuntimeTag<T, std::enable_if_t<std::is_pointer_v<T> && (RuntimeTag<std::remove_pointer_t<T>>::GetName() != "Unidentified"), void>> {

	static constexpr std::string_view GetName() {
		std::string name = (std::string)RuntimeTag<std::remove_pointer_t<T>>::GetName() + "_pointer";
		return name;
	}

	inline static const RuntimeTagIdType GetId() {
		static const RuntimeTagIdType id = SequentialIdGenerator::Id();
		return id;
	}

};