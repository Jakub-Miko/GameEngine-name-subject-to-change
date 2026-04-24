cmake_minimum_required(VERSION 3.15.0)
cmake_policy(SET CMP0091 OLD)


project(EngineCore VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/out")
set(BUILD_CPU_DEMOS on CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS on CACHE BOOL "" FORCE)
set(BUILD_EXTRAS on CACHE BOOL "" FORCE)
set(CMAKE_DEBUG_POSTFIX "")
set(OpenGL_GL_PREFERENCE GLVND)

add_compile_definitions("GLM_FORCE_QUAT_DATA_WXYZ")

add_compile_definitions("${RENDER_API}_API")
add_compile_definitions("GLM_ENABLE_EXPERIMENTAL")


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
"./Animation/*.cpp"
"./Animation/*/*.cpp"
"./TestLayers/*.cpp"
"./Renderer/*.cpp" "./Renderer/*.h"
"./Renderer/Renderer3D/*.cpp"
"./Renderer/Renderer3D/*.h"
"./Renderer/Renderer3D/*/*.cpp"
"./Renderer/Renderer3D/*/*.h"
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

if("${RENDER_API}" STREQUAL "Vulkan")

file(GLOB Vulkan_SRC 
"./platform/Vulkan/*.cpp" "./platform/Vulkan/*.h"
"./platform/Vulkan/*/*.cpp" "./platform/Vulkan/*/*.h"
"./platform/GLFW/*.cpp" "./platform/GLFW/*.h")

set(UPDATE_DEPS OFF)
set(BUILD_TESTS OFF)
set(SHADERC_SKIP_TESTS ON)
set(ALLOW_EXTERNAL_SPIRV_TOOLS ON)
target_compile_definitions(EngineCore PUBLIC "GLM_FORCE_DEPTH_ZERO_TO_ONE")
add_subdirectory("./platform/Vulkan/dependencies/headers")
get_target_property(VK_HEADER_DIRECTORY Vulkan-Headers INCLUDE_DIRECTORIES)

set(VULKAN_HEADERS_INSTALL_DIR "${VK_HEADER_DIRECTORY}")
add_subdirectory("./platform/Vulkan/dependencies/loader")
add_subdirectory("./platform/Vulkan/dependencies/vma")
set(BUILD_SHARED_LIBS on)
add_subdirectory("./platform/Vulkan/dependencies/bootstrap")
set(BUILD_SHARED_LIBS off)
add_subdirectory("./platform/Vulkan/dependencies/spirv-headers")
add_subdirectory("./platform/Vulkan/dependencies/spirv-tools")
add_subdirectory("./platform/Vulkan/dependencies/glslang")
add_subdirectory("./dependencies/shaderc")
message(WARNING "Vulkan mode")
target_link_libraries(EngineCore PRIVATE Vulkan-Headers vulkan GPUOpen::VulkanMemoryAllocator vk-bootstrap::vk-bootstrap shaderc)

target_sources(EngineCore PRIVATE ${Vulkan_SRC})


elseif()

target_sources(EngineCore PRIVATE ${OPENGL_SRC})

add_subdirectory("./dependencies/glew/build/cmake")

target_include_directories(glew_s PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dependencies/glew/include>")

target_link_libraries(EngineCore PRIVATE glew_s)


endif()

add_subdirectory("./dependencies/FreeType")
add_subdirectory("./dependencies/glfw")

add_subdirectory("./dependencies/EnTT")

add_subdirectory("./dependencies/glm")
add_subdirectory("./dependencies")
add_subdirectory("./dependencies/assimp")

add_subdirectory("./dependencies/Additional")

add_subdirectory("./dependencies/OpenAL")
set(BUILD_SHARED_LIBS off)
add_subdirectory("./Utilities")
set(BUILD_SHARED_LIBS on)
if(EDITOR)

file(GLOB EDITOR_FILES
 "./Editor/*.cpp"  "./Editor/*.h"
 "./Editor/*/*.cpp" "./Editor/*/*.h")

target_sources(EngineCore PRIVATE ${EDITOR_FILES})

target_compile_definitions(EngineCore PRIVATE EDITOR)

target_link_libraries(EngineCore PUBLIC IMGUI)

endif()
if(WIN32) 

file(GLOB WINDOWS_SRC
"./platform/Windows/WindowsOSApi.cpp" "./platform/Windows/WindowsOSApi.h")

target_sources(EngineCore PRIVATE ${WINDOWS_SRC})

set(FT_DISABLE_ZLIB on CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS on)

set(ALSOFT_INSTALL off CACHE BOOL "" FORCE)
set(ALSOFT_EXAMPLES off CACHE BOOL "" FORCE)



set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT off)
set(ASSIMP_BUILD_ASSIMP_TOOLS off)
set(ASSIMP_BUILD_TESTS off)
set(ASSIMP_INSTALL off)

target_link_libraries(EngineCore PUBLIC glfw glm Profiler MemoryManagement 
TaskSystem RuntimeTypeIdentifier ThreadManager FileManager ConfigManager EnTT::EnTT RuntimeTypeIdentifier JsonParser STB_Image assimp OpenAL ImGuizmo freetype)

target_compile_definitions(EngineCore PRIVATE BUILDDLL ENGINE_CORE)


set_property(CACHE RENDER_API PROPERTY STRINGS "OpenGL" "DirectX12")



file(GLOB WINDOWS_SRC
"./platform/Windows/WindowsOSApi.cpp" "./platform/Windows/WindowsOSApi.h")

file(GLOB DIRECTX_SRC
"./platform/DirectX/*.cpp" "./platform/DirectX/*.h"
"./platform/DirectX/*/*.cpp" "./platform/DirectX/*/*.h"
"./platform/Windows/*.cpp" "./platform/Windows/*.h")

target_compile_definitions(EngineCore PRIVATE "WINDOWS")

target_link_libraries(EngineCore PRIVATE Shlwapi.lib )

endif()

if(UNIX AND NOT APPLE)
    
target_link_libraries(EngineCore PUBLIC glfw glm Profiler MemoryManagement 
TaskSystem RuntimeTypeIdentifier ThreadManager FileManager ConfigManager EnTT RuntimeTypeIdentifier JsonParser STB_Image assimp OpenAL ImGuizmo
freetype)

target_compile_definitions(EngineCore PRIVATE "LINUX")

file(GLOB LINUX_SRC
"./platform/Linux/*.cpp" "./platform/Linux/*.h"
)

target_sources(EngineCore PRIVATE ${LINUX_SRC})

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
