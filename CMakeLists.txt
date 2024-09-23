cmake_minimum_required(VERSION 3.24)

if(WIN32)
    add_compile_options(/bigobj)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL-")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /LTCG- /INCREMENTAL")
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
if(WIN32)
    set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg Target Triplet to use")
else()
    set(VCPKG_TARGET_TRIPLET "x64-linux" CACHE STRING "Vcpkg Target Triplet to use")
endif()

project(Starflight)

include(FetchContent)

set(DILIGENT_NO_DIRECT3D11 ON CACHE BOOL "Disable D3D11 support")
set(DILIGENT_NO_DIRECT3D12 ON CACHE BOOL "Disable D3D12 support")
set(DILIGENT_NO_OPENGL ON CACHE BOOL "Disable OpenGL support")
set(DILIGENT_NO_METAL ON CACHE BOOL "Disable Metal support")
set(DILIGENT_BUILD_SAMPLES OFF CACHE BOOL "Disable samples building")
set(DILIGENT_MSVC_COMPILE_OPTIONS "" CACHE STRING "Remove warnings as errors")

FetchContent_Declare(
    DiligentCore
    GIT_REPOSITORY https://github.com/canadacow/DiligentCore.git
    GIT_TAG canadacow-development
    SOURCE_DIR _deps/DiligentCore
)
FetchContent_Declare(
    DiligentTools
    GIT_REPOSITORY https://github.com/canadacow/DiligentTools.git
    GIT_TAG canadacow-development
    SOURCE_DIR _deps/DiligentTools
)
FetchContent_Declare(
    DiligentFX
    GIT_REPOSITORY https://github.com/canadacow/DiligentFX.git
    GIT_TAG canadacow-development
    SOURCE_DIR _deps/DiligentFX
)
FetchContent_MakeAvailable(DiligentCore DiligentTools DiligentFX)

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
    "encounter.comp"
    "compositor.comp"
    "title.comp"
    # Add more shaders here
)
# Path to the Vulkan SDKs glslc shader compiler
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
find_package(SDL2_image CONFIG REQUIRED)
find_package(Zstd CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)
find_package(Vulkan 1.2.162 REQUIRED)
find_package(CURL REQUIRED)
find_package(Magnum REQUIRED)
find_package(unofficial-nuklear CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Include FidelityFX headers
include_directories(3rdparty/ffx/include)

# Link FidelityFX libraries
link_directories(3rdparty/ffx/lib)

set_source_files_properties(
    src/cpu/cpu.cpp 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/fract.cpp
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
    src/emul/platform.h
    src/emul/common.pch
    src/pbr/SF_PBR_Renderer.hpp
    src/pbr/SF_GLTFLoader.hpp
    src/pbr/SF_GLTFResourceManager.hpp
    src/pbr/SF_GLTFBuilder.hpp
    src/emul/DiligentShadowMapManager.hpp
    ${CURL_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}/nlohmann
)

add_executable(emulatesdl WIN32
    src/emul/emul.cpp 
    src/emul/call.cpp 
    src/emul/platform.cpp
    src/cpu/cpu.cpp
    src/emul/findword.cpp 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/graphics.cpp 
    src/emul/fract.cpp
    src/cpu/8086emu.cpp
    src/util/lodepng.cpp
    src/emul/sdl_helper.cpp
    src/emul/vulkan_helper.cpp
    src/emul/DiligentShadowMapManager.cpp
    src/tts/speech.cpp
    src/pbr/SF_PBR_Renderer.cpp
    src/pbr/SF_GLTFLoader.cpp
    src/pbr/SF_GLTFResourceManager.cpp
    src/pbr/SF_GLTFBuilder.cpp
    src/emul/DynamicMesh.cpp
)

target_include_directories(emulatesdl
PRIVATE
    ${diligentcore_SOURCE_DIR}
    ${diligenttools_SOURCE_DIR}
    ${diligentfx_SOURCE_DIR}
    ${SDL2_INCLUDE_DIRS}
)

target_precompile_headers(emulatesdl PRIVATE src/emul/common.pch)

target_compile_definitions(emulatesdl PRIVATE STARFLT1 SDL _CRT_SECURE_NO_WARNINGS NOMINMAX)

target_link_libraries(emulatesdl PRIVATE 
        $<$<CONFIG:Debug>:ffx_sssr_x64d>
        $<$<CONFIG:Debug>:ffx_denoiser_x64d>
        $<$<CONFIG:Debug>:ffx_backend_vk_x64d>
        $<$<CONFIG:Release>:ffx_sssr_x64>
        $<$<CONFIG:Release>:ffx_denoiser_x64>
        $<$<CONFIG:Release>:ffx_backend_vk_x64>
        $<$<CONFIG:RelWithDebInfo>:ffx_sssr_x64drel>
        $<$<CONFIG:RelWithDebInfo>:ffx_denoiser_x64drel>
        $<$<CONFIG:RelWithDebInfo>:ffx_backend_vk_x64drel>
        Diligent-BuildSettings
        Diligent-GraphicsEngineVk-shared
        DiligentFX
        nuklear
        cereal::cereal
        Magnum::Magnum
        SDL2::SDL2main
        SDL2::SDL2
        SDL2_image::SDL2_image
        xxHash::xxhash
        avk
        Vulkan::Vulkan
        Vulkan::Headers
        nlohmann_json::nlohmann_json
        $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>
        ${CURL_LIBRARIES})

foreach(SHADER ${SHADER_LIST})
    add_dependencies(emulatesdl ${SHADER}_compilation)
endforeach()

copy_required_dlls(emulatesdl)