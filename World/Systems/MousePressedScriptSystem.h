#pragma once 
class World;
class MouseButtonPressEvent;

/**
 * @brief Runs the OnMouseButtonPressed script in the Entities @ref inline_script "inline script"
 * @param world Reference to a world instance
 * @param e the MouseButtonPressEvent that caused the the script to run
 *
 * Runs for every Entity with a MousePressedScriptComponent
*/
void MousePressedScriptSystem(World& world, MouseButtonPressEvent* e);