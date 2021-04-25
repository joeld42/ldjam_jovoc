#
# project: hello-sokol
#
cmake_minimum_required(VERSION 3.0)

set( FIPS_EMSCRIPTEN_RELATIVE_SHELL_HTML basecode/shellpage.html )

project(ldjam_jovoc)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_C_STANDARD 99)
#if (CMAKE_SYSTEM_NAME STREQUAL "WindowsStore")
    set(CMAKE_CXX_STANDARD 17)
#else()
#    set(CMAKE_CXX_STANDARD 14)
#endif()

if (SOKOL_USE_WGPU_DAWN)
    set(USE_DAWN_SDK ON)
endif()


# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

fips_setup()
fips_ide_group("Imports")

# just suppress this pesky "missing field initializer warning" for now
if (FIPS_CLANG OR FIPS_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers")
endif()
if (FIPS_GCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-braces")
endif()

# macro to select the sokol-backend and shader dialects
macro(sokol_config)
    add_definitions(-DSOKOL_NO_DEPRECATED)
    if (FIPS_EMSCRIPTEN)
        if (FIPS_EMSCRIPTEN_USE_WEBGPU)
            add_definitions(-DSOKOL_WGPU)
            set(slang "wgpu")
        else()
            add_definitions(-DSOKOL_GLES3)
            set(slang "glsl300es:glsl100")
        endif()
    elseif (FIPS_RASPBERRYPI)
        add_definitions(-DSOKOL_GLES2)
        set(slang "glsl100")
    elseif (FIPS_ANDROID)
        add_definitions(-DSOKOL_GLES3)
        set(slang "glsl300es:glsl100")
    elseif (SOKOL_USE_D3D11)
        add_definitions(-DSOKOL_D3D11)
        set(slang "hlsl4")
    elseif (SOKOL_USE_METAL)
        add_definitions(-DSOKOL_METAL)
        if (FIPS_IOS)
            set(slang "metal_ios:metal_sim")
        else()
            set(slang "metal_macos")
        endif()
    else()
        if (FIPS_IOS)
            add_definitions(-DSOKOL_GLES3)
            set(slang "glsl300es:glsl100")
        else()
            add_definitions(-DSOKOL_GLCORE33)
            set(slang "glsl330")
        endif()
    endif()
endmacro()

include_directories( basecode )
include_directories( basecode/stb )
include_directories( basecode/fontstash )
add_subdirectory( basecode )

add_subdirectory( testgames )

add_subdirectory( ld48_deeper )

# fips_begin_app(hello-sokol windowed)
#     fips_files(hello-sokol.cpp)
#     fipsutil_copy(gamedata.yml)
#     fips_deps(stb)
# fips_end_app()

# fips_begin_app(ldjam_breakout windowed)
#     fips_files(ldjam_breakout.cpp 
#                 tk_sprite.cpp )
#     fipsutil_copy(breakout_gamedata.yml)
#     fips_deps(stb)
# fips_end_app()


fips_finish()


