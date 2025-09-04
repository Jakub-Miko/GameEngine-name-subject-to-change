#pragma once
#include <Core/ResettableSharedFromThis.h>
#include <Renderer/RenderCommandList.h>
#include "VulkanRenderCommandAllocator.h"
#include "VulkanDeferredDestruction.h"
#include "VulkanRenderDescriptorHeap.h"
#include <Renderer/PipelineManager.h>
#include "VulkanPipelineManager.h"
#include <unordered_map>
#include <unordered_set>
#include <vulkan/vulkan.h>

enum class VulkanCommandListDependencyType : char {
    WRITE, READ, INVALID, NONE
};

class VulkanRenderCommandList;

struct VulkanCommandListDependency {
    VulkanCommandListDependencyType access_type;
    RenderState desired_state;
};

struct VulkanCommandListDependencyState {

    VulkanCommandListDependencyState() = default;

    VulkanCommandListDependencyType type = VulkanCommandListDependencyType::INVALID; // aggregate access type of all accesses in a command buffer.
    VulkanCommandListDependencyType previous_access = VulkanCommandListDependencyType::INVALID; // type of the previous access to the resource in the command buffer
    RenderState current_state = RenderState::COMMON; // The state after the previous operation on the resource in the command buffer
    RenderState expected_state = RenderState::COMMON; // The state the resource is expected to be in at the beginning of the command buffer, dictated by the default_state of the resource
    RenderState desired_final_state = RenderState::EMPTY;  // The state the resource should transition to at the end of the command buffer, used to change resource default state
    bool allow_uninitialized = false; // whether the resource can be in an uninitialized state when submitting the command buffer 
};

struct VulkanCommandListDependencyExtra { //Used to tell Render Resource manager some extra information for the sake of optimizations
    PipelineStage source_stage = PipelineStage::ALL_STAGES;
    PipelineStage target_stage = PipelineStage::ALL_STAGES;
}; 

struct VulkanDrawResource {
    std::shared_ptr<RenderResource> resource;
    VulkanCommandListDependency dependency;
};

// draw resources deferred for dependency creation at drawcall
struct VulkanDrawState {

    /**
     * @brief Records a resource binding to the current pipeline
     * @return true when the binding didn't previously exist, false if the previous binding was replaced.
     */
    bool BindResource(uint32_t binding_id) {
        auto result = draw_resources.insert(binding_id);
        return result.second;
    }

    void SetMatertialResources(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t bind_id);

    void InvalidateDrawDependencies(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline); // Remove all drawcall dependencies after an invalid pipeline was set

    void AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id);

    bool IsPipelineReady();

    int expected_binding_count = 0, currently_bound_count = 0;
    std::unordered_set<uint32_t> draw_resources; 
    std::vector<VulkanDrawResource> pending_dependencies;
    std::unordered_set<VulkanRenderDescriptorTable> used_descriptor_tables; 
};

class VulkanDependencyHandler {
public:
    struct VulkanDependencyHandlerFeedback {
        uint64_t timeline_wait;
    };

    virtual VulkanCommandListDependencyState AddDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
        VulkanCommandListDependencyType access_type, RenderState desired_state, VulkanCommandListDependencyExtra extra = VulkanCommandListDependencyExtra()) = 0; // add an immediate dependency

    virtual void SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state) = 0;

    virtual void AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
        VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t root_paramter_id) = 0; // add a dependency for drawcalls

    virtual void AddMaterialDependency(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t root_paramter_id) = 0; // add a material dependency for drawcalls

    virtual void FlushDrawDependencies(VulkanRenderCommandList* list) = 0; // Process draw call dependencies before drawcalls

    virtual void PipelineChange(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline) = 0;
    virtual void RenderTargetChange(VulkanRenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> new_framebuffer) = 0;

    virtual bool IsPipelineReady() = 0; //Check if all resources have been set and are valid before launching a drawcall

    virtual void AddDescriptorTableDependency(VulkanRenderCommandList* list, VulkanRenderDescriptorTable desc_table) = 0;  

    virtual VulkanCommandListDependencyState GetDependency(std::shared_ptr<RenderResource> resource) = 0;
    virtual VulkanDependencyHandlerFeedback FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value) = 0;
    virtual void Reset() = 0;
};

class DefaultVulkanDependencyHandler : public VulkanDependencyHandler {
public:
    DefaultVulkanDependencyHandler() : dependencies(), non_dependent_resources(), framebuffer_dependency_pending(true) {}

    virtual VulkanCommandListDependencyState AddDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
        VulkanCommandListDependencyType access_type, RenderState desired_state, VulkanCommandListDependencyExtra extra = VulkanCommandListDependencyExtra()) override;
    
    virtual void SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state) override;

    virtual void AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
        VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id) override;

    virtual void AddMaterialDependency(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t bind_id) override;
    
    virtual void FlushDrawDependencies(VulkanRenderCommandList* list) override;

    virtual void AddDescriptorTableDependency(VulkanRenderCommandList* list, VulkanRenderDescriptorTable desc_table) override;  

    virtual void PipelineChange(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline) override;
    virtual void RenderTargetChange(VulkanRenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> new_framebuffer) override;

    virtual bool IsPipelineReady() override;

    virtual VulkanCommandListDependencyState GetDependency(std::shared_ptr<RenderResource> resource) override;
    virtual VulkanDependencyHandlerFeedback FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value) override;
    virtual void Reset() override;

private:
    std::unordered_map<std::shared_ptr<RenderResource>, VulkanCommandListDependencyState> dependencies;
    std::vector<std::shared_ptr<RenderResource>> non_dependent_resources; //Resource which dont have dependencies but their references need to be held until submision to prevent their destruction
    VulkanDrawState draw_state; // dependencies which will be used be the next drawcall
    bool framebuffer_dependency_pending = true;
};


class VulkanRenderCommandList : public RenderCommandList, public VulkanDeferredDestruction, public ResettableSharedFromThis<VulkanRenderCommandList>
{
public:

    friend Renderer;
    friend class VulkanRenderResourceManager;
    friend class VulkanRenderCommandQueue;

    VulkanRenderCommandList(std::shared_ptr<VulkanRenderCommandAllocator> alloc);
    virtual ~VulkanRenderCommandList();

    virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) override;
    virtual void Execute() override;
    virtual void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture) override;
    virtual void SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture) override;
    virtual void SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state) override;
    virtual void SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer) override;
    virtual void SetDefaultRenderTarget() override;
    virtual void Clear() override;
    virtual void SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer) override;
    virtual void SetScissorRect(const RenderScissorRect& scissor_rect) override;
    virtual void SetViewport(const RenderViewport& viewport) override;
    virtual void GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void Draw(uint32_t index_count, bool use_unsined_short_as_index = false, int index_offset = 0) override;
    virtual void DrawArray(uint32_t vertex_count) override;
    virtual void SetMaterial(const std::string& name, std::shared_ptr<Material> material) override;

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;

    VkCommandBuffer* GetVkCommandBuffer() { return &command_buffer; }
    std::shared_ptr<Pipeline> GetCurrentPipeline() { return std::static_pointer_cast<Pipeline>(current_pipeline); }
    std::shared_ptr<RenderFrameBufferResource> GetCurrentFrameBuffer() { return std::static_pointer_cast<RenderFrameBufferResource>(current_framebuffer); }

    VulkanCommandListDependencyState AddDependency(std::shared_ptr<RenderResource> dep_resource ,VulkanCommandListDependencyType access_type, RenderState desired_state); //Adds or updates a dependency, and returns the previous dependency state
    VulkanCommandListDependencyState GetDependency(std::shared_ptr<RenderResource> dep_resource); //Gets the previous dependency if present

    bool IsRenderPassActive() { return render_pass_active; }

    //Ensure the renderpass is active
    void InsideRenderPass();


    //Ensure the renderpass is inactive
    void OutsideRenderPass();

    std::shared_ptr<RenderResource> GetVertexBuffer() {
        return vertex_buffer;
    }

    std::shared_ptr<RenderResource> GetIndexBuffer() {
        return index_buffer;
    }

    /**
     * @brief The timeline value of the last submit of this command buffer.
     * Zero if not submitted.
     */
    uint32_t GetLastSubmitTimelineValue() const {
        return timeline_submitted;
    }

    void FlushDrawState();

    virtual bool Destroy() override;

    void ResetState();
    void ResetCommandBuffer();

private:
    VkCommandBuffer command_buffer;

    VulkanDependencyHandler* dependency_handler;
    std::weak_ptr<VulkanRenderCommandAllocator> allocator;
    std::shared_ptr<RenderFrameBufferResource> current_framebuffer = nullptr;
    std::shared_ptr<VulkanPipeline> current_pipeline = nullptr;
    std::shared_ptr<RenderResource> vertex_buffer = nullptr;
    std::shared_ptr<RenderResource> index_buffer = nullptr;
    RenderViewport viewport = RenderViewport({0,0}, {0,0}, 0.0f, 0.0f);
    RenderScissorRect scissor_rect = RenderScissorRect({0,0}, {0,0});
    uint32_t timeline_submitted = 0; // The last time this command buffer was submitted, 0 means never
    bool render_pass_active = false;
    bool is_scissorrect_defined = false, is_viewport_defined = false;
    bool are_index_vertex_buffers_bound = false;
};
