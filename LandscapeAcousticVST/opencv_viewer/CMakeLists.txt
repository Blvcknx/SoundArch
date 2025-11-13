cmake_minimum_required(VERSION 3.15)
project(OpenCVHillshadeViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV first.")
endif()

# Create executable
add_executable(opencv_hillshade_viewer main.cpp)

# Link OpenCV libraries
target_link_libraries(opencv_hillshade_viewer ${OpenCV_LIBS})

# Include OpenCV headers
target_include_directories(opencv_hillshade_viewer PRIVATE ${OpenCV_INCLUDE_DIRS})

# Set output directory
set_target_properties(opencv_hillshade_viewer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin"
)

# Copy OpenCV DLLs to output directory on Windows
if(WIN32)
    # Get OpenCV runtime libraries
    get_target_property(OPENCV_DLLS opencv_world IMPORTED_LOCATION_RELEASE)
    if(OPENCV_DLLS)
        add_custom_command(TARGET opencv_hillshade_viewer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${OPENCV_DLLS} $<TARGET_FILE_DIR:opencv_hillshade_viewer>
        )
    endif()
endif()

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")