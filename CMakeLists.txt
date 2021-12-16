# Top-level CMake project file, do global configuration and include sub-projects here
cmake_minimum_required (VERSION 3.8)
project ("RootProject")

# Set config specific defines that exist in Debug and Release with debug info builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(DEBUG_BUILD)
endif()

set(CMAKE_CXX_STANDARD           17)
set(CMAKE_CXX_STANDARD_REQUIRED  ON)
set(CMAKE_CXX_EXTENSIONS         OFF)

# Include sub-projects
add_subdirectory("dependencies/SDL")
add_subdirectory("dependencies/SDL_mixer")
add_subdirectory("dependencies/pugixml")
add_subdirectory ("src")