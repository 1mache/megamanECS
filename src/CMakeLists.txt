target_sources(
    ${EXECUTABLE_NAME}
    PRIVATE
        GlobalData.h
        MTransform.h
)

target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
