#pragma once
class Event;

/**
 * @brief Abstraction for custom behaviour implemented into the GameEngine
 * 
 * Not used extensively currently, but may be used in the future to encapsulate behavior supplied in runtime modules.
*/
class Layer {
public:
    /**
     * @brief Custom behaviour for processing events.
    */
    virtual void OnEvent(Event* e) {};

    /**
     * @brief Custom update behaviour.
     */
    virtual void OnUpdate(float delta_time) = 0;

    virtual ~Layer() {  };
};