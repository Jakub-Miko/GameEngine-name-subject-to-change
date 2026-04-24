#pragma once
#include <optional>
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
#include <Renderer/RenderResourceStore.h>

enum class VulkanCommandListDependencyType : unsigned char {
    WRITE = 1,
    READ = 2,
    INVALID = 4,
    NONE = 0
};

inline VulkanCommandListDependencyType operator|(const VulkanCommandListDependencyType& type_1, const VulkanCommandListDependencyType& type_2) {
    return (VulkanCommandListDependencyType)((unsigned char)type_1 | (unsigned char)type_2);
}

inline VulkanCommandListDependencyType operator|=(const VulkanCommandListDependencyType& type_1, const VulkanCommandListDependencyType& type_2) {
    return (VulkanCommandListDependencyType)((unsigned char)type_1 | (unsigned char)type_2);
}

inline VulkanCommandListDependencyType operator&(const VulkanCommandListDependencyType& type_1, const VulkanCommandListDependencyType& type_2) {
    return (VulkanCommandListDependencyType)((unsigned char)type_1 & (unsigned char)type_2);
}

inline VulkanCommandListDependencyType operator&=(const VulkanCommandListDependencyType& type_1, const VulkanCommandListDependencyType& type_2) {
    return (VulkanCommandListDependencyType)((unsigned char)type_1 & (unsigned char)type_2);
}

class VulkanRenderCommandList;

struct VulkanCommandListDependency {
    VulkanCommandListDependencyType access_type;
    RenderState desired_state;
};

struct VulkanCommandListDependencyState {

    VulkanCommandListDependencyState() = default;

    VulkanCommandListDependencyType type = VulkanCommandListDependencyType::NONE; // aggregate access type of all accesses in a command buffer.
    VulkanCommandListDependencyType previous_access = VulkanCommandListDependencyType::INVALID; // type of the previous access to the resource in the command buffer
    RenderState current_state = RenderState::COMMON; // The state after the previous operation on the resource in the command buffer
    RenderState expected_state = RenderState::COMMON; // The state the resource is expected to be in at the beginning of the command buffer, dictated by the default_state of the resource
    RenderState desired_final_state = RenderState::EMPTY;  // The state the resource should transition to at the end of the command buffer, used to change resource default state
    uint32_t last_render_pass_used = 0;
    bool allow_uninitialized = false; // whether the resource can be in an uninitialized state when submitting the command buffer 
};

struct VulkanDependencyState {
    VulkanCommandListDependencyType access_type;
    RenderState desired_state;
    PipelineStage stage = PipelineStage::ALL_STAGES;
};

struct VulkanDrawResource {
    std::shared_ptr<RenderResource> resource;
    VulkanDependencyState dependency_target_state;
};

struct VulkanBarrier {
    VkDependencyInfo GetBarrierInfo() const;

    void ClearBarriers() {
        global_memory_barriers.clear();
        image_barriers.clear();
        buffer_barriers.clear();
        force_emission = false;
        dependency_updates.clear();
    }

    void AddBarrier(std::shared_ptr<RenderResource> resource, const VulkanDependencyState& source_dependency_state, const VulkanDependencyState& target_dependency_state);

    void AddGlobalMemoryBarrier(const VulkanDependencyState& source_dependency_state, const VulkanDependencyState& target_dependency_state);

private:
    friend class VulkanDependencyHandler;
    struct DependencyUpdate {
        uint32_t dependency_index = 0;
        VulkanDependencyState target_state;
    };

    struct StoreUpdate {

        VulkanDependencyState target_state;
    };

    bool force_emission = false; // Whether this barrier needs to be emitted even inside a renderpass.
    std::vector<DependencyUpdate> dependency_updates;
    std::vector<VkImageMemoryBarrier2> image_barriers;
    std::vector<VkBufferMemoryBarrier2> buffer_barriers;
    std::vector<VkMemoryBarrier2> global_memory_barriers;
};

// draw resources deferred for dependency creation at drawcall
struct VulkanDrawState {

    void SetMaterialResources(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t bind_id);

    void InvalidateDrawDependencies(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline); // Remove all drawcall dependencies after an invalid pipeline was set

    void AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id);

    void AddStoreUsage(VulkanRenderCommandList* list, std::shared_ptr<RenderResourceStore> store, uint32_t bind_id);

    bool IsPipelineReady();

    void AssertIndividualResourceValidity(std::shared_ptr<RenderResource> resource);

    std::unordered_map<uint32_t, uint32_t> draw_resources;
    std::vector<VulkanDrawResource> pending_dependencies;
    std::vector<std::shared_ptr<RenderResourceStore>> pending_stores;
    std::unordered_set<VulkanRenderDescriptorTable> used_descriptor_tables;
    int expected_binding_count = 0, currently_bound_count = 0;
    bool dirty = true;
};

class VulkanDependencyHandler {
public:

    struct VulkanDependencyHandlerFeedback {
        uint64_t timeline_wait = 0;
    };

    VulkanDependencyHandler() = default;

    VulkanDependencyState UpdateDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, const VulkanDependencyState& dependency_target_state);

    void PrepareDependencyForEmission(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, VulkanDependencyState dependency_target_state);
    void PrepareStoreDependencyForEmission(VulkanRenderCommandList* list, std::shared_ptr<RenderResourceStore> store);
    void FlushPreparedDependencies(VulkanRenderCommandList* list);

    void AddDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, const VulkanDependencyState& dependency_target_state);

    void AddDependencyToBarrier(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource, VulkanDependencyState dependency_target_state, VulkanBarrier& barrier);

    void AddStoreUsage(VulkanRenderCommandList* list, std::shared_ptr<RenderResourceStore> store, uint32_t bind_id);

    void IterateStoreDependencies(std::shared_ptr<RenderResourceStore> store, std::function<void(const VulkanDrawResource&)> dependency_callback, bool clear_after_iteration = false);

    void ClearStoreDependencies(std::shared_ptr<RenderResourceStore> store);

    void SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state);

    void AddDrawDependency(VulkanRenderCommandList* list, std::shared_ptr<RenderResource> resource,
        VulkanCommandListDependencyType access_type, RenderState desired_state, uint32_t bind_id);

    void AddMaterialDependency(VulkanRenderCommandList* list, std::shared_ptr<Material> material, uint32_t bind_id);

    void FlushDrawDependencies(VulkanRenderCommandList* list);

    void PipelineChange(VulkanRenderCommandList* list, std::shared_ptr<Pipeline> new_pipeline);
    void RenderTargetChange(VulkanRenderCommandList* list, std::shared_ptr<RenderFrameBufferResource> new_framebuffer);

    bool IsPipelineReady();

    void EnsureInitialization(std::shared_ptr<RenderResource> resource);

    VulkanCommandListDependencyState GetDependency(std::shared_ptr<RenderResource> resource);
    VulkanDependencyHandlerFeedback FinalizeDependencies(RenderCommandList* list, uint64_t new_timeline_value);
    void Reset();

    bool IsDrawStateDirty() const {
        return draw_state.dirty;
    }

private:

    void AddIndividualResourceOverride(int individual_resource_index);

    struct RenderResourceStoreDependency {
        int32_t store_version = 0;
        uint32_t first_resource_override = std::numeric_limits<uint32_t>::max();
        uint32_t last_render_pass_used = 0;
    };

    using render_store_map_t = std::unordered_map<std::shared_ptr<RenderResourceStore>, RenderResourceStoreDependency>;
    using individual_resource_map_t = std::unordered_map<std::shared_ptr<RenderResource>, uint32_t>;

    struct IndividualResourceDependency {
        bool IsInitialized() {
            return state.type != VulkanCommandListDependencyType::NONE;
        }

        std::shared_ptr<RenderResource> resource;
        VulkanCommandListDependencyState state;
        std::optional<render_store_map_t::iterator> store_it;
        int32_t store_version = -1; // The state version of the resource store when the dependency was created.
        uint32_t next_resource = std::numeric_limits<uint32_t>::max(); // Index of the next dirty resource belonging to the same resource store
    };

    individual_resource_map_t::iterator GetNewIndividualResourceRecord(std::shared_ptr<RenderResource> resource);

    render_store_map_t resource_store_dependencies_map;
    individual_resource_map_t individual_resource_dependencies_map;
    std::vector<IndividualResourceDependency> individual_resource_dependency_storage;
    VulkanBarrier barrier;

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
    virtual void SetStorageBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) override;
    virtual void SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void SetStorageTexture(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture) override;
    virtual void SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture) override;
    virtual void SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture) override;
    virtual void SetPushConstantRange(void* data, size_t size) override;
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
    virtual void Dispatch(uint32_t thread_group_count_x, uint32_t thread_group_count_y, uint32_t thread_group_count_z) override;
    virtual void SetMaterial(const std::string& name, std::shared_ptr<Material> material) override;
    virtual void SetResourceStore(const std::string& name, std::shared_ptr<RenderResourceStore> resource_store) override;
    virtual void AttachResourceToStoreAfterSubmission(std::shared_ptr<RenderResourceStore> store, std::shared_ptr<RenderResource> resource) override;

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) override;

    void AddSubmissionCallback(std::function<void()> callback);

    VkCommandBuffer* GetVkCommandBuffer() { return &command_buffer; }
    std::shared_ptr<Pipeline> GetCurrentPipeline() { return current_pipeline; }
    std::shared_ptr<RenderFrameBufferResource> GetCurrentFrameBuffer() { return std::static_pointer_cast<RenderFrameBufferResource>(current_framebuffer); }

    void AddDependency(std::shared_ptr<RenderResource> dep_resource ,VulkanCommandListDependencyType access_type, RenderState desired_state); //Adds or updates a dependency, and returns the previous dependency state
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

    uint32_t GetRenderPassCounter() const { return render_pass_counter; }
    void IncrementRenderPassCounter() { render_pass_counter++; }

    virtual bool Destroy() override;

    void ResetState();
    void ResetCommandBuffer();

private:
    VkCommandBuffer command_buffer;

    VulkanDependencyHandler dependency_handler = {};
    std::weak_ptr<VulkanRenderCommandAllocator> allocator;
    std::shared_ptr<RenderFrameBufferResource> current_framebuffer = nullptr;
    std::shared_ptr<Pipeline> current_pipeline = nullptr;
    std::shared_ptr<RenderResource> vertex_buffer = nullptr;
    std::shared_ptr<RenderResource> index_buffer = nullptr;
    std::vector<std::function<void()>> submission_callbacks;
    RenderViewport viewport = RenderViewport({0,0}, {0,0}, 0.0f, 0.0f);
    RenderScissorRect scissor_rect = RenderScissorRect({0,0}, {0,0});
    uint32_t timeline_submitted = 0; // The last time this command buffer was submitted, 0 means never
    uint32_t render_pass_counter = 0;
    bool render_pass_active = false;
    bool is_scissorrect_defined = false, is_viewport_defined = false;
    bool are_index_vertex_buffers_bound = false;
};
