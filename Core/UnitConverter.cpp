#include "UnitConverter.h"
#include <Application.h>
#include <Window.h>

glm::vec2 UnitConverter::ScreenSpaceToNDC(glm::vec2 screen_space)
{
	auto res = Renderer3D::Get()->GetRenderResolution();
	screen_space.x /= (float)res.x / 2;
	screen_space.y /= (float)res.y / 2;
	screen_space -= glm::vec2(1.0f, 1.0f);
	screen_space.y *= -1;

	return screen_space;
}
