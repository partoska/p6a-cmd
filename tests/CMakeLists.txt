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

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

# test_base64 — tests base64.c (no external dependencies)
add_executable(test_base64 test_base64.c ${SRC}/base64.c)
target_include_directories(test_base64 PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME base64 COMMAND test_base64)

# test_hash — tests hash.c (no external dependencies)
add_executable(test_hash test_hash.c ${SRC}/hash.c)
target_include_directories(test_hash PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME hash COMMAND test_hash)

# test_arg — tests arg.c (depends on logger.c for PL_ERROR/PL_WARN output)
add_executable(test_arg test_arg.c ${SRC}/arg.c ${SRC}/logger.c)
target_include_directories(test_arg PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME arg COMMAND test_arg)

# test_throttle — tests throttle.c (depends on logger.c)
add_executable(test_throttle test_throttle.c ${SRC}/throttle.c ${SRC}/logger.c)
target_include_directories(test_throttle PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME throttle COMMAND test_throttle)

# test_wdir — tests wdir.c (depends on fs.c, config.c, ini.c, logger.c)
add_executable(test_wdir test_wdir.c
    ${SRC}/wdir.c ${SRC}/fs.c ${SRC}/config.c ${SRC}/ini.c ${SRC}/logger.c)
target_include_directories(test_wdir PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME wdir COMMAND test_wdir)

# test_ini — tests ini.c (depends on fs.c, logger.c)
add_executable(test_ini test_ini.c ${SRC}/ini.c ${SRC}/fs.c ${SRC}/logger.c)
target_include_directories(test_ini PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME ini COMMAND test_ini)

# test_config — tests config.c (depends on ini.c, fs.c, logger.c)
add_executable(test_config test_config.c
    ${SRC}/config.c ${SRC}/ini.c ${SRC}/fs.c ${SRC}/logger.c)
target_include_directories(test_config PRIVATE ${SRC} ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME config COMMAND test_config)
