#include "OpenGLRenderQueue.h"
#include "OpenGLDrawCommand.h"
#include "OpenGLRenderCommand.h"
#include <Renderer/Renderer.h>


OpenGLRenderQueue::OpenGLRenderQueue(std::shared_ptr<Renderer> renderer)
    : RenderQueue(renderer)
{
}

void OpenGLRenderQueue::DrawSquare(float pos_x, float pos_y, float size_x, float size_y) {
    m_Commands.push_back(new OpenGLDrawCommand(pos_x, pos_y, size_x, size_y));
}

void OpenGLRenderQueue::Submit() {
    m_Renderer->SubmitQueue(this);
}

void OpenGLRenderQueue::Execute()
{
    for (auto command : m_Commands) {
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
