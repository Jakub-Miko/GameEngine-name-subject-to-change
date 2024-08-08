#pragma once 
class World;
class KeyPressedEvent;

/**
 * @brief Runs the OnKeyPressed script in the Entities @ref inline_script "inline script"
 * @param world Reference to a world instance
 * @param e the KeyPressedEvent that caused the the script to run 
 * 
 * Runs for every Entity with a KeyPressedScriptComponent
*/
void KeyPressedScriptSystem(World& world, KeyPressedEvent* e);