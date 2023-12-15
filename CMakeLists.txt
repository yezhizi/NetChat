cmake_minimum_required(VERSION 3.24)
project(NetDesign2-Server VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set easylogingpp flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_NO_DEFAULT_LOG_FILE -DELPP_THREAD_SAFE")

# Directories
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Protobuf Directories
set(PROTO_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/proto")
file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")
set(PROTO_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Threads
find_package(Threads REQUIRED)

# Protobuf
include(FindProtobuf)
find_package(Protobuf REQUIRED)

# Use FetchContent to download third party dependencies
include(FetchContent)

# easyloggingpp
FetchContent_Declare(
    easyloggingpp
    URL https://github.com/abumq/easyloggingpp/archive/refs/tags/v9.97.1.tar.gz
    INACTIVITY_TIMEOUT 5
)

# libsodium
FetchContent_Declare(Sodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
    GIT_TAG f568ff02f1bed155ea598c0e803ef3c9db2703d2
    GIT_SHALLOW TRUE
)
set(SODIUM_DISABLE_TESTS ON)

# libuuid
FetchContent_Declare(libuuid
    GIT_REPOSITORY  https://github.com/gershnik/libuuid-cmake.git
    GIT_TAG         v2.39.3
    GIT_SHALLOW     TRUE
)

FetchContent_MakeAvailable(easyloggingpp Sodium libuuid)

# Source files
file(GLOB_RECURSE SOURCES "${SOURCE_DIR}/*.cpp")

# NetDesign2-Server executable
add_executable(NetDesign2-Server ${SOURCES} ${easyloggingpp_SOURCE_DIR}/src/easylogging++.cc)

protobuf_generate(
    TARGET NetDesign2-Server
    PROTOS ${PROTO_FILES}
    PROTOC_OUT_DIR ${PROTO_OUT_DIR}
    OUT_VAR PROTO_SRCS
)

# Link libraries
target_link_libraries(NetDesign2-Server 
    ${PROTOBUF_LIBRARY} 
    Threads::Threads
    uuid::uuid
    sodium
)

# Include directories
target_include_directories(NetDesign2-Server PRIVATE
    ${INCLUDE_DIR}
    ${PROTO_OUT_DIR}
    ${easyloggingpp_SOURCE_DIR}/src
    ${SOURCE_DIR}/Server
    ${SOURCE_DIR}/Client
)

# Set output directories
set_target_properties(NetDesign2-Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${LIB_DIR}"
)
