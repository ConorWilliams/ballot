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
    NAME PicoSHA2
    GITHUB_REPOSITORY okdshin/PicoSHA2
    VERSION 1.0.0
)

if(PicoSHA2_ADDED)
    add_library(PicoSHA2 INTERFACE IMPORTED)
    target_include_directories(PicoSHA2 INTERFACE ${PicoSHA2_SOURCE_DIR})
    target_compile_features(PicoSHA2 INTERFACE cxx_std_11)
endif()




CPMAddPackage(
    NAME csv2
    GITHUB_REPOSITORY p-ranav/csv2
    GIT_TAG a20992f7a1b75215609037c4cc66f690e8691aab
)




# ---- Create executable ----

set(sources "src/main.cpp")

add_executable(ballot ${sources})

target_compile_features(ballot PUBLIC cxx_std_17)

target_compile_options(ballot PRIVATE -Wall -Wextra -Wpedantic -Wdisabled-optimization)

target_link_libraries(ballot PUBLIC structopt pcg-cpp LAPJV PicoSHA2 csv2)

target_include_directories(
    ballot PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
                  $<INSTALL_INTERFACE:src/${PROJECT_NAME}-${PROJECT_VERSION}>
)

# ///
