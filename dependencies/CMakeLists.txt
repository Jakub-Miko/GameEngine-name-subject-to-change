
add_compile_definitions(GTEST_LANG_CXX11=1)
set(BUILD_SHARED_LIBS off)
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/bullet")
set(BUILD_SHARED_LIBS on)
add_library(Bullet INTERFACE)
target_link_libraries(Bullet INTERFACE BulletDynamics BulletCollision Bullet3Common LinearMath)
target_include_directories(Bullet INTERFACE ${CMAKE_SOURCE_DIR}/dependencies/bullet/src)
target_include_directories(Bullet INTERFACE ${CMAKE_SOURCE_DIR}/dependencies/bullet/Extras)
