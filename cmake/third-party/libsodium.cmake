include(ExternalProject)

set(LIBSODIUM_INSTALL ${THIRD_PARTY_DIR}/libsodium)
set(LIBSODIUM_INCLUDE_DIR ${LIBSODIUM_INSTALL}/include)
set(LIBSODIUM_LIBRARY_DIR ${LIBSODIUM_INSTALL}/lib)
set(LIBSODIUM_VERSION "stable")
set(LIBSODIUM_URL "git@github.com:jedisct1/libsodium.git")

# Check if the module is already installed
if(NOT EXISTS ${LIBSODIUM_INCLUDE_DIR})
    ExternalProject_Add(
        LIBSODIUM
        PREFIX LIBSODIUM
        GIT_REPOSITORY ${LIBSODIUM_URL}
        GIT_TAG ${LIBSODIUM_VERSION}
        TMP_DIR ${TMP_DIR}/LIBSODIUM
        SOURCE_DIR ${LIBSODIUM_INSTALL}/src
        STAMP_DIR LIBSODIUM/LIBSODIUM-stamp
        DOWNLOAD_DIR ${TMP_DIR}
        CONFIGURE_COMMAND ${LIBSODIUM_INSTALL}/src/configure --prefix=${LIBSODIUM_INSTALL} --disable-shared --enable-static
        UPDATE_COMMAND ""
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        BUILD_ALWAYS 1  # Always build the project
    )
endif()

add_library(libsodium_imported STATIC IMPORTED)
set_target_properties(libsodium_imported PROPERTIES IMPORTED_LOCATION ${LIBSODIUM_LIBRARY_DIR}/libsodium.a)
