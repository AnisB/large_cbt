cmake_minimum_required(VERSION 2.8...3.5)

# Define our project name
project(large_cbt)

# Declare the global variables for the cmake project
set(PROJECT_3RD_INCLUDES "${PROJECT_SOURCE_DIR}/3rd/include;${PROJECT_SOURCE_DIR}/3rd/include/libtiff")
set(PROJECT_3RD_LIBRARY ${PROJECT_SOURCE_DIR}/3rd/lib)
set(PROJECT_3RD_BINARY ${PROJECT_SOURCE_DIR}/3rd/bin)

# Demo SDK
set(DEMO_SDK_ROOT ${PROJECT_SOURCE_DIR}/demo)
set(DEMO_SDK_INCLUDE ${DEMO_SDK_ROOT}/include)

# Project root directory
set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/projects)

# Set the cmake path variable
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# This flag must be activated in order to handle properly folder flags
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Define the cmake macros
include(CMakePlatforms)
include(CMakeMacros)
include(CMakeBuildSettings)

# Define the build options
define_plaform_settings()

# Create the list of allowed files to be included
set(bacasable_source_extensions)
list(APPEND bacasable_source_extensions ".h" ".cpp" ".c" ".inl")

# Process the sub-dirs
add_subdirectory(${DEMO_SDK_ROOT}/src)
add_subdirectory(${PROJECT_ROOT})