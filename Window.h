#pragma once
#include <string>
#include <vector>
#include <Renderer/RenderSurface.h>

/**
 * @brief Properties of the window. Descriptor object for window creation.
*/
struct WindowProperties {
    glm::uvec2 resolution = glm::uvec2();
    std::string name = "GameEngine NSTC"; ///< Window name
    bool fullscreen = false; ///< Fullscreen mode
    bool main_window = false; ///< Whether the window is the main window for displaying the render output without editor.
};

/**
 * @brief Abstraction of the window class from the window manager.
 * 
 * Handles window creation and manipulation, and user input. \n
 * For other OS specific operations see OSApi
*/
class Window {
public:

    /**
     * @brief Base constructor which stores the WindowProperties descriptor object
     * @param props WindowProperties for window creation
    */
    Window(const WindowProperties& props);
    
    /**
     * @brief Initializes the windowing system.
    */
    static void Init();

    /**
     * @brief Destroys the windowing system.
     */
    static void Shutdown();

    /**
     * @brief Poll Events from the OS to facilitate for user input;
    */
    virtual void PollEvents() = 0;

    /**
     * @brief Swap front and back buffers to display the rendered frame to the window and allow for the rendering of a new frame
     * @warning in the case of OpenGL needs to be called on the Render thread
     * @deprecated This will need to be replaced since the vulkan Render API implementation doesn't use it.
    */
    virtual void SwapBuffers() = 0;

    /**
     * @brief Disabled the cursor for the in-game mode
    */
    virtual void DisableCursor() = 0;

    /**
     * @brief Enables the cursor for the editor mode
    */
    virtual void EnableCursor() = 0;

    glm::uvec2 GetResolution() const {
        return m_Properties.resolution;
    }

    virtual glm::ivec2 GetFramebufferResolution() = 0;

    virtual bool IsMinimized() = 0;

#ifdef EDITOR

    /**
     * @brief Set the window in a mode where the game gets displayed as if without an editor
    */
    virtual void AdjustWidowToDisabledEditor() = 0;
    /**
     * @brief Set the window in a mode where the game gets displayed in an Editor
    */
    virtual void AdjustWidowToEnabledEditor() = 0;


#endif

    /**
     * @brief Set Drag and Drop callback
     * @param callback the callback with the paths drag and dropped
    */
    virtual void RegistorDragAndDropCallback(void(*callback)(int count, std::vector<std::string> paths)) = 0;

    /**
     * @brief Get the Render Surface object which wraps the swapchain images and present functionality.
     * 
     * @return The RenderSurface abstraction associated with this window
     */
    virtual std::shared_ptr<RenderSurface> GetRenderSurface() = 0;

    virtual ~Window() {};
public:

    WindowProperties m_Properties; 

    /**
     * @brief Create a Window instance for the current window manager
    */
    static std::shared_ptr<Window> CreateWindow(const WindowProperties& props = WindowProperties());
};
