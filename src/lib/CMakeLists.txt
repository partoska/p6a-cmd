# Partoska Library - Client library for Partoska.com media sharing service.
# Copyright (C) 2026 Fabrika Charvat s.r.o. All rights reserved.
# Developed by Partoska Laboratory team, <https://lab.partoska.com>
#
# MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# You can contact the author(s) via email at ask <at> partoska.com.
#

# On Windows curl is linked statically, and pkg-config --static gives its
# full transitive dependency list (OpenSSL, zlib, nghttp2, etc.). On other
# systems the standard find module is used.
if(WIN32)
    find_package(PkgConfig REQUIRED)
    set(PKG_CONFIG_ARGN "--static")
    pkg_check_modules(CURL REQUIRED libcurl)
else()
    find_package(CURL REQUIRED)
endif()

add_library(partoska

    # Public includes
    inc/pktypes.h inc/partoska.h

    # Client, session and configuration
    src/client.h src/client.c src/session.c src/config.h src/config.c

    # Partoska.com API
    src/http.h src/http.c src/oauth.c src/api.c src/upload.c

    # Utilities
    src/throttle.h src/throttle.c src/base64.h src/base64.c

    # Platform layer
    src/plat.h src/plat.c

    # Vendored code
    ${PROJECT_SOURCE_DIR}/src/3rdparty/cJSON/cJSON.c)

target_include_directories(partoska
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/inc"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/3rdparty/cJSON"
        "${PROJECT_SOURCE_DIR}/src/3rdparty/curl/include")
target_link_libraries(partoska PRIVATE Bikini::Bikini)
set_target_properties(partoska PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    # A DLL exports nothing unless told; the API is functions only, so
    # exporting every symbol needs no dllexport in the headers.
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    PUBLIC_HEADER "inc/pktypes.h;inc/partoska.h")

if(WIN32)
    # Without CURL_STATICLIB the curl headers declare their functions
    # __declspec(dllimport), which forces dynamic linking.
    target_compile_definitions(partoska PRIVATE CURL_STATICLIB)
    target_include_directories(partoska PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(partoska PRIVATE ${CURL_STATIC_LIBRARIES})
    target_link_options(partoska PUBLIC ${CURL_STATIC_LDFLAGS_OTHER})

    # System libraries of a static libcurl (sockets, certificates, crypto),
    # in case its pkg-config file leaves them out.
    target_link_libraries(partoska PRIVATE ws2_32 crypt32 bcrypt)
else()
    target_link_libraries(partoska PRIVATE CURL::libcurl)
endif()

# pkg-config: a static library brings its dependencies to every link, a
# shared one only to static links. macOS has no libcurl.pc; its libcurl is a
# system library, named directly.
if(APPLE)
    set(PL_PC_DEPENDS "bikini")
    set(PL_PC_LIBS " -lcurl")
else()
    set(PL_PC_DEPENDS "libcurl bikini")
    set(PL_PC_LIBS "")
endif()
if(BUILD_SHARED_LIBS)
    set(PL_PC_REQUIRES "Requires.private: ${PL_PC_DEPENDS}")
else()
    set(PL_PC_REQUIRES "Requires: ${PL_PC_DEPENDS}")
endif()

# The prefix in partoska.pc is relative to the installed file, so it stays
# right under cmake --install --prefix and in unpacked archives. The base
# only has to be absolute (/ is not on Windows).
file(RELATIVE_PATH PL_PC_PREFIX
    "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}/pkgconfig" "${CMAKE_BINARY_DIR}")
string(REGEX REPLACE "/$" "" PL_PC_PREFIX "${PL_PC_PREFIX}")
configure_file(partoska.pc.in "${CMAKE_CURRENT_BINARY_DIR}/partoska.pc" @ONLY)

install(TARGETS partoska
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/partoska.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
