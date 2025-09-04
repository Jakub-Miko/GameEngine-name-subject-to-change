#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <Core/ExecutableCommand.h>
#include <Renderer/RenderResource.h>
#include <Renderer/PipelineManager.h>

class Renderer;
class Material;

class RenderCommandList : public ExecutableCommand
{
public:
    friend Renderer;

    virtual ~RenderCommandList() {};

    virtual void SetPipeline(std::shared_ptr<Pipeline> pipeline) = 0;

    virtual void SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer) = 0;
    virtual void SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer) = 0;
    virtual void SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture) = 0;
    virtual void SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture) = 0;
    virtual void SetTexture2DCubemap(const std::string& semantic_name, std::shared_ptr<RenderTexture2DCubemapResource> texture) = 0;
    virtual void SetResourceDefaultState(std::shared_ptr<RenderResource> resource, RenderState state) = 0;
    virtual void SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer) = 0;
    virtual void SetDefaultRenderTarget() = 0;
    virtual void Clear() = 0;
    virtual void SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer) = 0;
    virtual void SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer) = 0;
    virtual void SetScissorRect(const RenderScissorRect& scissor_rect) = 0;
    virtual void SetViewport(const RenderViewport& viewport) = 0;
    virtual void GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture) = 0;
    virtual void Draw(uint32_t index_count, bool use_unsined_short_as_index = false,int index_offset = 0) = 0;
    virtual void DrawArray(uint32_t vertex_count) = 0;
    virtual void SetMaterial(const std::string& name, std::shared_ptr<Material> material) = 0;

    virtual void DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color = {1.f,1.f,1.f,1.f}) = 0;
    virtual void DrawSquare(const glm::mat4& transform, glm::vec4 color = { 1.f,1.f,1.f,1.f }) = 0;

protected:

    // to get around encapsulation and allow the command list implementation to mutate the Material without exposing Mutable references to a public interface
    std::vector<Material::MaterialParameter>& GetMutableMaterialParameters(Material* material); 
    Material::Material_status& GetMutableMaterialStatus(Material* material); 
};
