#pragma once
class World;

/**
 * @brief Runs the OnUpdate scripts in the Entities @ref inline_script "inline script" 
 * @param world Reference to a world instance
 * @param delta_time The time passed between the last and the current frame
 * 
 * Runs for every Entity with a ScriptComponent
*/
void ScriptSystemUpdate(World& world, float delta_time);

/**
 * @brief Sets the DynamicPropertiesComponent properties, which have been set during the multithreaded script updates, 
 * but have been deffered due to being called across different Entities and as such could not be executed in a thread-safe manner immediately
 * @param world Reference to a world instance
 * 
 * Runs for every Entity with a DefferedUpdateComponent 
*/
void ScriptSystemDefferedSet(World& world);

/**
 * @brief Runs a Script function of an entity, which was called from another multithreaded Entity script, but was deffered since it was called by a different Entity
 * and as such could not be executed in a thread-safe manner immediately
 * @param world Reference to a world instance
 *
 * The list of entities to run is managed by the ScriptSystemManager
*/
void ScriptSystemDefferedCall(World& world);

/**
 * @brief Runs the OnCollision Scripts in Entities @ref inline_script "inline script" 
 * @param world Reference to a world instance
 * 
 * These Scripts are run for Entities with the PhysicsObjectProperties::RECIEVE_COLLISION_EVENTS flag set in their PhysicsComponent,
 * contain a OnCollision function in their @ref inline_script "inline script" and have experienced collisions in the last timestep
*/
void ScriptSystemCollisionCallback(World& world);