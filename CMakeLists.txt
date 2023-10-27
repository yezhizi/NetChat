cmake_minimum_required(VERSION 3.20)
project(NetChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
        CACHE PATH "Where to install third party headers and libs")  # third party libraries
    
set(TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp")  # temporary directory 

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")  # source directory
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")  # library directory




# add submodules
set(MYCMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH ${MYCMAKE_DIR})
list(APPEND CMAKE_MODULE_PATH ${MYCMAKE_DIR}/third-party)

# easyloggingpp
include(easyloggingpp)

# protobuf
include(zlib)
include(protobuf)
include(proto2cpp)
find_package(Threads REQUIRED)



file(GLOB PROTO_FILES ${SOURCE_DIR}/*.proto)

foreach(proto_name ${PROTO_FILES})
    file(RELATIVE_PATH proto_rel_name ${SOURCE_DIR} ${proto_name})
    list(APPEND REL_PROTO_FILES ${proto_rel_name})
endforeach()

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${SOURCE_DIR} ${REL_PROTO_FILES})

add_library(proto_lib STATIC ${PROTO_SRCS} ${PROTO_HDRS})
set_target_properties(proto_lib PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
target_include_directories(proto_lib PUBLIC ${PROTOBUF_INCLUDE_DIR})
target_link_libraries(proto_lib PRIVATE ${PROTOBUF_STATIC_LIBRARIES} Threads::Threads)

file(GLOB SRC ${SOURCE_DIR}/*.cpp)
file(GLOB HEADERS ${SOURCE_DIR}/*.h)


# test


add_executable(NetChat ${SRC})
set_target_properties(NetChat PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
target_include_directories(NetChat PRIVATE ${EASYLOGGINGPP_INCLUDE_DIR} ${HEADERS} ${PROTOBUF_INCLUDE_DIR})
target_link_libraries(NetChat PUBLIC easyloggingpp_imported )






