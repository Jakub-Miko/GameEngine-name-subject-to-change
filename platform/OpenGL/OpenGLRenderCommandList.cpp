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
#include <gl/glew.h>

template<typename T, typename ...Args>
void OpenGLRenderCommandList::PushCommand(Args&& ...args)
{
    OpenGLRenderCommandAllocator::Pool* pool = reinterpret_cast<OpenGLRenderCommandAllocator::Pool*>(m_Alloc->Get());
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
    : RenderCommandList(renderer, alloc)
{

}

void OpenGLRenderCommandList::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PushCommand<OpenGLDrawCommand>(pos, size, color);
}

void OpenGLRenderCommandList::DrawSquare(const glm::mat4& transform, glm::vec4 color)
{
    PushCommand<OpenGLDrawCommand>(transform, color);
}

void OpenGLRenderCommandList::BindOpenGLContext()
{
    PushCommand<OpenGLBindOpenGLContextCommand>();
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

void OpenGLRenderCommandList::Execute()
{
    OpenGLRenderCommand* next = m_Commands;
    while(next) {
        PROFILE("Command Execution");
        next->Execute();
        next->~OpenGLRenderCommand();
        next = next->next;
    }
    m_Commands = nullptr;
}

OpenGLRenderCommandList::~OpenGLRenderCommandList()
{
    m_Commands = nullptr;
}
