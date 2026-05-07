# add new code files here to compile
target_sources(
    ${EXECUTABLE_NAME}
    PRIVATE
        GlobalData.h
        MTransform.h
        MapLayer.h
        MapLayer.cpp
        Texture.h
        Texture.cpp
        megaman.cpp
        megaman.h
        megamanGame.cpp
        megamanGame.h
)

target_include_directories(
    ${EXECUTABLE_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
