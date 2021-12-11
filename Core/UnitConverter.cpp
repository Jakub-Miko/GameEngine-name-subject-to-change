#include "UnitConverter.h"
#include <Application.h>
#include <Window.h>

glm::vec2 UnitConverter::ScreenSpaceToNDC(glm::vec2 screen_space)
{
	WindowProperties props = Application::Get()->GetWindow()->m_Properties;
	screen_space.x /= (float)props.resolution_x / 2;
	screen_space.y /= (float)props.resolution_y / 2;
	screen_space -= glm::vec2(1.0f, 1.0f);
	screen_space.y *= -1;

	return screen_space;
}
