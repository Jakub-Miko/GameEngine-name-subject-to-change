set(BUILD_SHARED_LIBS off)
add_library(STB_Image "stb_image_source.cpp" "impl_custom_imgui_backend.h" "impl_custom_imgui_platform.h")
set(BUILD_SHARED_LIBS on)

target_include_directories(STB_Image PUBLIC "../stb_image")
add_library(IMGUI INTERFACE)
file(GLOB IMGUI_FILES "${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui*.cpp" "${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui*.h")


target_sources(IMGUI INTERFACE ${IMGUI_FILES})
target_include_directories(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/imgui")

add_library(ImGuizmo INTERFACE)
file(GLOB imguizmo_sources "${CMAKE_SOURCE_DIR}/dependencies/ImGuizmo/*.cpp")
target_sources(ImGuizmo INTERFACE ${imguizmo_sources})
target_include_directories(ImGuizmo INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/ImGuizmo")
target_include_directories(ImGuizmo INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/imgui")

if(WIN32) 


if(${RENDER_API} STREQUAL "OpenGL")

target_sources(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_glfw.h" "${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_glfw.cpp")
target_sources(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_backend.h" "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_backend.cpp")
target_sources(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_platform.h" "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_platform.cpp")

endif()

if(${RENDER_API} STREQUAL "DirectX")

message(FATAL_ERROR "DirectX isn't supported yet.")

endif()

else()

if(${RENDER_API} STREQUAL "OpenGL")

target_sources(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_glfw.h" "${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_glfw.cpp")
target_sources(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_backend.h" "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_backend.cpp")
target_sources(IMGUI INTERFACE "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_platform.h" "${CMAKE_SOURCE_DIR}/dependencies/Additional/impl_custom_imgui_platform.cpp")

endif()

endif()