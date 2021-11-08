cmake_minimum_required(VERSION 3.0.0)
project(PseudoTest VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")

set(RENDER_API "" CACHE STRING "")

option(ENABLE_PROFILING OFF)

file(GLOB SRCS 
"./*.cpp" "./*.h" 
"./Input/*.cpp"
"./States/*.cpp"
"./World/*.cpp"
"./World/*/*.cpp"
"./Renderer/*.cpp" "./Renderer/*.h"
"./Core/*.cpp")

add_executable(PseudoTest ${SRCS})

file(GLOB OPENGL_SRC 
"./platform/OpenGL/*.cpp" "./platform/OpenGL/*.h"
"./platform/OpenGL/*/*.cpp" "./platform/OpenGL/*/*.h"
"./platform/GLFW/*.cpp" "./platform/GLFW/*.h" )

add_subdirectory("./dependencies/glfw")

add_subdirectory("./dependencies/EnTT")

add_subdirectory("./dependencies/glew/build/cmake")

add_subdirectory("./dependencies/glm")

add_subdirectory("./Utilities")

target_include_directories(glew_s PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dependencies/glew/include>")

target_link_libraries(PseudoTest PRIVATE glfw glew_s glm Profiler MemoryManagement 
TaskSystem LuaEngine RuntimeTypeIdentifier ThreadManager FileManager ConfigManager EnTT::EnTT)



if(WIN32) 

set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL" "DirectX12")

file(GLOB DIRECTX_SRC
"./platform/DirectX/*.cpp" "./platform/DirectX/*.h"
"./platform/DirectX/*/*.cpp" "./platform/DirectX/*/*.h"
"./platform/Windows/*.cpp" "./platform/Windows/*.h" )

target_sources(PseudoTest PRIVATE ${DIRECTX_SRC} ${OPENGL_SRC})

else()

set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL")

target_sources(PseudoTest PRIVATE ${OPENGL_SRC})

endif()



if(ENABLE_PROFILING)

target_compile_definitions(PseudoTest PRIVATE ENABLE_PROFILING)

endif()

target_include_directories(PseudoTest PRIVATE .)

target_compile_definitions(PseudoTest PRIVATE ${RENDER_API})
