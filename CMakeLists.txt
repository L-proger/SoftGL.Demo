cmake_minimum_required(VERSION 3.16)

project(SoftGLDemo CXX C ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#add_subdirectory(Demo_Graphics2D)

add_subdirectory(Dependencies/SoftGL)
add_subdirectory(Dependencies/LMath)
add_subdirectory(Dependencies/LFramework)
add_subdirectory(Demo_TexturedTriangle)

