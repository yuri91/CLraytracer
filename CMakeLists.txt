CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(clraytracer)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-O2 -std=c++11")       
endif()


find_package(OpenCL REQUIRED)
find_package(PNG REQUIRED)


INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})

add_executable(clraytracer main.cpp renderer.cpp)

TARGET_LINK_LIBRARIES(clraytracer ${OPENCL_LIBRARIES} ${PNG_LIBRARIES})