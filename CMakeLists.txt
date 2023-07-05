cmake_minimum_required(VERSION 3.0.0)
project(EngineCore VERSION 0.1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(BUILD_BULLET2_DEMOS on CACHE BOOL "" FORCE)
set(BUILD_BULLET_ROBOTICS_EXTRA on CACHE BOOL "" FORCE)
set(BUILD_BULLET_ROBOTICS_GUI_EXTRA on CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS on CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS on CACHE BOOL "" FORCE)
set(BUILD_EXTRAS on CACHE BOOL "" FORCE)
set(CMAKE_DEBUG_POSTFIX "") 

add_compile_definitions("${RENDER_API}_API")

if(UNIX)
  add_compile_definitions(UNIX=1)
endif(UNIX)

set(RENDER_API "" CACHE STRING "")
set(MAX_NUM_OF_BONES "100" CACHE STRING "")

option(ENABLE_PROFILING OFF)
option(EDITOR ON)

file(GLOB SRCS 
"./*.cpp" "./*.h" 
"./Input/*.cpp"
"./States/*.cpp"
"./World/*.cpp"
"./World/*/*.cpp"
"./Audio/*.cpp"
"./Audio/*/*.cpp"
"./Renderer/*.cpp" "./Renderer/*.h"
"./Renderer/Renderer3D/*.cpp" "./Renderer/Renderer3D/*.h"
"./Renderer/Renderer3D/RenderPasses/*.cpp" "./Renderer/Renderer3D/RenderPasses/*.h"
"./Renderer/Renderer3D/Animations/*.cpp" "./Renderer/Renderer3D/Animations/*.h"
"./Core/*.cpp"
"./Core/Extensions/*.cpp")


file(GLOB EXE_FILES
 "./Entry/*.cpp")


add_executable(GameEngine ${EXE_FILES})
add_library(EngineCore SHARED ${SRCS})
set_target_properties(EngineCore PROPERTIES OUTPUT_NAME "EngineCore")

set_target_properties(EngineCore PROPERTIES ENABLE_EXPORTS true WINDOWS_EXPORT_ALL_SYMBOLS true)


file(GLOB OPENGL_SRC 
"./platform/OpenGL/*.cpp" "./platform/OpenGL/*.h"
"./platform/OpenGL/*/*.cpp" "./platform/OpenGL/*/*.h"
"./platform/GLFW/*.cpp" "./platform/GLFW/*.h" )

set(FT_DISABLE_ZLIB on CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS on)
add_subdirectory("./dependencies/FreeType")
add_subdirectory("./dependencies/glfw")

add_subdirectory("./dependencies/EnTT")

set(ALSOFT_INSTALL off CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES off CACHE BOOL "" FORCE)


add_subdirectory("./dependencies/glew/build/cmake")

add_subdirectory("./dependencies/glm")

set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT off)
set(ASSIMP_BUILD_ASSIMP_TOOLS off)
set(ASSIMP_BUILD_TESTS off)
set(ASSIMP_INSTALL off)

add_subdirectory("./dependencies")
add_subdirectory("./dependencies/assimp")

add_subdirectory("./dependencies/Additional")

add_subdirectory("./dependencies/OpenAL")
set(BUILD_SHARED_LIBS off)
add_subdirectory("./Utilities")
set(BUILD_SHARED_LIBS on)

target_include_directories(glew_s PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dependencies/glew/include>")

target_link_libraries(EngineCore PUBLIC glfw glew_s glm Profiler MemoryManagement 
TaskSystem LuaEngine RuntimeTypeIdentifier ThreadManager FileManager ConfigManager EnTT::EnTT RuntimeTypeIdentifier JsonParser STB_Image assimp OpenAL ImGuizmo
Bullet freetype)

target_compile_definitions(EngineCore PRIVATE BUILDDLL ENGINE_CORE)

if(EDITOR)

file(GLOB EDITOR_FILES
 "./Editor/*.cpp"  "./Editor/*.h"
 "./Editor/*/*.cpp" "./Editor/*/*.h")

target_sources(EngineCore PRIVATE ${EDITOR_FILES})

target_compile_definitions(EngineCore PRIVATE EDITOR)

target_link_libraries(EngineCore PUBLIC IMGUI)

endif()


if(WIN32) 

set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL" "DirectX12")

file(GLOB DIRECTX_SRC
"./platform/DirectX/*.cpp" "./platform/DirectX/*.h"
"./platform/DirectX/*/*.cpp" "./platform/DirectX/*/*.h"
"./platform/Windows/*.cpp" "./platform/Windows/*.h")

target_compile_definitions(EngineCore PRIVATE "WINDOWS")

target_sources(EngineCore PRIVATE ${DIRECTX_SRC} ${OPENGL_SRC})

target_link_libraries(EngineCore PRIVATE Shlwapi.lib )

endif()

if(UNIX AND NOT APPLE)
    
set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL")

target_compile_definitions(EngineCore PRIVATE "LINUX")

file(GLOB LINUX_SRC
"./platform/Linux/*.cpp" "./platform/Linux/*.h"
)

target_sources(EngineCore PRIVATE ${LINUX_SRC} ${OPENGL_SRC})

endif()



if(ENABLE_PROFILING)

target_compile_definitions(EngineCore PRIVATE ENABLE_PROFILING)

endif()

target_include_directories(EngineCore PUBLIC .)

target_compile_definitions(EngineCore PRIVATE ${RENDER_API} MAX_NUM_OF_BONES=${MAX_NUM_OF_BONES})


target_link_libraries(GameEngine PUBLIC EngineCore)

add_library(TestLib SHARED "./TestModule/TestFunc.cpp")
target_link_libraries(TestLib PUBLIC EngineCore)
target_compile_definitions(TestLib PRIVATE BUILDDLL)
