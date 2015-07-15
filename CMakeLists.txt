cmake_minimum_required(VERSION 2.8.11)

# include modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules")

# find glm
find_package(GLM REQUIRED)

# find GLUT
find_package(GLUT REQUIRED)

# find OpenGL
find_package(OpenGL REQUIRED)

# find OpenGL
find_package(FFMPEG REQUIRED)

include_directories(
    ${GLM_INCLUDE_DIRS}
    ${GLUT_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
#    ${FFMPEG_INCLUDE_DIR}
    /usr/local/include/libavformat
)

# find GLEW
find_package(GLEW REQUIRED)

# GL libraries
set(GL_LIBRARIES
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${FFMPEG_LIBRARIES}
)

add_definitions(-std=c++11)

add_executable(ffmpeg_opengl 
    ffmpeg_opengl/main.cpp 
    ffmpeg_opengl/cap_ffmpeg_impl.cpp
    ffmpeg_opengl/cap.cpp
    # ffmpeg_opengl/cap_ffmpeg.cpp
)

target_link_libraries(ffmpeg_opengl ${GL_LIBRARIES} pthread lzma bz2 swresample)