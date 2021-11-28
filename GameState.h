#pragma once
#include <Layer.h>
#include <GameStateMachine.h>
#include <string>

class Event;

class GameState {
public:
	friend GameStateMachine;
	GameState(const std::string& script_path, SCRIPT_FLAGS ScriptFlags = SCRIPT_FLAGS::NONE) : script_path(script_path), script_event_flags(ScriptFlags) {}
	GameState() = default;

	virtual void Update(float delta_time) = 0;
	virtual bool OnEvent(Event* e) = 0;
	virtual void OnAttach() = 0;
	virtual void OnDeattach() = 0;
	virtual ~GameState() {};

	Layer* GetGameLayer();

protected:
	std::string script_path = "";
	SCRIPT_FLAGS script_event_flags = SCRIPT_FLAGS::NONE;
};