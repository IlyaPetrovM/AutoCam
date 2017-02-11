# cmake needs this line
cmake_minimum_required(VERSION 2.8)

project(autocam)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
if(CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wextra -pedantic -Wno-long-long")
endif()

message(STATUS "\nBuilding project '${PROJECT_NAME}':")

find_package(OpenCV REQUIRED) # поиск пакета

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    lib_dir: ${OpenCV_LIB_DIR}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "    include path: ${CMAKE_SOURCE_DIR}")
include_directories(${CMAKE_SOURCE_DIR})
if(OpenCV_VERSION VERSION_GREATER "3.0.0")
    link_directories( ${OpenCV_LIB_DIR} )
    include_directories(${OpenCV_INCLUDE_DIRS}) ## Добавление директорий
    include_directories(${CMAKE_SOURCE_DIR}/src/)
    add_subdirectory(src)
    add_executable(${PROJECT_NAME} main.cpp ${SRC_DIR})
    target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} autocam_lib color -pthread)
else()
    message(FATAL_ERROR
          "OpenCV version is less than 2.4.12 (current is ${OpenCV_VERSION}) \n
                the project will not be compiled")
endif()
