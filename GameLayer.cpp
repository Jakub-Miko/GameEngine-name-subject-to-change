#include "GameLayer.h"
#include <Application.h>
#include <iostream>
#include <TaskSystem.h>
#include <cmath>
#include <Promise.h>
#include <Renderer/Renderer.h>
#include <Input/Input.h>
#include <World/Systems/ScriptSystem.h>
#include <Events/KeyPressEvent.h>
#include <Events/MouseButtonPressEvent.h>
#include <Events/MouseMoveEvent.h>
#include <glm/glm.hpp>
#include <World/World.h>
#include <World/Systems/SquareRenderSystem.h>
#include <World/Systems/BoxRenderer.h>
#include <World/Systems/InitializationSystem.h>
#include <World/Systems/MeshRenderSystem.h>
#include <World/Systems/KeyPressedScriptSystem.h>
#include <World/Systems/MousePressedScriptSystem.h>
#include <Renderer/MeshManager.h>
#include <World/Systems/EntityConstructionSystem.h>
#include <World/SceneGraph.h>



void GameLayer::OnEvent(Event* e) {
    EventDispacher dispatch(e);
    dispatch.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
        KeyPressedScriptSystem(Application::GetWorld(), e);
        return false;
        });

    dispatch.Dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent* e) {
        MousePressedScriptSystem(Application::GetWorld(), e);
        return false;
        });
}


void GameLayer::LoadSystem()
{
    World& world = Application::GetWorld();
    world.LoadSceneSystem();
}

void GameLayer::PreUpdate(float delta_time)
{
    World& world = Application::GetWorld();
    EntityConstructionSystem(world);
    ScriptSystemDefferedSet(world);
}

void GameLayer::OnUpdate(float delta_time) { 
    World& world = Application::GetWorld();
    InitializationSystem(world);
    ScriptSystemUpdate(world, delta_time);
    world.UpdateTransformMatricies();
    world.SetPrimaryEntitySystem();
    MeshManager::Get()->UpdateLoadedMeshes(); // MultiThread
    //SquareRenderSystem(world);
    BoundingVolumeRender(world);
    MeshRenderSystem(world);
}