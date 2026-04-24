#include "GameLayer.h"
#include <Application.h>
#include <iostream>
#include <TaskSystem.h>
#include <cmath>
#include <Promise.h>
#include <Renderer/Renderer.h>
#include <Input/Input.h>
#include <Events/KeyPressEvent.h>
#include <Events/MouseButtonPressEvent.h>
#include <Events/MouseMoveEvent.h>
#include <glm/glm.hpp>
#include <World/World.h>
#include <World/Systems/SquareRenderSystem.h>
#include <World/Systems/BoxRenderer.h>
#include <World/Systems/MeshRenderSystem.h>
#include <Renderer/MeshManager.h>
#include <Renderer/Renderer3D/SkeletalAnimations/SkeletalAnimationManager.h>
#include <Renderer/TextureManager.h>
#include <World/SceneGraph.h>

#include "World/Systems/AnimationSystem.h"


void GameLayer::OnEvent(Event* e) {

}


void GameLayer::LoadSystem()
{
    World& world = Application::GetWorld();
    world.LoadSceneSystem();
}

void GameLayer::PreUpdate(float delta_time)
{
    World& world = Application::GetWorld();
    world.CheckCamera();
}

void GameLayer::OnUpdate(float delta_time) { 
    World& world = Application::GetWorld();
    world.SetPrimaryEntitySystem();
    MeshManager::Get()->UpdateLoadedMeshes(); // MultiThread
    SkeletalAnimationManager::Get()->UpdateLoadedAnimations(); // MultiThread
    AnimationSystem(world, delta_time);
    TextureManager::Get()->UpdateLoadedReflectionMaps();
    world.UpdateTransformMatricies();
    world.DeletionSystem();
    //BoundingVolumeRender(world);
} 