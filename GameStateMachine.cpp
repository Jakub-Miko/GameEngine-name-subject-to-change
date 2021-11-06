#include "GameStateMachine.h"
#include <stdexcept>
#include "GameState.h"

GameStateMachine* GameStateMachine::instance = nullptr;

GameStateMachine* GameStateMachine::Get()
{
	return instance;
}

void GameStateMachine::Init()
{
	if (!instance) {
		instance = new GameStateMachine();
	}
}

void GameStateMachine::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

void GameStateMachine::PushState(std::shared_ptr<GameState> state)
{
	if (current_state) {
		m_States.push(current_state);
		current_state->OnDeattach();
	}
	current_state = state;
	current_state->OnAttach();
}

void GameStateMachine::PopState()
{
	if (current_state) {
		current_state->OnDeattach();
	}
	if (m_States.empty()) {
		throw std::runtime_error("No state in the state stack");
	}
	current_state = m_States.top();
	current_state->OnAttach();
}

void GameStateMachine::ChangeState(std::shared_ptr<GameState> state)
{
	if (current_state) {
		current_state->OnDeattach();
	}
	current_state = state;
	current_state->OnAttach();
}

void GameStateMachine::ClearStateStack()
{
	std::stack<std::shared_ptr<GameState>>().swap(m_States);
}

void GameStateMachine::UpdateState(float delta_time)
{
	current_state->Update(delta_time);
}

void GameStateMachine::OnEventState(Event* e)
{
	current_state->OnEvent(e);
}

GameStateMachine::GameStateMachine() : m_States()
{

}

GameStateMachine::~GameStateMachine()
{
	current_state->OnDeattach();
}
