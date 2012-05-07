CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(clraytracer)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O2 -std=c++11")       
endif()


find_package(PkgConfig)
pkg_check_modules(GLEW glew)
find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)

INCLUDE_DIRECTORIES(${GLEW_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${OPENCL_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${OGLPLUS_INCLUDE_DIR} )

add_executable(clraytracer main.cpp renderer.cpp)

TARGET_LINK_LIBRARIES(clraytracer ${GLEW_LIBRARIES} ${OPENGL_LIBRARIES} ${OPENCL_LIBRARIES} ${GLFW_LIBRARIES})
