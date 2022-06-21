#pragma once
#include <string>
#include <OSApi.h>

class WindowsOSApi : public OSApi {
public:
    virtual bool OpenFileInDefaultApp(const std::string& filepath) override;

    virtual ~WindowsOSApi() {};
public:

   
};
