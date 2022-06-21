cmake_minimum_required(VERSION 3.0.0)
project(PseudoTest VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")


set(RENDER_API "" CACHE STRING "")

option(ENABLE_PROFILING OFF)
option(EDITOR ON)

file(GLOB SRCS 
"./*.cpp" "./*.h" 
"./Input/*.cpp"
"./States/*.cpp"
"./World/*.cpp"
"./World/*/*.cpp"
"./Renderer/*.cpp" "./Renderer/*.h"
"./Core/*.cpp"
"./Core/Extensions/*.cpp")

add_executable(PseudoTest ${SRCS})

set_target_properties(PseudoTest PROPERTIES ENABLE_EXPORTS true WINDOWS_EXPORT_ALL_SYMBOLS true)


file(GLOB OPENGL_SRC 
"./platform/OpenGL/*.cpp" "./platform/OpenGL/*.h"
"./platform/OpenGL/*/*.cpp" "./platform/OpenGL/*/*.h"
"./platform/GLFW/*.cpp" "./platform/GLFW/*.h" )

add_subdirectory("./dependencies/glfw")

add_subdirectory("./dependencies/EnTT")

add_subdirectory("./dependencies/glew/build/cmake")

add_subdirectory("./dependencies/glm")

set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT off)
set(ASSIMP_BUILD_ASSIMP_TOOLS off)
set(ASSIMP_BUILD_TESTS off)
set(ASSIMP_INSTALL off)

add_subdirectory("./dependencies/assimp")

add_subdirectory("./dependencies/Additional")

add_subdirectory("./Utilities")

target_include_directories(glew_s PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dependencies/glew/include>")

target_link_libraries(PseudoTest PRIVATE glfw glew_s glm Profiler MemoryManagement 
TaskSystem LuaEngine RuntimeTypeIdentifier ThreadManager FileManager ConfigManager EnTT::EnTT RuntimeTypeIdentifier JsonParser STB_Image assimp)


if(EDITOR)

file(GLOB EDITOR_FILES
 "./Editor/*.cpp"  "./Editor/*.h"
 "./Editor/*/*.cpp" "./Editor/*/*.h")

target_sources(PseudoTest PRIVATE ${EDITOR_FILES})

target_compile_definitions(PseudoTest PRIVATE EDITOR)

target_link_libraries(PseudoTest PRIVATE IMGUI)

endif()


if(WIN32) 

set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL" "DirectX12")

file(GLOB DIRECTX_SRC
"./platform/DirectX/*.cpp" "./platform/DirectX/*.h"
"./platform/DirectX/*/*.cpp" "./platform/DirectX/*/*.h"
"./platform/Windows/*.cpp" "./platform/Windows/*.h")

target_compile_definitions(PseudoTest PRIVATE "WINDOWS")

target_sources(PseudoTest PRIVATE ${DIRECTX_SRC} ${OPENGL_SRC})

target_link_libraries(PseudoTest PRIVATE Shlwapi.lib )

endif()

if(UNIX AND NOT APPLE)
    
set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL")

target_compile_definitions(PseudoTest PRIVATE "LINUX")

file(GLOB LINUX_SRC
"./platform/Linux/*.cpp" "./platform/Linux/*.h"
)

target_sources(PseudoTest PRIVATE ${LINUX_SRC})

endif()



if(ENABLE_PROFILING)

target_compile_definitions(PseudoTest PRIVATE ENABLE_PROFILING)

endif()

target_include_directories(PseudoTest PRIVATE .)

target_compile_definitions(PseudoTest PRIVATE ${RENDER_API})
