#pragma once
#include <string>
#include <vector>

/**
 * @brief Properties of the window. Descriptor object for window creation.
*/
struct WindowProperties {
    int resolution_x = -1;  ///< window resolution x
    int resolution_y = -1; ///< window resolution y
    std::string name = "GameEngine NSTC"; ///< Window name
    bool fullscreen = false; ///< Fullscreen mode
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
     * @brief First phase of window creation, which handles creating the window.
    */
    virtual void PreInit() = 0;

    /**
     * @brief Second phase of window creation, which sets callbacks and binds the the window manager provided resources with the Renderer (i.e. bind the OpenGL context)
    */
    virtual void Init() = 0;

    /**
     * @brief Poll Events from the OS to facilitate for user input;
    */
    virtual void PollEvents() = 0;

    /**
     * @brief Getter for WindowProperties
    */
    const WindowProperties& GetProperties() const {
        return m_Properties;
    }

    /**
     * @brief Swap front and back buffers to display the rendered frame to the window and allow for the rendering of a new frame
     * @warning in the case of OpenGL needs to be called on the Render thread
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

    virtual ~Window() {};
public:

    WindowProperties m_Properties; 

    /**
     * @brief Create a Window instance for the current window manager
    */
    static Window* CreateWindow(const WindowProperties& props = WindowProperties());
};
