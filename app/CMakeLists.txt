# set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/main.cc")
set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sdl_main.cpp")

add_executable(${EXECUTABLE_NAME} ${APP_SOURCES})

target_include_directories(
    ${EXECUTABLE_NAME}
    PUBLIC
    "${CMAKE_SOURCE_DIR}/dependencies/bagel"
)

target_link_libraries(
    ${EXECUTABLE_NAME}
    PRIVATE
            SDL3-static
            SDL3_image-static
            nlohmann_json::nlohmann_json
            spdlog::spdlog
            cxxopts::cxxopts)

if(${ENABLE_WARNINGS})
    target_set_warnings(
        TARGET
        ${EXECUTABLE_NAME}
        ENABLE
        ${ENABLE_WARNINGS}
        AS_ERRORS
        ${ENABLE_WARNINGS_AS_ERRORS})
endif()

if(${ENABLE_LTO})
    target_enable_lto(
        TARGET
        ${EXECUTABLE_NAME}
        ENABLE
        ON)
endif()

if(${ENABLE_CLANG_TIDY})
    add_clang_tidy_to_target(${EXECUTABLE_NAME})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${EXECUTABLE_NAME} PRIVATE -Wno-old-style-cast)
endif()

# Copy resources next to executable so runtime paths like res/... work.
add_custom_command(
    TARGET ${EXECUTABLE_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/res"
    COMMENT "Copying res directory to output folder")
