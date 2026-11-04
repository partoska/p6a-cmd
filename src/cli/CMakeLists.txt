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

# The p6a executable.
add_executable(p6a
    src/arg.h src/arg.c src/log.h src/log.c src/report.h src/report.c
    src/plat.h src/plat.c
    src/ini.h src/ini.c src/list.h src/list.c src/media.h src/media.c
    src/create.h src/create.c src/update.h src/update.c src/edit.h src/edit.c
    src/approve.h src/approve.c src/qr.h src/qr.c src/card.h src/card.c
    src/link.h src/link.c src/sync.h src/sync.c src/download.h src/download.c
    src/join.h src/join.c src/trusted.h src/trusted.c
    src/main.c)
target_include_directories(p6a PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(p6a PRIVATE partoska Bikini::Bikini)

if(WIN32)
    # Linked statically, so that the .exe is self-contained (no DLL
    # dependencies on libcurl, libgcc, etc.).
    target_link_options(p6a PRIVATE -static)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_link_options(p6a PRIVATE -static-libgcc)
    endif()

    # Icon and version information of p6a.exe.
    set(P6A_ICON "${PROJECT_SOURCE_DIR}/assets/installer/p6a.ico")
    configure_file(src/p6a.rc.in "${CMAKE_CURRENT_BINARY_DIR}/p6a.rc" @ONLY)
    target_sources(p6a PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/p6a.rc")
endif()

# A shared libpartoska is installed next to the tool, in the library
# directory of the same prefix.
if(BUILD_SHARED_LIBS AND APPLE)
    set_target_properties(p6a PROPERTIES INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
elseif(BUILD_SHARED_LIBS AND UNIX)
    set_target_properties(p6a PROPERTIES INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")
endif()

install(TARGETS p6a RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
