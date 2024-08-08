#pragma once
class World;

/**
 * @brief Updates audio object positions to match Entity Transforms and Playback properties in their AudioComponents,
 * as well as updating the Listener position to sync with the Primary Entity
 * @param world world containing Entities with AudioComponents to update
*/
void AudioUpdateSystem(World& world);