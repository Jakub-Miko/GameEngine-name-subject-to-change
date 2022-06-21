#pragma once
#include <string>


class OSApi {
public:
    virtual bool OpenFileInDefaultApp(const std::string& filepath) = 0;

    virtual ~OSApi() {};
public:

    static OSApi* CreateOSApi();
};
