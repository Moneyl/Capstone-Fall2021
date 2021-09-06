# Subproject for SDL_ImGui_Testing exe, include source files and define project specific logic here.
cmake_minimum_required (VERSION 3.8)
project(CapstoneFall2021)

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
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends/imgui_impl_dx11.cpp
)

# Set include directories
target_include_directories(CapstoneFall2021 SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/Dependencies/imgui/
    ${CMAKE_SOURCE_DIR}/Dependencies/SDL/include/
)

# Set libraries to statically link
target_link_libraries(CapstoneFall2021 PRIVATE SDL2-static)