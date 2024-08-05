#pragma once
#include <Layer.h>
#include <GameStateMachine.h>
#include <string>

class Event;

/**
 * @brief Encapsulates the game state in the GameStateMachine
*/
class GameState {
public:
	friend GameStateMachine;
	/**
	 * @brief Game state base constructor.
	 * @param script_path Lua script which allows some GameState logic to be implemented in scripts. If ScriptFlags is NONE script_path can be empty .
	 * @param ScriptFlags Specifies what events should dispatch the Lua script (i.e. Update, KeyPress, MousePress, Attach, Deattach etc.), default to NONE.
	*/
	GameState(const std::string& script_path, SCRIPT_FLAGS ScriptFlags = SCRIPT_FLAGS::NONE) : script_path(script_path), script_event_flags(ScriptFlags) {}
	/**
	 * @brief Game state default base constructor
	 * 
	 * script_path and ScriptFlags are implicitly empty and NONE respectively.
	*/
	GameState() = default;

	/**
	 * @brief Update for GameState specific behaviour
	 * @param delta_time 
	*/
	virtual void Update(float delta_time) = 0;
	
	/**
	 * @brief Event processing for GameState specific behaviour
	 * @param e event to be processed
	 * @return whether the event was marked as processed (can have different implications for different events)
	*/
	virtual bool OnEvent(Event* e) = 0;

	/**
	 * @brief Behaviour for when this GameState is entered.
	*/
	virtual void OnAttach() = 0;

	/**
	 * @brief Behaviour for when this GameState is exited.
	*/
	virtual void OnDeattach() = 0;


	virtual ~GameState() {};

	/**
	 * @brief Get the main GameLayer from the Application, not used anymore, may be removed.
	*/
	Layer* GetGameLayer();

protected:
	std::string script_path = ""; ///< Path for the Lua script file associated with the GameState
	SCRIPT_FLAGS script_event_flags = SCRIPT_FLAGS::NONE; ///< Flags specifying what functionality the Lua script provides.
};