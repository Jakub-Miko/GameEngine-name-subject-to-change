#include "RayCastingModule.h"
#include "LocalEntityModule.h"
#include "MathModule.h"
#include <Core/Defines.h>

extern "C" {

	/**
	 * @brief Encapsulates the Results of a single Ray intersection in lua scripts
	 */
	LIBEXP typedef struct RayCastingResult_L {
		entity hit_ent;
		vec3 hit_pos;
	} RayCastingResult_L;

	/**
	 * @brief Encapsulates all intersections of a RayCast in lua scripts
	 */
	LIBEXP typedef struct RayCastingResultArray_L {
		int size;
		RayCastingResult_L* hit_points;
	} RayCastingResultArray_L;
	

	/**
	 * @brief Encapsulates the Results of a single Ray intersection in lua scripts
	 * 
	 * @note unlike RayCastingResult_L this is used with PhysicsEngine based Raycasts not SpatialIndex based Raycasts. 
	 * It provides more detailed information but can only be used on objects with a PhysicsComponent
	 */
	LIBEXP typedef struct RayCastingResultPhysics_L {
		entity hit_ent;
		vec3 hit_pos;
		vec3 hit_normal;
	} RayCastingResultPhysics_L;

	/**
	 * @brief Encapsulates all intersections of a RayCast in lua scripts
	 * 
	 * @note unlike RayCastingResult_L this is used with PhysicsEngine based Raycasts not SpatialIndex based Raycasts. 
	 * It provides more detailed information but can only be used on objects with a PhysicsComponent
	 */
	LIBEXP typedef struct RayCastingResultPhysicsArray_L {
		int size;
		RayCastingResultPhysics_L* hit_points;
	} RayCastingResultPhysicsArray_L;


	/**
	 * @brief Performs a SpatialIndex based Raycast and returns all detected intersections
	 * 
	 * SpatialIndex based Raycasts can be performed against any objects with an Entry in the SpatialIndex, but are only performed against their BoundingVolumeComponent and as such cannot return 
	 * precise hit position and hit normals.
	 * 
	 * @param start The origin of the Ray
	 * @param dir The direction the ray is pointing
	 * @return A RayCastingResultArray_L containing all detected Ray Intersections.
	 * @lua
	 */
	LIBEXP RayCastingResultArray_L RayCast_L(vec3 start, vec3 dir) {
		glm::vec3 origin = *reinterpret_cast<glm::vec3*>(&start);
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

	/**
	 * @brief Performs a PhysicsEngine based Raycast and returns all detected intersections
	 * 
	 * Unlike RayCast_L this function utilizes the PhysicsEngine to test against all Entities with a PhysicsComponent, and as such cannot be used to test against objects without it.
	 * It however provides more precise results as well as hit normals.
	 * 
	 * @param start_in The start point of the Ray
	 * @param end_in The end point of the Ray
	 * @return A RayCastingResultPhysicsArray_L containing all detected Ray Intersections
	 * @lua
	 */
	LIBEXP RayCastingResultPhysicsArray_L RayCastPhysics_L(vec3 start_in , vec3 end_in) {
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

	/**
	 * @brief Performs a PhysicsEngine based Raycast and returns the closest detected intersection
	 * 
	 * Unlike RayCast_L this function utilizes the PhysicsEngine to test against all Entities with a PhysicsComponent, and as such cannot be used to test against objects without it.
	 * It however provides more precise results as well as hit normals.
	 * 
	 * Unlike RayCastPhysics_L this function only returns the closest intersection.
	 * 
	 * @param start_in The start point of the Ray
	 * @param end_in The end point of the Ray
	 * @return A RayCastingResultPhysics_L containing the closest detected Ray Intersection
	 * @lua
	 */
	LIBEXP RayCastingResultPhysics_L RayCastPhysicsClosest_L(vec3 start_in, vec3 end_in) {
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

	/**
	 * @brief A destructor for RayCastingResultArray_L
	 * @param arr the instance to destroy
	 * @lua
	 */
	LIBEXP void FreeArray_L(RayCastingResultArray_L arr) {
 		delete[] arr.hit_points;
	}

	/**
	 * @brief A destructor for RayCastingResultPhysicsArray_L
	 * @param arr the instance to destroy
	 * @lua
	 */
	LIBEXP void FreeArrayPhysics_L(RayCastingResultPhysicsArray_L arr) {
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
