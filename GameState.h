#pragma once
#include <Layer.h>
#include <GameStateMachine.h>

class Event;

class GameState {
public:
	virtual void Update(float delta_time) = 0;
	virtual bool OnEvent(Event* e) = 0;
	virtual void OnAttach() = 0;
	virtual void OnDeattach() = 0;
	virtual ~GameState() {};

	Layer* GetGameLayer();

};