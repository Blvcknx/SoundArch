# Test configuration for LandscapeAcousticVST
cmake_minimum_required(VERSION 3.15)

# Find Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Test executable for unit tests
add_executable(unit_tests
    unit/test_acoustic_engine.cpp
    unit/test_fresnel_diffraction.cpp
    unit/test_terrain_profile.cpp
    unit/test_impulse_generator.cpp
    unit/test_utils.cpp
)

# Link test executable
target_link_libraries(unit_tests 
    ${GTEST_LIBRARIES} 
    ${GTEST_MAIN_LIBRARIES}
    pthread  # Required for Google Test on Linux
    
    # Link against plugin source files (as static library)
    # Note: You may need to create a static library target for the plugin sources
)

# Include plugin source directory
target_include_directories(unit_tests PRIVATE 
    ../Source
    ${GDAL_INCLUDE_DIRS}
)

# Add plugin source files directly for testing
target_sources(unit_tests PRIVATE
    ../Source/AcousticEngine.cpp
    ../Source/FresnelDiffraction.cpp
    ../Source/TerrainProfile.cpp
    ../Source/ImpulseGenerator.cpp
    ../Source/Utils.cpp
    ../Source/Environment.cpp
    # Note: Exclude JUCE-dependent files from unit tests
)

# Integration test executable
add_executable(integration_tests
    integration/test_dem_to_ir.cpp
    integration/test_realtime_convolution.cpp
    integration/test_qgis_roundtrip.cpp
)

target_link_libraries(integration_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
    ${GDAL_LIBRARIES}
)

target_include_directories(integration_tests PRIVATE
    ../Source
    ${GDAL_INCLUDE_DIRS}
)

# Regression test executable  
add_executable(regression_tests
    regression/test_regression.cpp
)

target_link_libraries(regression_tests
    ${GTEST_LIBRARIES}
    ${GTEST_MAIN_LIBRARIES}
    pthread
)

# Register tests with CTest
add_test(NAME UnitTests COMMAND unit_tests)
add_test(NAME IntegrationTests COMMAND integration_tests)
add_test(NAME RegressionTests COMMAND regression_tests)

# Test data directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Custom test targets
add_custom_target(run_unit_tests
    COMMAND unit_tests --gtest_output=xml:unit_test_results.xml
    DEPENDS unit_tests
    COMMENT "Running unit tests"
)

add_custom_target(run_all_tests  
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS unit_tests integration_tests regression_tests
    COMMENT "Running all tests"
)

# Coverage target (if coverage tools are available)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(GCOV_PATH gcov)
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(GCOV_PATH AND LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info
            COMMAND ${LCOV_PATH} --remove coverage.info '/usr/*' '*/test/*' --output-file coverage.info.cleaned
            COMMAND ${GENHTML_PATH} -o coverage coverage.info.cleaned
            COMMAND ${CMAKE_COMMAND} -E remove coverage.info coverage.info.cleaned
            DEPENDS unit_tests integration_tests
            COMMENT "Generating code coverage report"
        )
    endif()
endif()