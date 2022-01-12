#pragma once
#include <Renderer/RenderCommandList.h>
#include <vector>
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>

class OpenGLRenderCommand;
struct GLFWwindow;

class OpenGLRenderCommandList : public RenderCommandList {
public:
    friend class OpenGLRenderCommandQueue;
    friend class OpenGLRenderResourceManager;
    OpenGLRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);


    virtual void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer) override;
    virtual void SetPipeline(Pipeline* pipeline) override;
    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void Draw() override;

    virtual ~OpenGLRenderCommandList();
    
    void BindOpenGLContext();

private:
    Pipeline* current_pipeline = nullptr;
    template<typename T,typename ... Args>
    void PushCommand(Args&& ... args);

    void UpdateBufferResource(std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset);

    virtual void Execute() override;

    OpenGLRenderCommand* m_Commands = nullptr;
    OpenGLRenderCommand* m_Commands_tail = nullptr;
    std::shared_ptr<RenderBufferResource> index_buffer;
    
};

