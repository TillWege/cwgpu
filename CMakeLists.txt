cmake_minimum_required(VERSION 3.11)
project(cwgpu)

add_subdirectory(external/glfw)
add_subdirectory(external/glfw3webgpu)

include_directories("external/imgui" "external/imgui/backends")

file(GLOB sources CONFIGURE_DEPENDS
        "src/*.h" "src/*.cpp"
        "external/imgui/*.h" "external/imgui/*.cpp"
        "external/imgui/backends/imgui_impl_wgpu.cpp" "external/imgui/backends/imgui_impl_wgpu.h"
        "external/imgui/backends/imgui_impl_glfw.cpp" "external/imgui/backends/imgui_impl_glfw.h"
)

add_executable(cwgpu ${sources})


include(FetchContent)
FetchContent_Declare(
        webgpu
        GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
        GIT_TAG        main
)

FetchContent_MakeAvailable(webgpu)

target_link_libraries(cwgpu PRIVATE glfw webgpu glfw3webgpu)

set_target_properties(cwgpu PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if (MSVC)
    target_compile_options(cwgpu PRIVATE /W4)
else()
    target_compile_options(cwgpu PRIVATE -Wall -Wextra -pedantic)
endif()

target_copy_webgpu_binaries(cwgpu)