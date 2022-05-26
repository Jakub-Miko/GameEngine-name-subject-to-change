#include "GameStateMachine.h"
#include <stdexcept>
#include "GameState.h"
#include <Events/KeyCodes.h>
#include <LuaEngineUtilities.h>
#include <Events/Event.h>
#include <World/Components/ScriptComponent.h>
#include <Application.h>
#include <Events/KeyPressEvent.h>
#include <World/ScriptModules/DefferedPropertySetModule.h>
#include <World/ScriptModules/IOModule.h>
#include <World/ScriptModules/ApplicationDataModule.h>
#include <World/ScriptModules/TimeModule.h>
#include <Events/MouseButtonPressEvent.h>
#include <World/EntityManager.h>

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

void GameStateMachine::UpdateNextState()
{
	if (next_state) {
		if (current_state) {
			ScriptOnDeattach();
			current_state->OnDeattach();
		}
		current_state = next_state;

		ScriptOnAttach();
		current_state->OnAttach();
		next_state = nullptr;
	}
}

void GameStateMachine::PushState(std::shared_ptr<GameState> state)
{
	if (current_state) {
		m_States.push(current_state);
	}
	next_state = state;
}

void GameStateMachine::PopState()
{
	if (m_States.empty()) {
		throw std::runtime_error("No state in the state stack");
	}
	next_state = m_States.top();
	m_States.pop();
}

void GameStateMachine::ChangeState(std::shared_ptr<GameState> state)
{
	next_state = state;
}

void GameStateMachine::ClearStateStack()
{
	std::stack<std::shared_ptr<GameState>>().swap(m_States);
}

void GameStateMachine::UpdateState(float delta_time)
{
	ScriptOnUpdate(delta_time);
	current_state->Update(delta_time);
}

void GameStateMachine::OnEventState(Event* e)
{
	ScriptOnEvent(e);
	current_state->OnEvent(e);
}

GameStateMachine::GameStateMachine() : m_States(), m_LuaEngine(this), m_Loaded_Modules()
{
	BindLuaFunctions();
}

GameStateMachine::~GameStateMachine()
{
	ScriptOnDeattach();
	current_state->OnDeattach();
}

void GameStateMachine::ScriptOnUpdate(float delta_time)
{
	if ((int)current_state->script_event_flags & (int)SCRIPT_FLAGS::UPDATE) {
		m_LuaEngine.CallObject(LuaEngineUtilities::ScriptHash(current_state->script_path).c_str(), "OnUpdate", delta_time);
	}
}

void GameStateMachine::ScriptOnEvent(Event* e)
{
	if ((int)current_state->script_event_flags) {
		EventDispacher dispatcher(e);
		if ((int)current_state->script_event_flags & (int)SCRIPT_FLAGS::KEY_PRESS) {
			dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent* e) {
				m_LuaEngine.CallObject(LuaEngineUtilities::ScriptHash(current_state->script_path).c_str(), "OnKeyPressed", *e);
				return false;
				});
		}
		if ((int)current_state->script_event_flags & (int)SCRIPT_FLAGS::MOUSE_BUTTON_PRESS) {
			dispatcher.Dispatch<MouseButtonPressEvent>([this](MouseButtonPressEvent* e) {
				m_LuaEngine.CallObject(LuaEngineUtilities::ScriptHash(current_state->script_path).c_str(), "OnMouseButtonPressed", *e);
				return false;
				});
		}
	}
}

void GameStateMachine::ScriptOnAttach()
{
	if (current_state->script_path != "") {
		if (m_Loaded_Modules.find(LuaEngineUtilities::ScriptHash(current_state->script_path).c_str()) == m_Loaded_Modules.end()) {
			m_LuaEngine.RunString(LuaEngineUtilities::LoadScript(current_state->script_path));
			m_Loaded_Modules.insert(LuaEngineUtilities::ScriptHash(current_state->script_path));
		}
	}
	
	if ((int)current_state->script_event_flags & (int)SCRIPT_FLAGS::ON_ATTACH) {
		m_LuaEngine.CallObject(LuaEngineUtilities::ScriptHash(current_state->script_path).c_str(), "OnAttach");
	}
}

void GameStateMachine::ScriptOnDeattach()
{
	if ((int)current_state->script_event_flags & (int)SCRIPT_FLAGS::ON_DEATTACH) {
		m_LuaEngine.CallObject(LuaEngineUtilities::ScriptHash(current_state->script_path).c_str(), "OnDeattach");
	}
}

void GameStateMachine::BindLuaFunctions()
{
	m_LuaEngine.RunString(ScriptKeyBindings);
	m_LuaEngine.InitFFI();

	ModuleBindingProperties props;

	DefferedPropertySetModule().RegisterModule(props);
	IOModule().RegisterModule(props);
	ApplicationDataModule().RegisterModule(props);
	TimeModule().RegisterModule(props);

	m_LuaEngine.RegisterModule(props);
}

