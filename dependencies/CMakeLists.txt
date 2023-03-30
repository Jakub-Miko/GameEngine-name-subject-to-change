
add_compile_definitions(GTEST_LANG_CXX11=1)
add_subdirectory("${CMAKE_SOURCE_DIR}/dependencies/bullet")
add_library(Bullet INTERFACE)
target_link_libraries(Bullet INTERFACE BulletDynamics BulletCollision Bullet3Common BulletInverseDynamics LinearMath)
target_include_directories(Bullet INTERFACE ${CMAKE_SOURCE_DIR}/dependencies/bullet/src)
target_include_directories(Bullet INTERFACE ${CMAKE_SOURCE_DIR}/dependencies/bullet/Extras)
