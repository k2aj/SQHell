cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS source/*.c source/*.cpp)
add_executable(sqhell ${SOURCES})
target_include_directories(sqhell PRIVATE source ${GLFW_INCLUDE_DIRS})
target_link_libraries(sqhell ${GLFW_LIBRARIES})
target_compile_definitions(sqhell PRIVATE SQLITE_ENABLE_MATH_FUNCTIONS)