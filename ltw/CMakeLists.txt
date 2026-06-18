cmake_minimum_required(VERSION 3.10)

project(LTW)

set(CMAKE_OSX_DEPLOYMENT_TARGET 14.0)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target arm64-apple-ios14.0 -fcommon")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target arm64-apple-ios14.0 -fcommon -stdlib=libc++")
set(CMAKE_C_FLAGS_DEBUG "-funwin-tables -g")

if("${CMAKE_HOST_SYSTEM_NAME}" MATCHES "Linux|^GNU$|Android")
    set(GLOBAL_LDFLAGS "-isysroot ${CMAKE_OSX_SYSROOT} -Wl,-syslibroot,${CMAKE_OSX_SYSROOT} -fuse-ld=lld")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GLOBAL_LDFLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GLOBAL_LDFLAGS}")
endif()

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rpath @loader_path -rpath @executable_path")

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/glsl_optimizer
)

add_compile_definitions(
    NOMINMAX
    _USE_MATH_DEFINES
    __STDC_NO_THREADS__
    __STDC_LIMIT_MACROS
    __STDC_FORMAT_MACROS
    __STDC_CONSTANT_MACROS
    UTIL_ARCH_LITTLE_ENDIAN
    UNIX
    IOS DETECT_OS_APPLE
    HAVE_OPENGL
    HAVE_OPENGL_ES_1
    HAVE_OPENGL_ES_3
)

file(GLOB_RECURSE GLSL_OPTIMIZER
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/glsl_optimizer/src/*.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/glsl_optimizer/src/*.cpp
)

list(FILTER GLSL_OPTIMIZER EXCLUDE REGEX ".*u_debug_stack_android\\.cpp$")

add_library(LTW SHARED
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/basevertex.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/egl.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/framebuffer.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/glformats.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/license_notice.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/multidraw.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/of_buffer_copier.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/proc.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/query.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/shader_wrapper.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/string_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/stubs.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/swizzle.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/vertexattrib.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/env.c

    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/unordered_map/int_hash.c
    ${CMAKE_CURRENT_LIST_DIR}/src/main/tinywrapper/unordered_map/unordered_map.c

    ${GLSL_OPTIMIZER}
)

target_link_libraries(LTW
    "-F${CMAKE_CURRENT_LIST_DIR}/../external"
    "-framework libEGL"
    "-framework libGLESv2"
)