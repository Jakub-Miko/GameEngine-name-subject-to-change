#include "OpenGLRenderQueue.h"
#include "OpenGLDrawCommand.h"
#include "OpenGLRenderCommand.h"
#include <Renderer/Renderer.h>
#include <Utility/Profiler.h>

OpenGLRenderQueue::OpenGLRenderQueue(std::shared_ptr<Renderer> renderer)
    : RenderQueue(renderer)
{
}

void OpenGLRenderQueue::DrawSquare(glm::vec2 pos, glm::vec2 size, glm::vec4 color) {
    PROFILE("Draw Square Command");
    m_Commands.push_back(new OpenGLDrawCommand(pos, size,color));
}

void OpenGLRenderQueue::Submit() {
    m_Renderer->SubmitQueue(this);
}

void OpenGLRenderQueue::Execute()
{
    for (auto command: m_Commands) {
        PROFILE("Command Execution");
        command->Execute();
        delete command;
    }
    m_Commands.clear();
}

OpenGLRenderQueue::~OpenGLRenderQueue()
{
    for (auto command : m_Commands) {
        delete command;
    }
}
