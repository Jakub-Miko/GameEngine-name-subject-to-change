#pragma once
#include <GameState.h>

class SandboxState : public GameState {
	virtual void Update(float delta_time) override;
	virtual bool OnEvent(Event* e) override;
	virtual void OnAttach() override;
	virtual void OnDeattach() override;
};