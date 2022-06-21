#pragma once
#include <string>
#include <OSApi.h>

class LinuxOSApi : public OSApi {
public:
    virtual bool OpenFileInDefaultApp(const std::string& filepath) override;

    virtual ~LinuxOSApi() {};
public:

   
};
