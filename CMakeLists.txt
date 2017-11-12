project(autocam4)

cmake_minimum_required(VERSION 3.1)
add_compile_options(-std=gnu++11)
add_compile_options(-pthread) # CMake 2.8.12 or newer
aux_source_directory(. SRC_LIST)
find_package(OpenCV REQUIRED)
link_directories( ${OpenCV_LIB_DIR} )
include_directories(${OpenCV_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} ${SRC_LIST} detectingrules.cpp output.cpp cvwindow.cpp)
target_link_libraries(${PROJECT_NAME} "-pthread" ${OpenCV_LIBS})
