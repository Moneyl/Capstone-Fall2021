# Subproject for SDL_ImGui_Testing exe, include source files and define project specific logic here.
cmake_minimum_required (VERSION 3.8)
project(CapstoneFall2021)

# Set asset folder path depending on build type.
# In repo folder in debug builds and in the exe folder in release builds.
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_definitions(-DASSET_FOLDER_PATH=\"${CMAKE_SOURCE_DIR}/assets/\")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DASSET_FOLDER_PATH=\"./assets/\")
endif()

# Recursively add all files in source directory to SOURCES variable
file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    )

# Add source files to this project's executable
add_executable (CapstoneFall2021
    # All files in source directory that we found with the previous command
    ${SOURCES}
    # ImGui is header only so we manually link its files
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_sdl.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_opengl3.cpp
    # nativefiledialog files
    ${CMAKE_SOURCE_DIR}/dependencies/nativefiledialog/src/nfd_common.c
    ${CMAKE_SOURCE_DIR}/dependencies/nativefiledialog/src/nfd_win.cpp
)

# Set include directories
target_include_directories(CapstoneFall2021 SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/
    ${CMAKE_SOURCE_DIR}/Dependencies/SDL/include/
    ${CMAKE_SOURCE_DIR}/Dependencies/IconFontCppHeaders/
    ${CMAKE_SOURCE_DIR}/Dependencies/magic_enum/include/
    ${CMAKE_SOURCE_DIR}/Dependencies/nativefiledialog/src/
    ${CMAKE_SOURCE_DIR}/Dependencies/nativefiledialog/src/include/
)

# Set libraries to statically link
target_link_libraries(CapstoneFall2021 PRIVATE SDL2-static)