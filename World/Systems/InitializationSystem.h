#pragma once 
class World;

/** 
 * @brief Runs the OnStart script in the Entities @ref inline_script "inline script"
 * @param world Reference to a world instance
 * 
 * Runs for every Entity with a InitializationComponent, and then removes it. (This component is assigned when an Entity is created).
 * If an Entity has the InitializationComponent and no OnStart script, then nothing happens and InitializationComponent is just removed as normal.
*/
void InitializationSystem(World& world);