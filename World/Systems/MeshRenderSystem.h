#pragma once 
class World;

/**
 * @brief Renders all Entites containing the MeshComponent to the scene from the perspective of the primary entity with static lighting
 * @param world Reference to a world instance
 * 
 * @deprecated This is no longer used and was replaced by a proper deferred rendering pipeline, check Renderer3D
*/
void MeshRenderSystem(World& world);

/**
 * @brief Initializes the resources needed by the MeshRenderSystem
*/
void InitMeshRenderSystem();

/**
 * @brief Clears up the resources needed by the MeshRenderSystem
*/
void SutdownMeshRenderSystem();