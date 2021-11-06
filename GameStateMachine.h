#pragma once 
#include <stack>
#include <memory>

class GameState;
class Event;

class GameStateMachine {
public:
	GameStateMachine(const GameStateMachine& ref) = delete;
	GameStateMachine(GameStateMachine&& ref) = delete;
	GameStateMachine& operator=(const GameStateMachine& ref) = delete;
	GameStateMachine& operator=(GameStateMachine&& ref) = delete;

	static GameStateMachine* Get();
	static void Init();
	static void Shutdown();

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

	std::shared_ptr<GameState> current_state = nullptr;
	std::stack<std::shared_ptr<GameState>> m_States;

};