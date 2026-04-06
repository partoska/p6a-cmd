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

cmake_minimum_required(VERSION 3.13)
project(p6a VERSION 1.7.0)

# P6A build.
add_subdirectory(src)

# Unit tests (opt-in via -DWITH_TESTS=ON).
option(WITH_TESTS "Build and register unit tests" OFF)
if(WITH_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# CPack packaging.
set(CPACK_PACKAGE_NAME "p6a")
set(CPACK_PACKAGE_VENDOR "Partoska Laboratory")
set(CPACK_PACKAGE_CONTACT "info@partoska.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Command-line interface for Partoska.com media sharing service")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/dist")

if(WIN32)
  set(CPACK_PACKAGE_FILE_NAME "p6a_${PROJECT_VERSION}_windows_amd64")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "p6a")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/WELCOME.windows.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/README.windows.txt")
  set(CPACK_GENERATOR "NSIS;ZIP;WIX")
  set(CPACK_WIX_UPGRADE_GUID "020BFD64-2ED0-44F7-A887-9723D5FC5110")
  set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/p6a.ico")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/p6a.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/p6a.ico")
  set(CPACK_NSIS_DISPLAY_NAME "Partoska Command-Line Interface")
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  set(CPACK_NSIS_MODIFY_PATH ON)
elseif(APPLE)
  set(CPACK_PACKAGE_FILE_NAME "p6a_${PROJECT_VERSION}_darwin_universal")
  cmake_policy(SET CMP0161 NEW)
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cpack")
  set(CPACK_GENERATOR "productbuild;TGZ")
  set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
  set(CPACK_COMMAND_PKGBUILD "${CMAKE_CURRENT_SOURCE_DIR}/cpack/pkgbuild_wrapper.sh")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/WELCOME.macos.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/README.macos.txt")
  set(CPACK_PRODUCTBUILD_IDENTIFIER "com.partoska.p6a")
  set(CPACK_PRODUCTBUILD_DOMAINS TRUE)
  set(CPACK_PRODUCTBUILD_DOMAINS_ANYWHERE TRUE)
  set(CPACK_PRODUCTBUILD_DOMAINS_USER FALSE)
  set(CPACK_PRODUCTBUILD_DOMAINS_ROOT TRUE)
else()
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(CPACK_ARCH "arm64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(CPACK_ARCH "arm")
  else()
    set(CPACK_ARCH "amd64")
  endif()
  set(CPACK_PACKAGE_FILE_NAME "p6a_${PROJECT_VERSION}_linux_${CPACK_ARCH}")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/WELCOME.linux.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/README.linux.txt")
  set(CPACK_GENERATOR "DEB;RPM;TGZ")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4")
  set(CPACK_DEBIAN_PACKAGE_SECTION "net")
  set(CPACK_RPM_PACKAGE_REQUIRES "libcurl")
  set(CPACK_RPM_PACKAGE_LICENSE "MIT")
endif()

include(CPack)
