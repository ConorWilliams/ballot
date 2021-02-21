cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----

project(
    ChuBallotMCR
    VERSION 0.1
    DESCRIPTION "Code for the Churchill MCR Postgrad room-ballot"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
    )
endif()

# System packages
find_package(Threads)

# ---- Add dependencies via CPM ----

# see https://github.com/TheLartians/CPM.cmake for more info

include(cmake/CPM.cmake)
include(cmake/tools.cmake)

# Fetch all dependcies using CPM

CPMAddPackage(
    NAME Format.cmake
    VERSION 1.6
    GITHUB_REPOSITORY TheLartians/Format.cmake
    OPTIONS # enable cmake formatting (optional)
            "FORMAT_CHECK_CMAKE ON"
)

CPMAddPackage(
    NAME structopt
    GITHUB_REPOSITORY p-ranav/structopt
    # Version 0.1.1 commit
    GIT_TAG 78299d63f2d0c316b7509d32530fda8026637712
)

CPMAddPackage(
    NAME LAPJV
    GITHUB_REPOSITORY ConorWilliams/LAPJV-algorithm-c
    GIT_TAG 234cbe0bbc35e9f31792ca34567050910a9e215d
)

CPMAddPackage(
    NAME pcg-cpp
    GITHUB_REPOSITORY imneme/pcg-cpp
    VERSION 0.98.1
)

if(pcg-cpp_ADDED)
    add_library(pcg-cpp INTERFACE IMPORTED)
    target_include_directories(pcg-cpp INTERFACE ${pcg-cpp_SOURCE_DIR}/include)
    target_compile_features(pcg-cpp INTERFACE cxx_std_17)
endif()

CPMAddPackage(
    NAME fast-csv
    GITHUB_REPOSITORY ben-strasser/fast-cpp-csv-parser
    GIT_TAG 75600d0b77448e6c410893830df0aec1dbacf8e3
)

if(fast-csv_ADDED)
    add_library(fast-csv INTERFACE IMPORTED)
    target_include_directories(fast-csv INTERFACE ${fast-csv_SOURCE_DIR})
    target_compile_features(fast-csv INTERFACE cxx_std_11)
    target_link_libraries(fast-csv INTERFACE ${CMAKE_THREAD_LIBS_INIT})
endif()

# ---- Create executable ----

set(sources "src/main.cpp")

add_executable(ballot ${sources})

target_compile_features(ballot PUBLIC cxx_std_17)

target_compile_options(ballot PRIVATE -Wall -Wextra -Wpedantic -Wdisabled-optimization)

target_link_libraries(ballot PUBLIC structopt pcg-cpp LAPJV fast-csv)

target_include_directories(
    ballot PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
                  $<INSTALL_INTERFACE:src/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ///
