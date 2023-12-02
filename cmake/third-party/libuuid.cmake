include (ExternalProject)

set(LIBUUID_INSTALL ${THIRD_PARTY_DIR}/uuid)
set(LIBUUID_INCLUDE_DIR ${LIBUUID_INSTALL}/include)
set(LIBUUID_LIBRARY_DIR ${LIBUUID_INSTALL}/lib)
set(LIBUUID_URL https://sourceforge.net/projects/libuuid/files/latest/download)

ExternalProject_Add(
    libuuid
    PREFIX libuuid
    URL ${LIBUUID_URL}
    TMP_DIR ${TMP_DIR}/libuuid
    SOURCE_DIR ${LIBUUID_INSTALL}/src
    STAMP_DIR libuuid/libuuid-stamp
    DOWNLOAD_DIR ${TMP_DIR}
    CONFIGURE_COMMAND ${LIBUUID_INSTALL}/src/configure --prefix=${LIBUUID_INSTALL} --enable-static=yes --enable-shared=no 

    BUILD_COMMAND make
    INSTALL_COMMAND make install
)

add_library(libuuid_imported STATIC IMPORTED)
set_target_properties(libuuid_imported PROPERTIES IMPORTED_LOCATION ${LIBUUID_LIBRARY_DIR}/libuuid.a)

