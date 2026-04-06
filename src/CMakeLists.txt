# Command Line Interface for Partoska.com media sharing service.
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

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -D_XOPEN_SOURCE=700 -pedantic \
    -Wall -Werror -Wextra -Werror=return-type -Werror=array-bounds \
    -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -D_DEBUG=1")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0 -DNDEBUG")

option(WITH_SANITIZERS "Use sanitizers")
if(WITH_SANITIZERS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-omit-frame-pointer \
        -fsanitize=undefined -fsanitize=address -fsanitize=float-cast-overflow \
        -fsanitize-address-use-after-scope -fno-sanitize-recover")

    # Integer sanitizer is only supported by Clang compiler.
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=integer")
    endif()
endif()

option(WITH_VERBOSE_DEBUG "Extra verbose mode in Debug build")
if(WITH_VERBOSE_DEBUG)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_SLOW=1")
endif()

# On Windows, link everything statically so the .exe is self-contained
# (no DLL dependencies on libcurl, libgcc, etc.).
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc")

    # Required: without this, curl headers declare functions as __declspec(dllimport)
    # which forces dynamic linking regardless of whether libcurl.a is used.
    add_compile_definitions(CURL_STATICLIB)
endif()

# On Windows use pkg-config with --static to get the full transitive dep list
# (OpenSSL, zlib, nghttp2, etc.). On other systems use the standard find module.
if(WIN32)
    find_package(PkgConfig REQUIRED)
    set(PKG_CONFIG_ARGN "--static")
    pkg_check_modules(CURL REQUIRED libcurl)
else()
    find_package(CURL REQUIRED)
endif()

add_executable(p6a hash.c base64.c oauth.c api.c arg.c ini.c
    config.c sync.c list.c qr.c link.c create.c update.c media.c
    download.c logger.c fs.c wdir.c rng.c throttle.c main.c
    3rdparty/cJSON/cJSON.c)

# Include cJSON and CURL headers.
target_include_directories(p6a PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cJSON
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/curl/include
)

# Link against CURL and also link winsock, crypt32, and bcrypt
# which rng.c depend on.
if(WIN32)
    # CURL
    target_include_directories(p6a PRIVATE ${CURL_INCLUDE_DIRS})
    target_link_libraries(p6a PRIVATE ${CURL_STATIC_LIBRARIES})
    target_link_options(p6a PRIVATE ${CURL_STATIC_LDFLAGS_OTHER})

    # rng.c
    target_link_libraries(p6a PRIVATE ws2_32 crypt32 bcrypt)
else()
    target_link_libraries(p6a PRIVATE CURL::libcurl)
endif()

install(TARGETS p6a)
