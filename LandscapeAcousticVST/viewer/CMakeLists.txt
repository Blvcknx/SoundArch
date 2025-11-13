cmake_minimum_required(VERSION 3.15)
project(DEMViewer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Find GDAL
set(GDAL_FOUND TRUE)
set(GDAL_INCLUDE_DIRS "C:/Temp/GDAL/gcore;C:/Temp/GDAL/ogr;C:/Temp/GDAL/port")
set(GDAL_LIBRARIES "C:/GDAL/gdal.dll")
message(STATUS "GDAL found: ${GDAL_INCLUDE_DIRS}, ${GDAL_LIBRARIES}")

# Add executable
add_executable(DEMViewer WIN32
    main.cpp
)

# Link libraries
target_link_libraries(DEMViewer PRIVATE
    Qt5::Widgets
)

# Include directories
target_include_directories(DEMViewer PRIVATE
    ${GDAL_INCLUDE_DIRS}
)

# Add GDAL compile definitions
target_compile_definitions(DEMViewer PRIVATE
    GDAL_FOUND=1
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(DEMViewer PRIVATE
        ${GDAL_LIBRARIES}
    )
endif()

# Install target
install(TARGETS DEMViewer
    RUNTIME DESTINATION bin
)
