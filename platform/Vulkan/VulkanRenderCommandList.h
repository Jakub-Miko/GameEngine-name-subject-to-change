#pragma once
#include <Renderer/RenderCommandList.h>
#include <unordered_map>
#include <vulkan/vulkan.h>

class VulkanRenderCommandList : public RenderCommandList
{
public:
    enum class VulkanCommandListDependencyType : char {
        WRITE, READ, INVALID
    };

    struct VulkanCommandListDependency {
        VulkanCommandListDependencyType type;
        VulkanCommandListDependencyType previous_access;
        RenderState expected_state = RenderState::COMMON;
        RenderState current_state = RenderState::COMMON;
    };

    friend Renderer;
    friend class VulkanRenderResourceManager;
    friend class VulkanRenderCommandQueue;

    VulkanRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc);
    virtual ~VulkanRenderCommandList();

    virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) override;
    virtual void Execute() override;
    virtual void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture) override;
    virtual void SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture) override;
    virtual void SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer) override;
    virtual void SetDefaultRenderTarget() override;
    virtual void Clear() override;
    virtual void SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer) override;
    virtual void SetScissorRect(const RenderScissorRect& scissor_rect) override;
    virtual void SetViewport(const RenderViewport& viewport) override;
    virtual void SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table) override;
    virtual void GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void Draw(uint32_t index_count, bool use_unsined_short_as_index = false, int index_offset = 0) override;
    virtual void DrawArray(uint32_t vertex_count) override;

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;

    VkCommandBuffer* GetVkCommandBuffer() { return &command_buffer; }

    VulkanCommandListDependency AddDependency(std::shared_ptr<RenderResource> dep_resource ,VulkanCommandListDependency dep); //Adds or updates a dependency, and returns the previous dependency state
    VulkanCommandListDependency GetDependency(std::shared_ptr<RenderResource> dep_resource); //Gets the previous dependency if present

private:
    VkCommandBuffer command_buffer;

    std::unordered_map<std::shared_ptr<RenderResource>, VulkanCommandListDependency> command_list_dependencies;
    std::shared_ptr<RenderFrameBufferResource> current_framebuffer = nullptr;
};
