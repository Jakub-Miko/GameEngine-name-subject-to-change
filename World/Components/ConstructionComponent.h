#pragma once
#include <Core/RuntimeTag.h>
#include <string>
#include <World/Entity.h>

class ConstructionComponent {
    RUNTIME_TAG("ConstructionComponent")
public:
    ConstructionComponent(const std::string& path, Entity parent) : path(path), parent(parent) {}
    Entity parent;
    std::string path;
};