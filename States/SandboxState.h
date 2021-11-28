#pragma once
#include <GameState.h>
#include <Layer.h>

class SandboxState : public GameState{
public:
	SandboxState();
	virtual void Update(float delta_time) override;
	virtual bool OnEvent(Event* e) override;
	virtual void OnAttach() override;
	virtual void OnDeattach() override;

private:
	Layer* m_TestLayer;
};