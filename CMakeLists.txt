cmake_minimum_required(VERSION 2.6)
project(MandelbrotCL)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#use double

# comment the following out for double precision support
add_definitions( -DUSE_DOUBLE )

# find SDL2
find_package(SDL2 REQUIRED)
#find_package(SDL2_image REQUIRED)

# find OpenGL
find_package(OpenGL REQUIRED)
add_definitions(${OpenGL_DEFINITIONS})

# find OpenCL
find_package(OpenCL REQUIRED)


# find GLEW
find_package(GLEW REQUIRED)

# find GLM
#find_package(GLM REQUIRED)

set(SOURCE_FILES
		main.cpp
		Shader.cpp
		Shader.hpp
		ShaderProgram.cpp
		ShaderProgram.hpp
		GLMain.cpp
		GLMain.hpp
		OCLRenderer.cpp
		OCLRenderer.hpp
		CLUtils.cpp
		CLUtils.hpp
		Texture.hpp)

add_executable(MandelbrotCL ${SOURCE_FILES})

target_include_directories(MandelbrotCL PUBLIC
		${GLM_INCLUDE_DIR}
		${GLEW_INCLUDE_DIRS}
		${OpenGL_INCLUDE_DIRS}
		${OpenCL_INCLUDE_DIRS}
		${SDL2_INCLUDE_DIR})

target_link_libraries(MandelbrotCL
		${OPENGL_LIBRARIES}
		${OpenCL_LIBRARIES}
		${GLUT_LIBRARY}
		${GLEW_LIBRARIES}
		${SDL2_LIBRARY})
