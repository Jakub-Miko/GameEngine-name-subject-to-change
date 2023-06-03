#include "TimeModule.h"
#include <glm/glm.hpp>
#include <thread>
#include <chrono>
#include <glm/gtc/quaternion.hpp>
#include <Core/Defines.h>

extern "C" {
	LIBEXP void wait_milliseconds_L(int miliseconds) {
		std::this_thread::sleep_for(std::chrono::milliseconds(miliseconds));
	}
}

void TimeModule::OnRegisterModule(ModuleBindingProperties& props)
{
	props.Add_FFI_declarations(R"(
		void wait_milliseconds_L(int miliseconds);
	)");

	props.Add_FFI_aliases({
		{"wait_milliseconds_L","wait_milliseconds"}
	});

}
