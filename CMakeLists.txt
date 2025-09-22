cmake_minimum_required(VERSION 3.10)
project(olsr-sim)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(external)
include_directories(external/imgui)
include_directories(external/imgui/backends)
include_directories(external/ImGuiFileDialog)

add_subdirectory(external/glfw)

set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

add_executable(olsr-sim ${PROJECT_SOURCES} ${IMGUI_SOURCES})

target_link_libraries(olsr-sim PRIVATE
    OpenGL::GL
    glfw
)
