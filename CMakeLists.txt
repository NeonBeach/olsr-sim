cmake_minimum_required(VERSION 3.10)
project(olsr-sim)
set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)

# Automatically include all source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Print the files being included (useful for debugging)
message(STATUS "Source files: ${SOURCES}")

# Create executable
add_executable(olsr-sim ${SOURCES})
