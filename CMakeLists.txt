cmake_minimum_required(VERSION 3.24)
project(NetDesign2-Server VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directories
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# Protobuf Directories
set(PROTO_SRC_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/proto")
file(GLOB PROTO_FILES "${PROTO_SRC_DIR}/*.proto")

set(SERVER_DIR "${SOURCE_DIR}/Server")
set(CLIENT_DIR "${SOURCE_DIR}/Client")


# CMake module paths
list(APPEND CMAKE_MODULE_PATH ${MYCMAKE_DIR})
list(APPEND CMAKE_MODULE_PATH ${MYCMAKE_DIR}/third-party)

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
FetchContent_Declare(
    libsodium
    URL https://github.com/jedisct1/libsodium/releases/download/1.0.19-RELEASE/libsodium-1.0.19.tar.gz
    INACTIVITY_TIMEOUT 5
)

# libuuid
FetchContent_Declare(
    libuuid
    URL https://sourceforge.net/projects/libuuid/files/libuuid-1.0.3.tar.gz
    INACTIVITY_TIMEOUT 5
)

FetchContent_MakeAvailable(easyloggingpp libsodium libuuid)

# Source files
set(SRC_FILES
    ${SOURCE_DIR}/main.cpp
    ${SERVER_DIR}/server_van.h
    ${SERVER_DIR}/Server.h
    ${SERVER_DIR}/Server.cpp
)

# NetDesign2-Server executable
add_executable(NetDesign2-Server ${SRC_FILES})

protobuf_generate(
    TARGET NetDesign2-Server
    PROTOS ${PROTO_FILES}
    OUT_VAR PB_SRCS
)

# Link libraries
target_link_libraries(NetDesign2-Server 
    ${PROTOBUF_LIBRARY} 
    Threads::Threads
    easyloggingpp
    sodium
    uuid
)

# Include directories
target_include_directories(NetDesign2-Server PRIVATE
    ${INCLUDE_DIR}
    ${PROTOBUF_INCLUDE_DIR}
)

# Set output directories
set_target_properties(NetDesign2-Server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${LIB_DIR}"
)
