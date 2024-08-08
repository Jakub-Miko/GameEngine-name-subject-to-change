#pragma once
class World;

/**
 * @brief System responsible for Initializing new prefab roots by populating and replicating their prefab hierarchies and their components from their prefab files
 * @param world Reference to a world instance
*/
void EntityConstructionSystem(World& world);

