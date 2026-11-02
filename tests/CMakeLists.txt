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

set(LIB ${PROJECT_SOURCE_DIR}/src/lib)
set(CLI ${PROJECT_SOURCE_DIR}/src/cli)

# Library tests see the public and internal library headers, command line
# tests the public library headers and the command line sources.
function(pl_lib_test name)
    add_executable(test_${name} test_${name}.c ${ARGN})
    target_include_directories(test_${name} PRIVATE
        ${LIB}/inc ${LIB}/src ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND test_${name})
endfunction()

function(pl_cli_test name)
    add_executable(test_${name} test_${name}.c ${ARGN})
    target_include_directories(test_${name} PRIVATE
        ${LIB}/inc ${CLI}/src ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${name} COMMAND test_${name})
endfunction()

# test_base64 — tests base64.c (no external dependencies)
pl_lib_test(base64 ${LIB}/src/base64.c)

# test_throttle — tests throttle.c (depends on plat.c)
pl_lib_test(throttle ${LIB}/src/throttle.c ${LIB}/src/plat.c)

# The client core, which every other library test builds on.
set(LIB_CORE ${LIB}/src/client.c ${LIB}/src/config.c ${LIB}/src/plat.c
    ${LIB}/src/throttle.c)

# test_config — tests config.c (with the client core and Bikini)
pl_lib_test(config ${LIB_CORE})
target_link_libraries(test_config PRIVATE Bikini::Bikini)

# test_session — tests session.c: working directory, import, logout
pl_lib_test(session ${LIB}/src/session.c ${LIB_CORE})
target_link_libraries(test_session PRIVATE Bikini::Bikini)

# test_arg — tests the command line parser (depends on plat.c, log.c)
pl_cli_test(arg ${CLI}/src/arg.c ${CLI}/src/plat.c ${CLI}/src/log.c)

# test_ini — tests the INI files of sync (depends on plat.c, log.c and Bikini)
pl_cli_test(ini ${CLI}/src/ini.c ${CLI}/src/plat.c ${CLI}/src/log.c)
target_link_libraries(test_ini PRIVATE Bikini::Bikini)
