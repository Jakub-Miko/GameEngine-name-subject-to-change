#include "GameState.h"
#include <Application.h>

Layer* GameState::GetGameLayer() {
	return Application::Get()->m_GameLayer;
}