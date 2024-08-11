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
 * @brief Runs a Script function of an entity which was called from another multithreaded Entity script in earlier stages, but was deffered since it was called by a different Entity
 * and as such could not be executed in a thread-safe manner immediately 
 * @param world Reference to a world instance
 * 
 * ScriptSystemDefferedCall runs in multiple iteration, on each iteration one list of deffered calls is read from, and another is written to, when a deffered call spawns another deffered call.
 * When one iteration ends and the list used for writing is not empty, the writing and reading lists are swapped and another iteration begins.
 * The reading list used in the first iteration also contains deffered calls from earlier script stages such as OnUpdate.
 * Iterations end when the last iteration didn't write any other deffered call.
 * @ref ScriptSystemManager maintains both these alternating lists (actually sets of list, one list stores @ref ScriptSystemManager::m_Pending_Deffered_call_vectors "entities",
 * another stores @ref ScriptSystemManager::m_Deffered_call_maps "calls for entities").
 * 
 * @see DefferedPropertySetModule
 * @bug This can cause infinite loops when deffered calls cyclically spawn more deffered calls, implement max counter and set scripts to error state
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