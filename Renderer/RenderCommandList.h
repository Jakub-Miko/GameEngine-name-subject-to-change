#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <Renderer/RenderCommandAllocator.h>
#include <Core/ExecutableCommand.h>
#include <Renderer/RenderResource.h>
#include <Renderer/RootSignature.h>
#include <Renderer/PipelineManager.h>

class Renderer;

class RenderCommandList : public ExecutableCommand
{
public:
    friend Renderer;

    RenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);
    virtual ~RenderCommandList() {};

    virtual void SetPipeline(Pipeline* pipeline) = 0;

    virtual void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) = 0;
    virtual void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) = 0;
    virtual void SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer) = 0;
    virtual void SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer) = 0;
    virtual void Draw() = 0;

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = {1.f,1.f,1.f,1.f}) = 0;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) = 0;

protected:

    Renderer* m_Renderer;
    std::shared_ptr<RenderCommandAllocator> m_Alloc;

public:
    static RenderCommandList* CreateQueue(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);
};
