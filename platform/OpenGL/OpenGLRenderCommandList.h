#pragma once
#include <Renderer/RenderCommandList.h>
#include <vector>
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>
#include <stdint.h>

class OpenGLRenderCommand;
struct GLFWwindow;

class OpenGLRenderCommandList : public RenderCommandList {
public:
    friend class OpenGLRenderCommandQueue;
    friend class OpenGLRenderResourceManager;
    OpenGLRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);


    virtual void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> buffer) override;
    virtual void SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer) override;
    virtual void SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer) override;
    virtual void SetDefaultRenderTarget() override;
    virtual void SetScissorRect(const RenderScissorRect& scissor_rect) override;
    virtual void SetViewport(const RenderViewport& viewport) override;
    virtual void GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) override;
    virtual void SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table) override;
    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void Draw(uint32_t index_count,bool use_unsined_short_as_index = false, int index_offset = 0) override;

    virtual ~OpenGLRenderCommandList();
    
    void BindOpenGLContext();

private:
    std::shared_ptr<Pipeline> current_pipeline = nullptr;
    template<typename T,typename ... Args>
    void PushCommand(Args&& ... args);

    void UpdateBufferResource(std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset);
    void UpdateBufferResourceAndReallocate(std::shared_ptr<RenderBufferResource> resource, void* data, size_t size);
    void UpdateTexture2DResource(std::shared_ptr<RenderTexture2DResource> resource, int level, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y);
    
    virtual void Execute() override;

    OpenGLRenderCommand* m_Commands = nullptr;
    OpenGLRenderCommand* m_Commands_tail = nullptr;
    std::shared_ptr<RenderBufferResource> index_buffer;
    std::shared_ptr<RenderBufferResource> vertex_buffer;

    
};

