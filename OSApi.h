#pragma once
#include <string>

/**
 * @brief Abstraction for OS specific operations not handled by the Window Manager.
*/
class OSApi {
public:

    /**
     * @brief Opens a file in its default application
     * @param filepath file to open
     * @return 
    */
    virtual bool OpenFileInDefaultApp(const std::string& filepath) = 0;

    virtual ~OSApi() {};
public:

    /**
     * @brief Create an OSApi instance for the current OS.
    */
    static OSApi* CreateOSApi(); 
};
