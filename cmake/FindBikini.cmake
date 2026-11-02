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

# Finds the Bikini INI library (https://github.com/partoska/bikini) and
# provides the target Bikini::Bikini.
#
# pkg-config is used when available. Without it (e.g. on macOS), the header
# and library are located by hand; point CMAKE_PREFIX_PATH or Bikini_ROOT at
# an unpacked release archive to use one outside the default search paths.
#
# A major release changes the API, so with a version requested only that
# major is accepted: find_package(Bikini 2.0) takes 2.x, not 1.x or 3.x.
#
# Bikini::Bikini is a plain interface library rather than an imported one, so
# that its headers come in with -I: Apple's clang searches /usr/local/include
# before every -isystem directory, and a Bikini installed there would shadow
# the one found.

if(TARGET Bikini::Bikini)
    set(Bikini_FOUND TRUE)
    return()
endif()

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_BIKINI QUIET bikini)
endif()

if(PC_BIKINI_FOUND)
    set(Bikini_VERSION ${PC_BIKINI_VERSION})
    set(Bikini_INCLUDE_DIRS ${PC_BIKINI_INCLUDE_DIRS})
    set(Bikini_LIBRARIES ${PC_BIKINI_LINK_LIBRARIES})
    set(Bikini_REQUIRED_VARS PC_BIKINI_LINK_LIBRARIES)
else()
    find_path(Bikini_INCLUDE_DIR bikini.h)
    find_library(Bikini_LIBRARY bikini)
    set(Bikini_INCLUDE_DIRS ${Bikini_INCLUDE_DIR})
    set(Bikini_LIBRARIES ${Bikini_LIBRARY})
    set(Bikini_REQUIRED_VARS Bikini_LIBRARY Bikini_INCLUDE_DIR)

    # The version is in the header: #define BK_VERSION "2.0.0".
    if(Bikini_INCLUDE_DIR AND EXISTS "${Bikini_INCLUDE_DIR}/bikini.h")
        file(STRINGS "${Bikini_INCLUDE_DIR}/bikini.h" Bikini_VERSION_LINE
            REGEX "^#define BK_VERSION \"[^\"]*\"")
        string(REGEX REPLACE "^#define BK_VERSION \"([^\"]*)\".*" "\\1"
            Bikini_VERSION "${Bikini_VERSION_LINE}")
    endif()
endif()

if(Bikini_FIND_VERSION AND Bikini_VERSION)
    string(REGEX MATCH "^[0-9]+" Bikini_VERSION_MAJOR "${Bikini_VERSION}")
    if(NOT Bikini_VERSION_MAJOR EQUAL Bikini_FIND_VERSION_MAJOR)
        set(Bikini_FOUND FALSE)
        set(Bikini_MESSAGE "Bikini ${Bikini_VERSION} found, but ${Bikini_FIND_VERSION_MAJOR}.x is required (at least ${Bikini_FIND_VERSION})")
        if(Bikini_FIND_REQUIRED)
            message(FATAL_ERROR "${Bikini_MESSAGE}")
        elseif(NOT Bikini_FIND_QUIETLY)
            message(STATUS "${Bikini_MESSAGE}")
        endif()
        return()
    endif()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Bikini
    REQUIRED_VARS ${Bikini_REQUIRED_VARS} VERSION_VAR Bikini_VERSION)

if(Bikini_FOUND)
    add_library(pl_bikini INTERFACE)
    target_include_directories(pl_bikini INTERFACE ${Bikini_INCLUDE_DIRS})
    target_link_libraries(pl_bikini INTERFACE ${Bikini_LIBRARIES})
    if(PC_BIKINI_FOUND)
        target_compile_options(pl_bikini INTERFACE ${PC_BIKINI_CFLAGS_OTHER})
    endif()
    add_library(Bikini::Bikini ALIAS pl_bikini)
endif()

mark_as_advanced(Bikini_INCLUDE_DIR Bikini_LIBRARY Bikini_VERSION_LINE)
