cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")
set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "Vcpkg Target Triplet to use")

project(Starflight)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL2 CONFIG REQUIRED)
find_package(Zstd CONFIG REQUIRED)
find_package(xxHash CONFIG REQUIRED)

set_source_files_properties(
    src/cpu/cpu.c 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/fract.c
    src/emul/findword.cpp 
    PROPERTIES LANGUAGE CXX
)

add_executable(emulatesdl 
    src/emul/emul.cpp 
    src/emul/call.cpp 
    src/cpu/cpu.c 
    src/emul/findword.cpp 
    src/emul/callstack.c 
    src/disasOV/global.c 
    src/emul/graphics.cpp 
    src/emul/fract.c
)

target_compile_definitions(emulatesdl PRIVATE STARFLT1 SDL)

target_link_libraries(emulatesdl PRIVATE 
        SDL2::SDL2 
        SDL2::SDL2main
        xxHash::xxhash 
        $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>)