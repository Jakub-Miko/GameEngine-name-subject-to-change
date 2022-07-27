#pragma once
#include <Renderer/RenderResource.h>
#include <string>
#include <Core/RuntimeTag.h>
#include <vector>

enum class RenderPassResourceDescriptor_Access {
	READ = 0, WRITE = 1
};

struct RenderPassResourceDescriptor {
	std::string resource_name;
	RuntimeTagIdType type_id;
	RenderPassResourceDescriptor_Access desc_access;
};

using RenderPassResourceDefinnition = typename std::vector<RenderPassResourceDescriptor>;

class RenderPass {
public:

	virtual void Setup(RenderPassResourceDefinnition& setup_builder) = 0;
	virtual void Render() = 0;

};