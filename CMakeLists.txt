cmake_minimum_required(VERSION 3.10)

# Set the project name
project(xcomms LANGUAGES C CXX ASM)

# Find the FLTK package
find_package(FLTK REQUIRED)

# Include FLTK directories
include_directories(${FLTK_INCLUDE_DIRS})

# Add include directories
include_directories(include resource)

# Find all source files
file(GLOB_RECURSE SOURCES "source/*.cpp" "source/*.cxx")
file(GLOB_RECURSE RESOURCE_SOURCES "resource/*.rc")
file(GLOB_RECURSE DATA_SOURCES "data/*.cxx")

list(APPEND SOURCES ${RESOURCE_SOURCES} ${DATA_SOURCES})

# # Add assembly source files (in order to use this CMake file on Windows, this file needs to be included)
#list(APPEND SOURCES "source/xboo_port.s")

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-multichar -Wunused -fno-rtti -fno-exceptions")



# Add the executable
add_executable(xcomms ${SOURCES})

# Link FLTK libraries
target_link_libraries(xcomms ${FLTK_LIBRARIES})
