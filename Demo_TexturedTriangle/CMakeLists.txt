cmake_minimum_required(VERSION 3.16)

add_executable(Demo_TexturedTriangle)

target_sources(Demo_TexturedTriangle
PRIVATE
    app.cpp
)

target_link_libraries(Demo_TexturedTriangle
PRIVATE
    SoftGL
    LFramework
    LFramework::Input
)