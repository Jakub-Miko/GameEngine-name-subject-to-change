#pragma once
#include <Renderer/RenderCommandList.h>
#include <unordered_map>
#include <vulkan/vulkan.h>

enum class VulkanCommandListDependencyType : char {
    WRITE, READ, INVALID
};

struct VulkanCommandListDependency {
    VulkanCommandListDependency(VulkanCommandListDependencyType type, RenderState current_state, RenderState expected_state) 
        : type(type),previous_access(type), expected_state(expected_state), current_state(current_state) {}

    VulkanCommandListDependency() = default;

    VulkanCommandListDependencyType type = VulkanCommandListDependencyType::INVALID;
    VulkanCommandListDependencyType previous_access = VulkanCommandListDependencyType::INVALID;
    RenderState expected_state = RenderState::COMMON;
    RenderState current_state = RenderState::COMMON;
};

struct VulkanCommandListDependencyExtra { //Used to tell Render Resource manager some extra information for the sake of optimizations
    PipelineStage source_stage = PipelineStage::ALL_STAGES;
    PipelineStage target_stage = PipelineStage::ALL_STAGES;
};

class VulkanDependencyHandler {
public:
    struct VulkanDependencyHandlerFeedback {
        uint64_t timeline_wait;
    };

    virtual VulkanCommandListDependency AddDependency(RenderCommandList* list, std::shared_ptr<RenderResource> resource, 
        VulkanCommandListDependency dependency, VulkanCommandListDependencyExtra extra = VulkanCommandListDependencyExtra()) = 0;
    virtual VulkanCommandListDependency GetDependency(std::shared_ptr<RenderResource> resource) = 0;
    virtual VulkanDependencyHandlerFeedback FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value) = 0;
    virtual void Reset() = 0;
};

class DefaultVulkanDependencyHandler : public VulkanDependencyHandler {
public:
    DefaultVulkanDependencyHandler() : dependencies(), non_dependent_resources() {}

    virtual VulkanCommandListDependency AddDependency(RenderCommandList* list, std::shared_ptr<RenderResource> resource, 
        VulkanCommandListDependency dependency, VulkanCommandListDependencyExtra extra = VulkanCommandListDependencyExtra()) override;
    virtual VulkanCommandListDependency GetDependency(std::shared_ptr<RenderResource> resource) override;
    virtual VulkanDependencyHandlerFeedback FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value) override;
    virtual void Reset() override;

private:
    std::unordered_map<std::shared_ptr<RenderResource>, VulkanCommandListDependency> dependencies;
    std::vector<std::shared_ptr<RenderResource>> non_dependent_resources; //Resource which dont have dependencies but their references need to be held until submision to prevent their destruction
};


class VulkanRenderCommandList : public RenderCommandList
{
public:

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

    VulkanDependencyHandler* dependency_handler;
    std::shared_ptr<RenderFrameBufferResource> current_framebuffer = nullptr;
};
