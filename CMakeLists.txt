cmake_minimum_required(VERSION 3.10)

if(WIN32)
    add_compile_options(/bigobj)
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg Target Triplet to use")
else()
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg Target Triplet to use")
endif()

project(Starflight)

# Find the Vulkan package
find_package(Vulkan REQUIRED)

# Path to the Vulkan SDK's glslc shader compiler
set(GLSL_COMPILER "$ENV{VULKAN_SDK}/bin/glslc")

# Path to the shader source file
set(SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/emul/rotoscope.comp")

# Path to the output SPIR-V binary
set(SHADER_BINARY "${CMAKE_CURRENT_BINARY_DIR}/rotoscope.spv")

# Custom command to compile the shader
add_custom_command(
    OUTPUT ${SHADER_BINARY}
    COMMAND ${GLSL_COMPILER} ${SHADER_SOURCE} -o ${SHADER_BINARY}
    DEPENDS ${SHADER_SOURCE}
    COMMENT "Compiling shader ${SHADER_SOURCE}"
)

# Custom target to trigger the shader compilation
add_custom_target(
    ShaderCompilation ALL
    DEPENDS ${SHADER_BINARY}
)

add_subdirectory(Auto-Vk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(sdl2 CONFIG REQUIRED)
find_package(Zstd CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(Vulkan 1.2.162 REQUIRED)

set_source_files_properties(
    src/cpu/cpu.cpp 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/fract.c
    PROPERTIES LANGUAGE CXX
)

add_executable(emulatesdl 
    src/emul/emul.cpp 
    src/emul/call.cpp 
    src/cpu/cpu.cpp
    src/emul/findword.cpp 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/graphics.cpp 
    src/emul/fract.c
    src/cpu/8086emu.cpp
    src/util/lodepng.cpp
    src/emul/sdl_helper.cpp
    src/emul/vulkan_helper.cpp
)

target_compile_definitions(emulatesdl PRIVATE STARFLT1 SDL _CRT_SECURE_NO_WARNINGS)

target_link_libraries(emulatesdl PRIVATE 
        SDL2::SDL2main
        SDL2::SDL2
        xxHash::xxhash
        avk
        Vulkan::Vulkan
        Vulkan::Headers
        $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>)

# Make sure your main target depends on the ShaderCompilation
add_dependencies(emulatesdl ShaderCompilation)
