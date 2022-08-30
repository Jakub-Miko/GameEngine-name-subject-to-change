#include "OpenGLRenderCommandList.h"
#include "OpenGLDrawCommand.h"
#include "OpenGLBindOpenGLContextCommand.h"
#include "OpenGLRenderCommand.h"
#include "OpenGLRenderCommandAllocator.h"
#include <memory>
#include <platform/OpenGL/OpenGLRenderResource.h>
#include <memory_resource>
#include <type_traits>
#include <stdexcept>
#include <Renderer/Renderer.h>
#include <Profiler.h>
#include <utility>
#include <GL/glew.h>
#include "OpenGLConstantBufferCommands.h"
#include "OpenGLUnitConverter.h"
#include "OpenGLResourceDescriptorCommands.h"
#include "OpenGLPipelineCommands.h"
#include "OpenGLTextureCommands.h"
#include "OpenGLFrameBufferCommands.h"
#include "OpenGLDrawCommands.h"
#include "OpenGLVertexIndexBufferCommands.h"
#include "OpenGLPipelineManager.h"

template<typename T, typename ...Args>
void OpenGLRenderCommandList::PushCommand(Args&& ...args)
{
    OpenGLRenderCommandAllocator::Pool* pool = static_cast<OpenGLRenderCommandAllocator::Pool*>(m_Alloc->Get());
    std::pmr::polymorphic_allocator<T> alloc(pool);

    T* cmd = std::allocator_traits<decltype(alloc)>::allocate(alloc, 1);
    std::allocator_traits<decltype(alloc)>::construct(alloc, cmd, std::forward<Args>(args)...);
    if (!m_Commands) {
        m_Commands = cmd;
        m_Commands_tail = cmd;
    }
    else {
        m_Commands_tail->next = cmd;
        m_Commands_tail = cmd;
    }
}


OpenGLRenderCommandList::OpenGLRenderCommandList(Renderer* renderer, std::shared_ptr<RenderCommandAllocator> alloc)
    : RenderCommandList(renderer, alloc), index_buffer(), vertex_buffer()
{

}

void OpenGLRenderCommandList::SetPipeline(std::shared_ptr<Pipeline> pipeline)
{
    current_pipeline = pipeline;
    PushCommand<OpenGLSetPipelineCommand>(pipeline);
}

void OpenGLRenderCommandList::Clear()
{
    auto command = OpenGLRenderCommandAdapter([]() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::SetDescriptorTable(const std::string& semantic_name, RenderDescriptorTable table)
{
    if (current_pipeline) {
        PushCommand<OpenGLSetDescriptorTableCommand>(current_pipeline, semantic_name, table);
    }
    else {
        throw std::runtime_error("No pipeline is bound");
    }
}

void OpenGLRenderCommandList::SetConstantBuffer(RootBinding binding_id, std::shared_ptr<RenderBufferResource> buffer)
{
    if (current_pipeline) {
        PushCommand<OpenGLSetConstantBufferCommandId>(current_pipeline, binding_id, buffer);
    }
    else {
        throw std::runtime_error("No pipeline is bound");
    }
}

void OpenGLRenderCommandList::SetConstantBuffer(const std::string& semantic_name, std::shared_ptr<RenderBufferResource> buffer)
{
    if (current_pipeline) {
        PushCommand<OpenGLSetConstantBufferCommandName>(current_pipeline, semantic_name, buffer);
    }
    else {
        throw std::runtime_error("No pipeline is bound");
    }
}

void OpenGLRenderCommandList::SetTexture2D(const std::string& semantic_name, std::shared_ptr<RenderTexture2DResource> texture)
{
    if (current_pipeline) {
        PushCommand<OpenGLSetTexture2DCommand>(current_pipeline, semantic_name, texture);
    }
    else {
        throw std::runtime_error("No pipeline is bound");
    }
}

void OpenGLRenderCommandList::SetTexture2DArray(const std::string& semantic_name, std::shared_ptr<RenderTexture2DArrayResource> texture)
{
    if (current_pipeline) {
        PushCommand<OpenGLSetTexture2DArrayCommand>(current_pipeline, semantic_name, texture);
    }
    else {
        throw std::runtime_error("No pipeline is bound");
    }
}

//Needs to be Reworked
void OpenGLRenderCommandList::SetIndexBuffer(std::shared_ptr<RenderBufferResource> buffer)
{
    if (!buffer && buffer->GetBufferDescriptor().usage != RenderBufferUsage::INDEX_BUFFER) {
        throw std::runtime_error("Buffer must be an Index buffer");
    }
    index_buffer = buffer;
}

void OpenGLRenderCommandList::SetVertexBuffer(std::shared_ptr<RenderBufferResource> vertex_buffer_in)
{
    if (!vertex_buffer_in && vertex_buffer_in->GetBufferDescriptor().usage != RenderBufferUsage::VERTEX_BUFFER) {
        throw std::runtime_error("Buffer must be an Vertex buffer");
    }
    vertex_buffer = vertex_buffer_in;

    if (bool(current_pipeline->GetPipelineFlags() & PipelineFlags::IS_MULTI_WINDOW)) {
        auto command = OpenGLRenderCommandAdapter([vertex_buffer_in, this]() {
            static_cast<OpenGLPipeline*>(current_pipeline.get())->BeginVertexContext(vertex_buffer_in);
            });
        PushCommand<decltype(command)>(command);
    }

}

void OpenGLRenderCommandList::SetRenderTarget(std::shared_ptr<RenderFrameBufferResource> framebuffer)
{
    PushCommand<OpenGLSetRenderTargetCommand>(framebuffer);
    current_framebuffer = framebuffer;
}

void OpenGLRenderCommandList::SetDefaultRenderTarget()
{
    PushCommand<OpenGLSetDefaultRenderTargetCommand>(Renderer::Get()->GetDefaultFrameBuffer());
    current_framebuffer = Renderer::Get()->GetDefaultFrameBuffer();
}

void OpenGLRenderCommandList::SetScissorRect(const RenderScissorRect& scissor_rect)
{
    PushCommand<OpenGLSetScissorRect>(scissor_rect);
}

void OpenGLRenderCommandList::SetViewport(const RenderViewport& viewport)
{
    PushCommand<OpenGLSetViewport>(viewport);
}

void OpenGLRenderCommandList::GenerateMIPs(std::shared_ptr<RenderTexture2DResource> texture)
{
    PushCommand<OpenGLGenerateMIPsCommand>(texture);
}

void OpenGLRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PushCommand<OpenGLDrawCommand>(pos, size, color);
}

void OpenGLRenderCommandList::DrawSquare(const glm::mat4& transform, glm::vec4 color)
{
    PushCommand<OpenGLDrawCommand>(transform, color);
}

void OpenGLRenderCommandList::Draw(uint32_t index_count, bool use_unsined_short_as_index, int index_offset)
{
    PushCommand<OpenGLImplicitDrawCommand>(current_pipeline,index_buffer,vertex_buffer, index_count, use_unsined_short_as_index, index_offset);
}

void OpenGLRenderCommandList::BindOpenGLContext()
{
    PushCommand<OpenGLBindOpenGLContextCommand>();
}

void OpenGLRenderCommandList::RefreshVertexContext()
{
    auto command = OpenGLRenderCommandAdapter([this]() {
        static_cast<OpenGLPipeline*>(current_pipeline.get())->EndVertexContext();
        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::UpdateBufferResource(std::shared_ptr<RenderBufferResource> resource, void* data, size_t size, size_t offset)
{
    auto command = OpenGLRenderCommandAdapter([resource, data, size, offset]() {
        RenderState state = RenderState::COMMON;
        resource->GetRenderStateAtomic().compare_exchange_strong(state, RenderState::WRITE);
        if (state == RenderState::COMMON) {
            if ((offset + size) > resource->GetBufferDescriptor().buffer_size) {
                throw std::runtime_error("Buffer out of range.");
            }
            glBindBuffer(GL_COPY_WRITE_BUFFER, static_cast<OpenGLRenderBufferResource*>(resource.get())->GetRenderId());
            glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
            resource->SetRenderState(RenderState::COMMON);
            delete[] static_cast<char*>(data);
        }
        else {
            delete[] static_cast<char*>(data);
            throw std::runtime_error("Can't update an Uninitialized resource");
        }
        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::UpdateBufferResourceAndReallocate(std::shared_ptr<RenderBufferResource> resource, void* data, size_t size)
{
    auto command = OpenGLRenderCommandAdapter([resource, data, size]() {
        RenderState state = RenderState::COMMON;
        resource->GetRenderStateAtomic().compare_exchange_strong(state, RenderState::WRITE);
        if (state == RenderState::COMMON) {
            glBindBuffer(GL_COPY_WRITE_BUFFER, static_cast<OpenGLRenderBufferResource*>(resource.get())->GetRenderId());
            glBufferData(GL_COPY_WRITE_BUFFER, size, data, resource->GetBufferDescriptor().type == RenderBufferType::DEFAULT ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
            resource->SetRenderState(RenderState::COMMON);
            delete[] static_cast<char*>(data);
        }
        else {
            delete[] static_cast<char*>(data);
            throw std::runtime_error("Can't update an Uninitialized resource");
        }
        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::UpdateTexture2DResource(std::shared_ptr<RenderTexture2DResource> resource,int level, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y)
{
    auto command = OpenGLRenderCommandAdapter([resource, data, width, height, offset_x, offset_y, level]() {
        RenderState state = RenderState::COMMON;
        resource->GetRenderStateAtomic().compare_exchange_strong(state, RenderState::WRITE);
        if (state == RenderState::COMMON) {
            if ((offset_x + width) > resource->GetBufferDescriptor().width || (offset_y + height) > resource->GetBufferDescriptor().height) {
                throw std::runtime_error("Buffer out of range.");
            }
            glBindTexture(GL_TEXTURE_2D, static_cast<OpenGLRenderTexture2DResource*>(resource.get())->GetRenderId());
            glTexSubImage2D(GL_TEXTURE_2D, level, offset_x, offset_y, width, height, OpenGLUnitConverter::TextureFormatToGLFormat(resource->GetBufferDescriptor().format),
                OpenGLUnitConverter::TextureFormatToGLDataType(resource->GetBufferDescriptor().format),data);


            glBindTexture(GL_TEXTURE_2D, 0);
            resource->SetRenderState(RenderState::COMMON);
            delete[] static_cast<char*>(data);
        }
        else {
            delete[] static_cast<char*>(data);
            throw std::runtime_error("Can't update an Uninitialized resource");
        }
        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::UpdateTexture2DArrayResource(std::shared_ptr<RenderTexture2DArrayResource> resource, int layer, int level, void* data, size_t width, size_t height, size_t offset_x, size_t offset_y)
{
    auto command = OpenGLRenderCommandAdapter([resource, layer, data, width, height, offset_x, offset_y, level]() {
        RenderState state = RenderState::COMMON;
        resource->GetRenderStateAtomic().compare_exchange_strong(state, RenderState::WRITE);
        if (state == RenderState::COMMON) {
            if ((offset_x + width) > resource->GetBufferDescriptor().width || (offset_y + height) > resource->GetBufferDescriptor().height || layer >= resource->GetBufferDescriptor().num_of_textures) {
                throw std::runtime_error("Buffer out of range.");
            }
            glBindTexture(GL_TEXTURE_2D_ARRAY, static_cast<OpenGLRenderTexture2DArrayResource*>(resource.get())->GetRenderId());
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, offset_x, offset_y,layer, width, height, 1 , OpenGLUnitConverter::TextureFormatToGLFormat(resource->GetBufferDescriptor().format),
                OpenGLUnitConverter::TextureFormatToGLDataType(resource->GetBufferDescriptor().format), data);


            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
            resource->SetRenderState(RenderState::COMMON);
            delete[] static_cast<char*>(data);
        }
        else {
            delete[] static_cast<char*>(data);
            throw std::runtime_error("Can't update an Uninitialized resource");
        }
        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::CopyFrameBufferDepthAttachment(std::shared_ptr<RenderFrameBufferResource> source_frame_buffer, std::shared_ptr<RenderFrameBufferResource> destination_frame_buffer)
{
    auto command = OpenGLRenderCommandAdapter([source_frame_buffer, destination_frame_buffer,this]() {
        auto source_desc = source_frame_buffer->GetBufferDescriptor().depth_stencil_attachment->GetBufferDescriptor();
        auto destination_desc = destination_frame_buffer->GetBufferDescriptor().depth_stencil_attachment->GetBufferDescriptor();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<OpenGLRenderFrameBufferResource*>(source_frame_buffer.get())->GetRenderId());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<OpenGLRenderFrameBufferResource*>(destination_frame_buffer.get())->GetRenderId());
        glBlitFramebuffer(0, 0, source_desc.width, source_desc.height, 0, 0, destination_desc.width, destination_desc.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<OpenGLRenderFrameBufferResource*>(current_framebuffer.get())->GetRenderId());

        });
    PushCommand<decltype(command)>(command);
}

void OpenGLRenderCommandList::Execute()
{
    OpenGLRenderCommand* next = m_Commands;
    while(next) {
        next->Execute();
        next->~OpenGLRenderCommand();
        next = next->next;
    }
    m_Commands = nullptr;
}

OpenGLRenderCommandList::~OpenGLRenderCommandList()
{
    if (m_Commands != nullptr) {
        OpenGLRenderCommand* next = m_Commands;
        while (next) {
            next->~OpenGLRenderCommand();
            next = next->next;
        }
    }
    m_Commands = nullptr;
}
