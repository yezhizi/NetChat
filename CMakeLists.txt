cmake_minimum_required(VERSION 3.20)
project(NetChat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_NO_DEFAULT_LOG_FILE")

set(THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party"
        CACHE PATH "Where to install third party headers and libs")  # third party libraries
    
set(TMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp")  # temporary directory 

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")  # source directory
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")  # include directory
set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")  # library directory

set(SERVER_DIR "${SOURCE_DIR}/Server")  # server source directory
set(CLIENT_DIR "${SOURCE_DIR}/Client")  # client source directory


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

file(GLOB SRC ${SOURCE_DIR}/*.cpp ${SOURCE_DIR}/Client/*.cpp ${SOURCE_DIR}/Server/*.cpp)
set(HEADERS ${SOURCE_DIR} ${INCLUDE_DIR} ${SERVER_DIR} ${CLIENT_DIR})



# test
add_executable(NetChat ${SRC} )
set_target_properties(NetChat PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
target_include_directories(NetChat PRIVATE ${EASYLOGGINGPP_INCLUDE_DIR} ${PROTOBUF_INCLUDE_DIR}  ${HEADERS})
target_link_libraries(NetChat PUBLIC easyloggingpp_imported proto_lib)






