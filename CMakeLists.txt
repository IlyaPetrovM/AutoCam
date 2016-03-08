# cmake needs this line
cmake_minimum_required(VERSION 2.8)

project(facedetect)
message(STATUS "\nBuilding project '${PROJECT_NAME}':")

find_package(OpenCV REQUIRED) # поиск пакета

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
if(OpenCV_VERSION VERSION_GREATER "2.9.0")
    if(CMAKE_VERSION VERSION_LESS "2.8.11")
      include_directories(${OpenCV_INCLUDE_DIRS}) ## Добавление директорий
    endif()
    add_executable(${PROJECT_NAME} facedetect.cpp)
    target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
else()
    message(FATAL_ERROR
          "OpenCV version is less than 3.0.0 (current is ${OpenCV_VERSION}) \n
                the project will not be compiled")
endif()
