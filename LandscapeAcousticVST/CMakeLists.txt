cmake_minimum_required(VERSION 3.15)
project(LandscapeAcousticVST VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE as a subdirectory
# Note: Add JUCE as a git submodule or download JUCE framework
# git submodule add https://github.com/juce-framework/JUCE.git
add_subdirectory(JUCE)

# Find GDAL
# find_package(GDAL QUIET)
set(GDAL_FOUND TRUE)
set(GDAL_INCLUDE_DIRS "C:/Temp/GDAL/gcore;C:/Temp/GDAL/ogr;C:/Temp/GDAL/port")
set(GDAL_LIBRARIES "C:/GDAL/gdal.dll")
message(STATUS "GDAL found manually: ${GDAL_INCLUDE_DIRS}, ${GDAL_LIBRARIES}")

# Plugin target
juce_add_plugin(LandscapeAcousticVST
    COMPANY_NAME "SoundArch"
    PLUGIN_MANUFACTURER_CODE SndA
    PLUGIN_CODE Lav1
    FORMATS VST3 AU
    PRODUCT_NAME "LandscapeAcousticVST"
    
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
    Source/TerrainRenderer.cpp
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
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_events
    juce::juce_core
    juce::juce_data_structures
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_dsp
    juce::juce_opengl
)

if(GDAL_FOUND)
    # target_link_libraries(LandscapeAcousticVST PRIVATE ${GDAL_LIBRARIES})
endif()

# Compiler definitions
target_compile_definitions(LandscapeAcousticVST PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Include directories
target_include_directories(LandscapeAcousticVST PRIVATE
    Source
    ${CMAKE_BINARY_DIR}/LandscapeAcousticVST_artefacts/JuceLibraryCode
)

if(GDAL_FOUND)
    target_include_directories(LandscapeAcousticVST PRIVATE ${GDAL_INCLUDE_DIRS})
endif()

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

# Add standalone application target
add_executable(LandscapeAcousticVST_Standalone WIN32
    Source/StandaloneMain.cpp
    Source/StandaloneEditor.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/TerrainLoader.cpp
    Source/TerrainRenderer.cpp
    Source/AcousticEngine.cpp
    Source/ImpulseGenerator.cpp
    Source/FresnelDiffraction.cpp
    Source/Environment.cpp
    Source/TerrainProfile.cpp
    Source/Utils.cpp
)

target_link_libraries(LandscapeAcousticVST_Standalone PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_events
    juce::juce_core
    juce::juce_data_structures
    juce::juce_audio_basics
    juce::juce_audio_formats
    juce::juce_audio_devices
    juce::juce_dsp
    juce::juce_opengl
)

if(GDAL_FOUND)
    # target_link_libraries(LandscapeAcousticVST_Standalone PRIVATE ${GDAL_LIBRARIES})
endif()

target_compile_definitions(LandscapeAcousticVST_Standalone PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STANDALONE_APPLICATION=1
    JucePlugin_Name="LandscapeAcousticVST"
    JucePlugin_Desc="Real-time Terrain-Based Sound Propagation VST Plugin"
    JucePlugin_Manufacturer="SoundArch"
    JucePlugin_ManufacturerCode=SndA
    JucePlugin_PluginCode=Lav1
    JucePlugin_IsSynth=0
    JucePlugin_WantsMidiInput=0
    JucePlugin_ProducesMidiOutput=0
    JucePlugin_IsMidiEffect=0
    JucePlugin_EditorRequiresKeyboardFocus=0
    JucePlugin_Version=1.0.0
    JucePlugin_VersionCode=0x10000
    JucePlugin_VersionString="1.0.0"
)

target_include_directories(LandscapeAcousticVST_Standalone PRIVATE
    Source
    ${CMAKE_BINARY_DIR}/LandscapeAcousticVST_artefacts/JuceLibraryCode
)

if(GDAL_FOUND)
    target_include_directories(LandscapeAcousticVST_Standalone PRIVATE ${GDAL_INCLUDE_DIRS})
endif()

# Platform-specific settings for standalone
if(WIN32)
    target_link_libraries(LandscapeAcousticVST_Standalone PRIVATE
        winmm.lib
        version.lib
        ole32.lib
        oleaut32.lib
    )
elseif(APPLE)
    target_link_libraries(LandscapeAcousticVST_Standalone PRIVATE
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

# Installation
install(TARGETS LandscapeAcousticVST LandscapeAcousticVST_Standalone
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