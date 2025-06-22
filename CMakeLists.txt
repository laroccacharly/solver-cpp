cmake_minimum_required(VERSION 3.10)

project(solver-cpp)

option(CXX "enable C++ compilation" ON)

# Export commands for vscode linting 
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Add the current directory to CMAKE_MODULE_PATH so we can find FindGurobi.cmake
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

# Enable C++ 
enable_language(CXX)
set(CMAKE_CXX_STANDARD 17)

# Debug configuration - Add debug symbols and sanitizers
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")

# Set default build type to Debug if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(${PROJECT_NAME} main.cpp src/diet_c++.cpp)

# Link fmt
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# Link Gurobi libraries
find_package(GUROBI REQUIRED)
message(STATUS "Gurobi include dirs: ${GUROBI_INCLUDE_DIRS}")
include_directories(${GUROBI_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE ${GUROBI_CXX_LIBRARY})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GUROBI_LIBRARY})