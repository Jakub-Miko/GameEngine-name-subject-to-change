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
	
	typedef struct RayCastingResultPhysics_L {
		entity hit_ent;
		vec3 hit_pos;
		vec3 hit_normal;
	} RayCastingResultPhysics_L;

	typedef struct RayCastingResultPhysicsArray_L {
		int size;
		RayCastingResultPhysics_L* hit_points;
	} RayCastingResultPhysicsArray_L;


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

	RayCastingResultPhysicsArray_L RayCastPhysics_L(vec3 start_in , vec3 end_in) {
		glm::vec3 start = *reinterpret_cast<glm::vec3*>(&start_in);
		glm::vec3 end = *reinterpret_cast<glm::vec3*>(&end_in);
		PhysicsRayTestResultArray results;
		Application::GetWorld().GetPhysicsEngine().RayCast(start, end, results);

		RayCastingResultPhysics_L* arr = new RayCastingResultPhysics_L[results.size()];
		for (int i = 0; i < results.size(); i++) {
			vec3 hit = *reinterpret_cast<vec3*>(&(results[i].position));
			vec3 normal = *reinterpret_cast<vec3*>(&(results[i].normal));
			arr[i] = RayCastingResultPhysics_L{ entity{ results[i].ent.id }, hit, normal };
		}
		return RayCastingResultPhysicsArray_L{ (int)results.size(), arr };
	}

	RayCastingResultPhysics_L RayCastPhysicsClosest_L(vec3 start_in, vec3 end_in) {
		glm::vec3 start = *reinterpret_cast<glm::vec3*>(&start_in);
		glm::vec3 end = *reinterpret_cast<glm::vec3*>(&end_in);
		PhysicsRayTestResult result;
		bool succeeded = Application::GetWorld().GetPhysicsEngine().RayCastSingle(start, end, result);
		vec3 hit = *reinterpret_cast<vec3*>(&(result.position));
		vec3 normal = *reinterpret_cast<vec3*>(&(result.normal));
		if (succeeded) {
			return RayCastingResultPhysics_L{ entity{result.ent.id}, hit,normal};
		}
		else {
			return RayCastingResultPhysics_L{ entity{Entity().id}, vec3{0.0f,0.0f,0.0f},vec3{0.0f,1.0f,0.0f} };
		}
	}

	void FreeArray_L(RayCastingResultArray_L arr) {
 		delete[] arr.hit_points;
	}
	void FreeArrayPhysics_L(RayCastingResultPhysicsArray_L arr) {
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

		
	typedef struct RayCastingResultPhysics_L {
		entity hit_ent;
		vec3 hit_pos;
		vec3 hit_normal;
	} RayCastingResultPhysics_L;

	typedef struct RayCastingResultPhysicsArray_L {
		int size;
		RayCastingResultPhysics_L* hit_points;
	} RayCastingResultPhysicsArray_L;

	

	void FreeArray_L(RayCastingResultArray_L arr);
	void FreeArrayPhysics_L(RayCastingResultPhysicsArray_L arr);
	RayCastingResultArray_L RayCast_L(vec3 or, vec3 dir);
	RayCastingResultPhysicsArray_L RayCastPhysics_L(vec3 start_in , vec3 end_in);
	RayCastingResultPhysics_L RayCastPhysicsClosest_L(vec3 start_in, vec3 end_in);
	)");

	props.Add_FFI_aliases({
		{"struct RayCastingResult_L", "RayCastingResult"},
		{"struct RayCastingResultArray_L", "RayCastingResultArray"},
		{"struct RayCastingResultPhysics_L", "RayCastingResultPhysics"},
		{"struct RayCastingResultPhysicsArray_L", "RayCastingResultPhysicsArray"},
		{"FreeArray_L","FreeArray"},
		{"FreeArrayPhysics_L","FreeArrayPhysics"},
		{"RayCast_L","RayCast_L_Unmanaged"},
		{"RayCastPhysics_L","RayCastPhysics_L_Unmanaged"},
		{"RayCastPhysicsClosest_L", "RayCastPhysicsClosest"}
		});


	props.Add_init_script(R"(

		function RayCast(origin, direction) 
			return ffi.gc(RayCast_L_Unmanaged(origin,direction), FreeArray)
		end

		function RayCastPhysics(start_point, end_point) 
			return ffi.gc(RayCastPhysics_L_Unmanaged(start_point,end_point), FreeArrayPhysics)
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
		ffi.metatype("RayCastingResultPhysicsArray_L",  mt)

	)");
}
