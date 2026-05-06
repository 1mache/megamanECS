cmake_minimum_required(VERSION 3.22)

project(
    "megamanECS"
    VERSION 1.0.0
    LANGUAGES CXX)

# Global CMake variables are set here
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(USE_CPM "Whether to use CPM." ON)

option(ENABLE_WARNINGS "Enable to add warnings to a target." ON)
option(ENABLE_WARNINGS_AS_ERRORS "Enable to treat warnings as errors." ON)

option(ENABLE_TESTING "Enable a Unit Testing build." ON)

option(ENABLE_SANITIZE_ADDR "Enable address sanitize." OFF)
option(ENABLE_SANITIZE_UNDEF "Enable undefined sanitize." OFF)
option(ENABLE_SANITIZE_LEAK "Enable leak sanitize (Gcc/Clang only)." OFF)
option(ENABLE_SANITIZE_THREAD "Enable thread sanitize (Gcc/Clang only)." OFF)


# Project/Library Names
set(EXECUTABLE_NAME "main")

# CMAKE MODULES
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(AddGitSubmodule)
include(Docs)

if(WIN32)
    add_compile_definitions(_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()

if(ENABLE_WARNINGS)
    include(Warnings)
endif()

if(ENABLE_SANITIZE_ADDR
   OR ENABLE_SANITIZE_UNDEF
   OR ENABLE_SANITIZE_LEAK
   OR ENABLE_SANITIZE_THREAD)
    include(Sanitizer)
    add_sanitizer_flags()
endif()

# EXTERNAL LIBRARIES

if(USE_CPM)
    message(STATUS "Using CPM")
    include(CPM)
    if(ENABLE_TESTING)
        cpmaddpackage("gh:catchorg/Catch2#v3.5.3")
        list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
    endif()
else()
    message(STATUS "Using FetchContent")
    if(ENABLE_TESTING)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2
            GIT_TAG v3.5.3
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(Catch2)
        list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
    endif()
endif()

# SDL3
set(SDL_STATIC ON CACHE BOOL "Build SDL as a static library" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Build SDL as a static library" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Build the SDL3_test library" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "Enable installation of SDL3" FORCE)
add_subdirectory(dependencies/SDL "${CMAKE_BINARY_DIR}/dependencies/SDL" EXCLUDE_FROM_ALL)
# Box2D
set(BOX2D_SAMPLES OFF CACHE BOOL "Build Box2D samples" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "Build Box2D unit tests" FORCE)
set(BOX2D_DOCS OFF CACHE BOOL "Build Box2D docs" FORCE)
add_subdirectory(dependencies/box2d "${CMAKE_BINARY_DIR}/dependencies/box2d" EXCLUDE_FROM_ALL)

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(dependencies/SDL_image "${CMAKE_BINARY_DIR}/dependencies/SDL_image" EXCLUDE_FROM_ALL)

# SUBDIRECTORIES

add_subdirectory("pong ecs demo")
add_subdirectory(src)

add_executable(megaman_demo
    app/mockup.cpp
)

target_include_directories(megaman_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bagel
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(megaman_demo
    PRIVATE
        SDL3-static
        SDL3_image-static
)

add_custom_command(
    TARGET megaman_demo
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:megaman_demo>/res"
    COMMENT "Copying res directory to megaman_demo output folder")

add_executable(megaman_ecs
    app/megaman_ecs.cpp
    src/megamanGame.cpp
    src/megaman.cpp
)

target_include_directories(megaman_ecs
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bagel
)

target_link_libraries(megaman_ecs
    PRIVATE
        SDL3-static
        SDL3_image-static
        box2d
)

add_custom_command(
    TARGET megaman_ecs
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:megaman_ecs>/res"
    COMMENT "Copying res directory to megaman_ecs output folder")


if(ENABLE_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
