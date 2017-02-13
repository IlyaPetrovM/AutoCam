include_directories(${CMAKE_SOURCE_DIR}/src/3rdparty)
add_subdirectory(3rdparty)
set(AUTOCAM_LIB_VIEW_SRC preview.cpp)
set(AUTOCAM_LIB_SRC
    "automotion.cpp"
    "autozoom.cpp"
    "statemachine.cpp"
    "arg.cpp"
    "autocamera.cpp"
    "detector.cpp"
    "viewfinder.cpp")
add_library(autocam_lib STATIC ${AUTOCAM_LIB_SRC} ${AUTOCAM_LIB_VIEW_SRC})
target_link_libraries(autocam_lib ${OpenCV_LIBS})

