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

# List of shaders
set(SHADER_LIST
    "rotoscope.comp"
    "text.comp"
    "navigation.comp"
    "orbit.comp"
    "orrery.comp"
    "starmap.comp"
    # Add more shaders here
)
# Path to the Vulkan SDK's glslc shader compiler
set(GLSL_COMPILER "$ENV{VULKAN_SDK}/bin/glslc")

set(DUMMY_FILE "${CMAKE_CURRENT_BINARY_DIR}/always_rebuild")
add_custom_target(AlwaysRebuild ALL
    COMMAND ${CMAKE_COMMAND} -E touch ${DUMMY_FILE}
)

foreach(SHADER ${SHADER_LIST})
    # Path to the shader source file
    set(SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/${SHADER}")

    # Path to the output SPIR-V binary
    set(SHADER_BINARY "${CMAKE_CURRENT_BINARY_DIR}/../${SHADER}.spv")

    # Custom command to compile the shader
    add_custom_command(
        OUTPUT ${SHADER_BINARY}
        COMMAND ${GLSL_COMPILER} ${SHADER_SOURCE} -g -o ${SHADER_BINARY}
        DEPENDS ${SHADER_SOURCE} ${DUMMY_FILE} 
        COMMENT "Compiling shader ${SHADER_SOURCE}"
    )

    # Custom target to trigger the shader compilation
    add_custom_target(
        ${SHADER}_compilation ALL
        DEPENDS ${SHADER_BINARY}
    )
    
endforeach()

add_subdirectory(Auto-Vk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(sdl2 CONFIG REQUIRED)
find_package(Zstd CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(Vulkan 1.2.162 REQUIRED)
find_package(CURL REQUIRED)
find_package(Magnum REQUIRED)
find_package(unofficial-nuklear CONFIG REQUIRED)

set_source_files_properties(
    src/cpu/cpu.cpp 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/fract.c
    PROPERTIES LANGUAGE CXX
)

set_source_files_properties(src/cpu/8086emu.cpp PROPERTIES COMPILE_FLAGS -O3)

set(HEADER_FILES
    src/emul/call.h
    src/cpu/cpu.h
    src/emul/findword.h
    src/emul/callstack.h
    src/disasOV/global.h
    src/emul/graphics.h
    src/emul/fract.h
    src/disasmX86/debugger.h
	src/util/lodepng.h 
    src/tts/speech.h
    ${CURL_INCLUDE_DIRS}
)

add_executable(emulatesdl 
    ${HEADER_FILES}
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
    src/tts/speech.cpp
)

target_compile_definitions(emulatesdl PRIVATE STARFLT1 SDL _CRT_SECURE_NO_WARNINGS NOMINMAX)

target_link_libraries(emulatesdl PRIVATE 
        nuklear
        Magnum::Magnum
        SDL2::SDL2main
        SDL2::SDL2
        xxHash::xxhash
        avk
        Vulkan::Vulkan
        Vulkan::Headers
        $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>
        ${CURL_LIBRARIES})

foreach(SHADER ${SHADER_LIST})
    add_dependencies(emulatesdl ${SHADER}_compilation)
endforeach()