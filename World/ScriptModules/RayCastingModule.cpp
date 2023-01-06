#include "RayCastingModule.h"
#include "LocalEntityModule.h"
#include "MathModule.h"

extern "C" {

	typedef struct RayCastingResult_L {
		entity hit_ent;
		vec3 hit_pos;
	} RayCastingResult_L;

	typedef struct RayCastingResultArray_L {
		int size;
		RayCastingResult_L* hit_points;
	} RayCastingResultArray_L;
	

	RayCastingResultArray_L TestRaycast_L() {
		RayCastingResult_L* arr = new RayCastingResult_L[5];
		for (int i = 0; i < 5; i++) {
			arr[i] = RayCastingResult_L{ entity{ (unsigned int)i }, vec3{ 1.0f,1.0f,1.0f }
		};
		}
		return RayCastingResultArray_L{ 5, arr };
	}

	RayCastingResultArray_L RayCast_L(vec3 or, vec3 dir) {
		glm::vec3 origin = *reinterpret_cast<glm::vec3*>(&or);
		glm::vec3 direction = *reinterpret_cast<glm::vec3*>(&dir);
		std::vector<RayCastResult> results;
		Application::GetWorld().GetSpatialIndex().RayCast(Application::GetWorld(), Ray{ origin, direction }, results);

		RayCastingResult_L* arr = new RayCastingResult_L[results.size()];
		for (int i = 0; i < results.size(); i++) {
			vec3 hit = *reinterpret_cast<vec3*>(&(results[i].hit_pos));
			arr[i] = RayCastingResult_L{ entity{ results[i].hit_ent.id }, hit };
		}
		return RayCastingResultArray_L{ (int)results.size(), arr };
	}

	void FreeArray_L(RayCastingResultArray_L arr) {
 		delete[] arr.hit_points;
	}
}

void RayCastingModule::OnRegisterModule(ModuleBindingProperties& props)
{
	MathModule().RegisterModule(props);
	LocalEntityModule().RegisterModule(props);

	props.Add_FFI_declarations(R"(
	typedef struct RayCastingResult_L {
		entity hit_ent;
		vec3 hit_pos;
	} RayCastingResult_L;

	typedef struct RayCastingResultArray_L {
		int size;
		RayCastingResult_L* hit_points;
	} RayCastingResultArray_L;
	
	void FreeArray_L(RayCastingResultArray_L arr);
	RayCastingResultArray_L TestRaycast_L();
	RayCastingResultArray_L RayCast_L(vec3 or, vec3 dir);
	)");

	props.Add_FFI_aliases({
		{"struct RayCastingResult_L", "RayCastingResult"},
		{"struct RayCastingResultArray_L", "RayCastingResultArray"},
		{"FreeArray_L","FreeArray"},
		{"TestRaycast_L", "TestRaycast_L_Unmanaged"},
		{"RayCast_L","RayCast_L_Unmanaged"}
		});


	props.Add_init_script(R"(
		function TestRaycast() 
			return ffi.gc(TestRaycast_L_Unmanaged(), FreeArray)
		end

		function RayCast(origin, direction) 
			return ffi.gc(RayCast_L_Unmanaged(origin,direction), FreeArray)
		end

		mt = {
			__call = function(table) 
				local i = 0
				return function()
					if i < table.size then
					temp = i
					i = i + 1 
					return table.hit_points[temp]
					else 
					return nil
					end
					
				end				

			end

		}

		ffi.metatype("RayCastingResultArray_L",  mt)

	)");
}
