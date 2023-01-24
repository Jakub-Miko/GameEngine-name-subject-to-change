#include "CollisionModule.h"
#include <Window.h>
#include <World/World.h>
#include <Application.h>
#include <glm/glm.hpp>


extern "C" {


}

void CollisionModule::OnRegisterModule(ModuleBindingProperties& props)
{
    GlobalEntityModule().RegisterModule(props);
    MathModule().RegisterModule(props);

    props.Add_FFI_declarations(R"(
    typedef struct CollisionEvent_L {
		entity collider;
		int num_collision_points;
		vec3 collision_points[4];
	};
    )");

    props.Add_FFI_aliases({
        {"struct CollisionEvent_L", "CollisionEvent"}
        });

}



