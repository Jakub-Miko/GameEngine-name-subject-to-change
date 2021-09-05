#include "OpenGLRenderContext.h"
#include <GL/glew.h>
#include <Application.h>

void OpenGLRenderContext::Init()
{
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        Application::Get()->Exit();

    }
}

void OpenGLRenderContext::PreInit()
{

}
