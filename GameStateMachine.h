#pragma once 
#include <stack>
#include <memory>
#include <unordered_set>
#include <functional>
#include <stdexcept>
#include <string>

#include "Core/RuntimeTag.h"

class GameState;
class Event;

/**
 * @brief A singleton State machine used for managing Game Engine bahaviour specific to certain states, and handling transitions between such states.
 * 
 * A Finite State Machine with Pushdown Automaton functionality which manages invoking GameState behaviour and scripts and handles transition between GameStates.
 * @see Gamestate
*/
class GameStateMachine {
public:
	GameStateMachine(const GameStateMachine& ref) = delete;
	GameStateMachine(GameStateMachine&& ref) = delete;
	GameStateMachine& operator=(const GameStateMachine& ref) = delete;
	GameStateMachine& operator=(GameStateMachine&& ref) = delete;

	/**
	 * @brief Singleton getter
	*/
	static GameStateMachine* Get();
	
	/**
	 * @brief Singleton initialization
	*/
	static void Init();

	/**
	 * @brief Singleton destructor
	*/
	static void Shutdown();

	/**
	 * @brief current state is updated according to the next_state set during the last frame.
	 * 
	 * States are not changed immediately when requested, the state change is instead deferred until this function gets called.
	*/
	void UpdateNextState();

	/**
	 * @brief Update state while saving the previous state onto a stack, so it can be resumed later
	*/
	void PushState(std::shared_ptr<GameState> state);
	
	/**
	 * @brief Resume the last previous state on top the stack.
	*/
	void PopState();

	/**
	 * @brief Change state without saving the previous one onto the stack.
	*/
	void ChangeState(std::shared_ptr<GameState> state);

	/**
	 * @brief Clear the state history on the stack.
	*/
	void ClearStateStack();

	/**
	 * @brief Update the current state and optionally its script
	*/
	void UpdateState(float delta_time);

	/**
	 * @brief Propagate an Event to the current state and optionally its script
	*/
	void OnEventState(Event* e);

	/**
	 * @brief Register a GameState subclass under its RuntimeTag, so it can be queried on runtime.
	 * @tparam T GameState subclass to be registered
	*/
	template<typename T>
	void RegisterState() {
		constexpr std::string_view name = RuntimeTag<T>::GetName();
		static_assert(name != "Unidentified");
		state_map.insert(std::make_pair(name, std::make_shared<T>));
	}

	/**
	 * @brief Get a GameState Subclass on runtime with its RuntimeTag.
	 * @param name RuntimeTag of the GameState subclass.
	 * @return Pointer to the queried GameState subclass.
	*/
	std::shared_ptr<GameState> GetStateFromName(const std::string& name) {
		auto fnd = state_map.find(name);
		if (fnd == state_map.end()) throw std::runtime_error("State with name " + name + " wasn't registered");
		std::function<std::shared_ptr<GameState>()> construct = fnd->second;
		return construct();

	}

private:
	static GameStateMachine* instance;
	friend class World;

	GameStateMachine();
	~GameStateMachine();

	/**
	 * @brief Register GameStates subclasses using GameStateMachine::RegisterState
	*/
	void RegisterStates();

	std::shared_ptr<GameState> next_state = nullptr; ///< The requested state @see GameStateMachine::UpdateNextState
	std::shared_ptr<GameState> current_state = nullptr; ///< The current state
	std::stack<std::shared_ptr<GameState>> m_States; ///< The state history stack
	std::unordered_map<std::string, std::function<std::shared_ptr<GameState>()>> state_map; ///< a Map between RuntimeTags and the GameState subclass constructors of the Registered GameStates
};