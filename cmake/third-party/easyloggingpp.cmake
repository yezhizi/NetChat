include (ExternalProject)

set(EASYLOGGINGPP_INSTALL ${THIRD_PARTY_DIR}/easyloggingpp)
set(EASYLOGGINGPP_INCLUDE_DIR ${EASYLOGGINGPP_INSTALL}/include)
set(EASYLOGGINGPP_LIBRARY_DIR ${EASYLOGGINGPP_INSTALL}/lib)
set(EASYLOGGINGPP_URL https://github.com/abumq/easyloggingpp/archive/refs/tags/v9.97.1.tar.gz)

if(WIN32)
    set(EASYLOGGINGPP_LIBRARY_NAMES easyloggingpp.lib)
else()
    if(BUILD_SHARED_LIBS)
        if("${CMAKE_SHARED_LIBRARY_SUFFIX}" STREQUAL ".dylib")
            set(EASYLOGGINGPP_LIBRARY_NAMES libeasyloggingpp.dylib)
        elseif("${CMAKE_SHARED_LIBRARY_SUFFIX}" STREQUAL ".so")
            set(EASYLOGGINGPP_LIBRARY_NAMES libeasyloggingpp.so)
        else()
            message(FATAL_ERROR "${CMAKE_SHARED_LIBRARY_SUFFIX} not support for easyloggingpp")
        endif()
    else()
        set(EASYLOGGINGPP_LIBRARY_NAMES libeasyloggingpp.a)
        set(build_static_lib ON)
    endif()
endif()

foreach(LIBRARY_NAME ${EASYLOGGINGPP_LIBRARY_NAMES})
    list(APPEND EASYLOGGINGPP_STATIC_LIBRARIES ${EASYLOGGINGPP_LIBRARY_DIR}/${LIBRARY_NAME})
endforeach()

set(EASYLOGGINGPP_HEADERS
        "${EASYLOGGINGPP_INCLUDE_DIR}/easylogging++.h"
        )

ExternalProject_Add(easyloggingpp
        PREFIX easyloggingpp
        TMP_DIR ${TMP_DIR}/easyloggingpp
        URL ${EASYLOGGINGPP_URL}
        URL_MD5 763dbf5249dcf7a1585946502567f28d
        UPDATE_COMMAND ""
        BINARY_DIR easyloggingpp/easyloggingpp-build
        STAMP_DIR easyloggingpp/easyloggingpp-stamp
        DOWNLOAD_DIR ${TMP_DIR}

        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/easyloggingpp/src/easyloggingpp
        BUILD_BYPRODUCTS ${EASYLOGGINGPP_STATIC_LIBRARIES}
        CMAKE_CACHE_ARGS
        -DCMAKE_C_COMPILER_LAUNCHER:STRING=${CMAKE_C_COMPILER_LAUNCHER}
        -DCMAKE_CXX_COMPILER_LAUNCHER:STRING=${CMAKE_CXX_COMPILER_LAUNCHER}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
        -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
        -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
        -DCMAKE_INSTALL_PREFIX:STRING=${EASYLOGGINGPP_INSTALL}
        -Dbuild_static_lib:BOOL=${build_static_lib}
        )

add_library(easyloggingpp_imported STATIC IMPORTED GLOBAL)
set_property(TARGET easyloggingpp_imported PROPERTY IMPORTED_LOCATION "${EASYLOGGINGPP_STATIC_LIBRARIES}")
