cmake_minimum_required(VERSION 3.0)

project(cmake_test)

set(LIB "../lib/hsb")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

ADD_DEFINITIONS(-DUNIT_TEST)

set(LIB_SOURCES
    ../lib/hsb/hsb.cpp
    ../lib/settings/settings.cpp
    ../lib/lightningutils/cmdhandler.cpp
    ../lib/lightningutils/basiceffects.cpp
    ../lib/lightningutils/basicfilters.cpp
    ../lib/lightningutils/colorcontrollerservice.cpp
    ../lib/utils/propertyutils.cpp
    ../.pio/libdeps/LC01_RGB/opt-parser/src/optparser.cpp
    ../lib/lightningutils/hsbToRGB.cpp
)

set(LIB_HEADERS
    ../lib/hsb
    ../lib/settings
    ../lib/lightningutils
    ../lib/command
    ../lib/libhelpers
    ../lib/utils
    ../lib/helpers
    ../.pio/libdeps/LC01_RGB/opt-parser/src
)

include_directories(catch2 ${LIB_HEADERS})

# Make test executable
add_executable(tests main.cpp src/arduinostubs.hpp ${LIB_SOURCES})
target_link_libraries(tests Catch)
