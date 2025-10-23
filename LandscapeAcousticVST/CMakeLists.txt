cmake_minimum_required(VERSION 3.15)
project(LandscapeAcousticVST VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
# Note: Add JUCE as a git submodule or download JUCE framework
# git submodule add https://github.com/juce-framework/JUCE.git
add_subdirectory(JUCE)

# Find GDAL
find_package(GDAL REQUIRED)
if(GDAL_FOUND)
    include_directories(${GDAL_INCLUDE_DIRS})
    message(STATUS "GDAL found: ${GDAL_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "GDAL not found. Please install GDAL development libraries.")
endif()

# Plugin target
juce_add_plugin(LandscapeAcousticVST
    COMPANY_NAME "SoundArch"
    PLUGIN_MANUFACTURER_CODE SndA
    PLUGIN_CODE Lav1
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Landscape Acoustic VST"
    
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_SYNTH FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    
    # Platform-specific copy directories
    VST3_COPY_DIR "${CMAKE_BINARY_DIR}/VST3"
    AU_COPY_DIR "${CMAKE_BINARY_DIR}/AU"
)

# Source files
target_sources(LandscapeAcousticVST PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/TerrainLoader.cpp
    Source/AcousticEngine.cpp
    Source/ImpulseGenerator.cpp
    Source/FresnelDiffraction.cpp
    Source/Environment.cpp
    Source/TerrainProfile.cpp
    Source/Utils.cpp
)

# Link libraries
target_link_libraries(LandscapeAcousticVST PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_opengl
    ${GDAL_LIBRARIES}
)

# Compiler definitions
target_compile_definitions(LandscapeAcousticVST PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Include directories
target_include_directories(LandscapeAcousticVST PRIVATE
    Source
    ${GDAL_INCLUDE_DIRS}
)

# Platform-specific settings
if(WIN32)
    target_link_libraries(LandscapeAcousticVST PRIVATE
        winmm.lib
        version.lib
        ole32.lib
        oleaut32.lib
    )
elseif(APPLE)
    target_link_libraries(LandscapeAcousticVST PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework QuartzCore"
        "-framework AudioUnit"
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreMIDI"
        "-framework DiscRecording"
    )
endif()

# Enable testing
enable_testing()

# Add Google Test
find_package(GTest QUIET)
if(GTest_FOUND)
    add_subdirectory(tests)
    message(STATUS "Google Test found - building tests")
else()
    message(WARNING "Google Test not found - tests will not be built")
endif()

# Installation
install(TARGETS LandscapeAcousticVST
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# CPack configuration for packaging
include(CPack)
set(CPACK_PACKAGE_NAME "LandscapeAcousticVST")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Real-time Terrain-Based Sound Propagation VST Plugin")
set(CPACK_PACKAGE_VENDOR "SoundArch")