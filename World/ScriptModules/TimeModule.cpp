#include "TimeModule.h"
#include <glm/glm.hpp>
#include <thread>
#include <chrono>
#include <glm/gtc/quaternion.hpp>
#include <Core/Defines.h>

extern "C" {

	/**
	 * @brief Pauses the execution for a certain time
	 * @param miliseconds the number of milliseconds to wait
	 * 
	 * @warning This is a blocking call and it will block the execution of the script and thus block the main game thread. So it should not be used. 
	 * 
	 * @deprecated This function does not make much sense and will just cause trouble in the future so it will be removed or replaced eventually.
	 * 
	 * @lua
	 */
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
