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

cmake_minimum_required(VERSION 3.20)
project(p6a VERSION 2.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(BUILD_SHARED_LIBS "Build libpartoska as a shared library" OFF)
option(WITH_SANITIZERS "Use address and undefined sanitizers" OFF)
option(WITH_VERBOSE_DEBUG "Extra verbose mode in Debug build" OFF)
option(WITH_TESTS "Build and register unit tests" OFF)

include(GNUInstallDirs)

# Bikini 2.x, the INI library, linked statically from its release archive or
# package; point CMAKE_PREFIX_PATH at an unpacked archive outside the default
# search paths.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Bikini 2.0 REQUIRED)

# Compiler configuration shared by the tool and the unit tests. The options
# are limited to C, so that the resource compiler of Windows builds does not
# receive them.
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -D_DEBUG=1")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0 -DNDEBUG")
set(PL_C_OPTIONS -D_XOPEN_SOURCE=700 -pedantic
  -Wall -Werror -Wextra -Werror=return-type -Werror=array-bounds
  -Wno-deprecated-declarations)
add_compile_options("$<$<COMPILE_LANGUAGE:C>:${PL_C_OPTIONS}>")

if(WITH_SANITIZERS)
  set(PL_SANITIZERS -fno-omit-frame-pointer
    -fsanitize=undefined -fsanitize=address -fsanitize=float-cast-overflow
    -fsanitize-address-use-after-scope -fno-sanitize-recover)
  # Integer sanitizer is only supported by Clang compiler.
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    list(APPEND PL_SANITIZERS -fsanitize=integer)
  endif()
  add_compile_options("$<$<COMPILE_LANGUAGE:C>:${PL_SANITIZERS}>")
  add_link_options(${PL_SANITIZERS})
endif()

if(WITH_VERBOSE_DEBUG)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG_SLOW=1")
endif()

if(WIN32)
  # The icon and version information of p6a.exe.
  enable_language(RC)
endif()

# The library and the command line tool.
add_subdirectory(src/lib)
add_subdirectory(src/cli)

# Unit tests (opt-in via -DWITH_TESTS=ON).
if(WITH_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

install(FILES LICENSE.txt README.md THIRD_PARTY_LICENSES.txt
  DESTINATION ${CMAKE_INSTALL_DOCDIR})

# CPack packaging: installer and tarball on macOS, DEB, RPM and tarball on
# Linux, installers (NSIS, WiX) and ZIP on Windows. Every package holds the
# tool, the static library with its headers and pkg-config file, and the
# docs.
set(CPACK_PACKAGE_NAME "p6a")
set(CPACK_PACKAGE_VENDOR "Partoska Laboratory")
set(CPACK_PACKAGE_CONTACT "info@partoska.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Command-line interface and C library for Partoska.com media sharing service")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.partoska.com/p6a")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_STRIP_FILES TRUE)
set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/dist")

if(APPLE)
  # Release builds are universal (-DCMAKE_OSX_ARCHITECTURES="x86_64;arm64").
  set(CPACK_PACKAGE_FILE_NAME "p6a_${PROJECT_VERSION}_darwin_universal")
  # The productbuild identifier and domains below need CMake 3.23; older
  # CPack ignores them and builds a package that does not match
  # cpack/CPack.distribution.dist.in.
  if(POLICY CMP0161)
    cmake_policy(SET CMP0161 NEW)
  endif()
  list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cpack")
  set(CPACK_GENERATOR "productbuild;TGZ")
  set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
  set(CPACK_COMMAND_PKGBUILD "${CMAKE_CURRENT_SOURCE_DIR}/cpack/pkgbuild_wrapper.sh")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/WELCOME.macos.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/README.macos.txt")
  # Installer background image, referenced by cpack/CPack.distribution.dist.in.
  set(CPACK_PRODUCTBUILD_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/macos")
  set(CPACK_PRODUCTBUILD_IDENTIFIER "com.partoska.p6a")
  set(CPACK_PRODUCTBUILD_DOMAINS TRUE)
  set(CPACK_PRODUCTBUILD_DOMAINS_ANYWHERE TRUE)
  set(CPACK_PRODUCTBUILD_DOMAINS_USER FALSE)
  set(CPACK_PRODUCTBUILD_DOMAINS_ROOT TRUE)
elseif(UNIX)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(CPACK_ARCH "arm64")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(CPACK_ARCH "armhf")
  else()
    set(CPACK_ARCH "amd64")
  endif()
  set(CPACK_PACKAGE_FILE_NAME "p6a_${PROJECT_VERSION}_linux_${CPACK_ARCH}")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/WELCOME.linux.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/README.linux.txt")
  set(CPACK_GENERATOR "DEB;RPM;TGZ")
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcurl4t64 | libcurl4")
  set(CPACK_DEBIAN_PACKAGE_SECTION "net")
  set(CPACK_RPM_PACKAGE_REQUIRES "libcurl")
  # Programs linking libpartoska (static) also link Bikini; the tool alone
  # does not need it.
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS "bikini")
  set(CPACK_RPM_PACKAGE_SUGGESTS "bikini")
  set(CPACK_RPM_PACKAGE_LICENSE "MIT")
  set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")

  # Application icons for the freedesktop.org hicolor theme.
  set(P6A_ICON_SIZES 16x16 24x24 32x32 48x48 64x64 128x128 256x256 512x512 scalable)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/hicolor" DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
  # The icon theme and pkg-config directories belong to the system, not to
  # this package.
  set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    /usr/lib/pkgconfig
    /usr/share/icons
    /usr/share/icons/hicolor)
  foreach(size ${P6A_ICON_SIZES})
    list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
      /usr/share/icons/hicolor/${size}
      /usr/share/icons/hicolor/${size}/apps)
  endforeach()
elseif(WIN32)
  # A native CMAKE_SYSTEM_PROCESSOR is the host's, not the target's: MinGW
  # toolchains run natively on their target, others may know their target.
  if(CMAKE_C_COMPILER_ARCHITECTURE_ID)
    set(P6A_ARCH_ID "${CMAKE_C_COMPILER_ARCHITECTURE_ID}")
  else()
    set(P6A_ARCH_ID "${CMAKE_SYSTEM_PROCESSOR}")
  endif()
  if(P6A_ARCH_ID MATCHES "ARM64|aarch64|arm64")
    set(CPACK_ARCH "arm64")
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CPACK_ARCH "amd64")
  else()
    set(CPACK_ARCH "x86")
  endif()
  set(CPACK_PACKAGE_FILE_NAME "p6a_${PROJECT_VERSION}_windows_${CPACK_ARCH}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "p6a")
  set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/WELCOME.windows.txt")
  set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/README.windows.txt")

  # The installers are built when their tools are around; the ZIP always.
  set(CPACK_GENERATOR "ZIP")
  find_program(P6A_MAKENSIS makensis)
  if(P6A_MAKENSIS)
    list(APPEND CPACK_GENERATOR "NSIS")
  endif()
  find_program(P6A_WIX_CANDLE candle HINTS "$ENV{WIX}/bin")
  if(P6A_WIX_CANDLE)
    list(APPEND CPACK_GENERATOR "WIX")
  endif()

  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/p6a.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/p6a.ico")
  # The icon of Apps & features, taken from p6a.exe (src/p6a.rc.in). Written
  # unescaped into CPackConfig.cmake, so the backslash is escaped twice.
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\p6a.exe")
  set(CPACK_NSIS_DISPLAY_NAME "Partoska Command-Line Interface")
  set(CPACK_NSIS_PACKAGE_NAME "Partoska Command-Line Interface")
  set(CPACK_NSIS_URL_INFO_ABOUT "https://www.partoska.com/p6a")
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

  # Never change the upgrade code: it ties every version of the MSI
  # together, so a new one replaces the old.
  set(CPACK_WIX_UPGRADE_GUID "020BFD64-2ED0-44F7-A887-9723D5FC5110")
  set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/installer/p6a.ico")
  set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://www.partoska.com/p6a")
  set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cpack/wix_patch.xml")

  # The NSIS installer gets a _setup suffix, so it does not take the name
  # of the bare p6a_<version>_windows_<arch>.exe of a release.
  set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cpack/CPackOptions.cmake")
endif()

include(CPack)
