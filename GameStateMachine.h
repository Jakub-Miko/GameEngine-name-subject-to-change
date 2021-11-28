#pragma once 
#include <stack>
#include <memory>
#include <LuaEngine.h>
#include <unordered_set>
#include <string>

class GameState;
class Event;

enum class SCRIPT_FLAGS : unsigned char {
	NONE = 0,
	UPDATE = 1,
	KEY_PRESS = 2,
	MOUSE_BUTTON_PRESS = 4,
	ON_ATTACH = 8,
	ON_DEATTACH = 16
};

inline SCRIPT_FLAGS operator | (SCRIPT_FLAGS lhs, SCRIPT_FLAGS rhs)
{
	return static_cast<SCRIPT_FLAGS>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline SCRIPT_FLAGS& operator |= (SCRIPT_FLAGS& lhs, SCRIPT_FLAGS rhs)
{
	lhs = lhs | rhs;
	return lhs;
}



class GameStateMachine {
public:
	GameStateMachine(const GameStateMachine& ref) = delete;
	GameStateMachine(GameStateMachine&& ref) = delete;
	GameStateMachine& operator=(const GameStateMachine& ref) = delete;
	GameStateMachine& operator=(GameStateMachine&& ref) = delete;

	static GameStateMachine* Get();
	static void Init();
	static void Shutdown();

	void UpdateNextState();

	void PushState(std::shared_ptr<GameState> state);
	void PopState();

	void ChangeState(std::shared_ptr<GameState> state);
	void ClearStateStack();

	void UpdateState(float delta_time);
	void OnEventState(Event* e);

private:
	static GameStateMachine* instance;

	GameStateMachine();
	~GameStateMachine();

	void ScriptOnUpdate(float delta_time);
	void ScriptOnEvent(Event* e);


	void ScriptOnAttach();
	void ScriptOnDeattach();

	std::shared_ptr<GameState> next_state = nullptr;
	std::shared_ptr<GameState> current_state = nullptr;
	std::stack<std::shared_ptr<GameState>> m_States;

	LuaEngineClass<GameStateMachine> m_LuaEngine;
	std::unordered_set<std::string> m_Loaded_Modules;

private:
	void BindLuaFunctions();
	//Lua Bindings




};