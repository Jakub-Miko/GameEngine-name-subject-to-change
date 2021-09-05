cmake_minimum_required(VERSION 3.0.0)
project(PseudoTest VERSION 0.1.0)

set(WINDOW_TYPE "" CACHE STRING "")
set_property(CACHE WINDOW_TYPE PROPERTY STRINGS "WIN32Window" "Glfw_Window")

file(GLOB SRCS "./*.cpp" "./*.h" "./*/*/*.h" "./*/*/*.cpp" "./*/*.h" "./*/*.cpp")

add_executable(PseudoTest ${SRCS})

if("${WINDOW_TYPE}" STREQUAL "Glfw_Window")

add_subdirectory("./dependencies/glfw")

add_subdirectory("./dependencies/glew/build/cmake")

target_include_directories(glew_s PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/dependencies/glew/include>")

target_link_libraries(PseudoTest PRIVATE glfw glew_s)

endif()

target_include_directories(PseudoTest PRIVATE .)

target_compile_definitions(PseudoTest PRIVATE ${WINDOW_TYPE})
