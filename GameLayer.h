#pragma once
#include "Layer.h"

/**
 * @brief Layer responsible for updating and propagating events to systems required by the Engine.
 * 
 * @todo This is the only Layer which isn't intended to implement custom behaviour, it technically doesn't need to be derived from Layer at all.
*/
class GameLayer : public Layer
{
public:

    /**
     * @brief Process event primarily to fire scripts of entities in the game world on keypresses and mouse presses 
     * @param e 
    */
    virtual void OnEvent(Event* e) override;

    /**
     * @brief Runs during initialization and update and loads the requested scenes into the game World.
    */
    void LoadSystem();
    
    /**
     * @brief Called on update before input polling
     * 
     * Specific to the GameLayer, because the order of system updates is important.
    */
    void PreUpdate(float delta_time);

    /**
     * @brief Called on update before input polling
     * 
     * Most game engine systems are updated here. 
    */
    virtual void OnUpdate(float delta_time) override;

};