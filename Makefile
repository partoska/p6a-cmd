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

# Project configuration.
PROJECT = p6a
LIBRARY = partoska
VERSION = 2.0.0
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
LIBDIR ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include
PKGCONFIGDIR ?= $(LIBDIR)/pkgconfig

# Build configuration.
BUILD_TYPE ?= release
SANITIZERS ?= 0
VERBOSE_DEBUG ?= 0

# Directories.
SRCDIR = src
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
BINDIR_BUILD = $(BUILDDIR)

# Compiler configuration.
CC ?= cc
AR ?= ar
ARFLAGS = rcs
STRIP ?= strip

# Source files; keep them in step with src/lib/CMakeLists.txt and
# src/cli/CMakeLists.txt.
LIB_SRCS = $(SRCDIR)/lib/src/client.c \
           $(SRCDIR)/lib/src/session.c \
           $(SRCDIR)/lib/src/config.c \
           $(SRCDIR)/lib/src/http.c \
           $(SRCDIR)/lib/src/oauth.c \
           $(SRCDIR)/lib/src/api.c \
           $(SRCDIR)/lib/src/upload.c \
           $(SRCDIR)/lib/src/throttle.c \
           $(SRCDIR)/lib/src/base64.c \
           $(SRCDIR)/lib/src/plat.c \
           $(SRCDIR)/3rdparty/cJSON/cJSON.c

CLI_SRCS = $(SRCDIR)/cli/src/arg.c \
           $(SRCDIR)/cli/src/log.c \
           $(SRCDIR)/cli/src/report.c \
           $(SRCDIR)/cli/src/plat.c \
           $(SRCDIR)/cli/src/ini.c \
           $(SRCDIR)/cli/src/list.c \
           $(SRCDIR)/cli/src/media.c \
           $(SRCDIR)/cli/src/create.c \
           $(SRCDIR)/cli/src/update.c \
           $(SRCDIR)/cli/src/edit.c \
           $(SRCDIR)/cli/src/approve.c \
           $(SRCDIR)/cli/src/qr.c \
           $(SRCDIR)/cli/src/card.c \
           $(SRCDIR)/cli/src/link.c \
           $(SRCDIR)/cli/src/sync.c \
           $(SRCDIR)/cli/src/download.c \
           $(SRCDIR)/cli/src/main.c

LIB_OBJS = $(LIB_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
CLI_OBJS = $(CLI_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS = $(LIB_OBJS:.o=.d) $(CLI_OBJS:.o=.d)

# Include directories: the library sees its public and internal headers,
# the command line tool only the public ones.
LIB_INCLUDES = -I$(SRCDIR)/lib/inc \
               -I$(SRCDIR)/lib/src \
               -I$(SRCDIR)/3rdparty/cJSON \
               -I$(SRCDIR)/3rdparty/curl/include
CLI_INCLUDES = -I$(SRCDIR)/lib/inc \
               -I$(SRCDIR)/cli/src

# Compiler flags
CFLAGS_COMMON = -std=c99 -D_XOPEN_SOURCE=700 -pedantic \
                -Wall -Werror -Wextra -Werror=return-type -Werror=array-bounds \
                -Wno-deprecated-declarations

CFLAGS_DEBUG = -O0 -g3 -D_DEBUG=1
CFLAGS_RELEASE = -Os -g0 -DNDEBUG

# Build type selection.
ifeq ($(BUILD_TYPE),debug)
    CFLAGS_BUILD = $(CFLAGS_DEBUG)
else
    CFLAGS_BUILD = $(CFLAGS_RELEASE)
endif

# Verbose debug flag.
ifeq ($(VERBOSE_DEBUG),1)
    CFLAGS_BUILD += -DDEBUG_SLOW=1
endif

# Sanitizer flags.
ifeq ($(SANITIZERS),1)
    SANITIZER_FLAGS = -fno-omit-frame-pointer \
                      -fsanitize=undefined \
                      -fsanitize=address \
                      -fsanitize=float-cast-overflow \
                      -fsanitize-address-use-after-scope \
                      -fno-sanitize-recover

    # Add integer sanitizer for Clang.
    ifeq ($(shell $(CC) --version 2>/dev/null | grep -i clang),)
        # No Clang (e.g. GCC) - No integer sanitizer.
    else
        # Clang - Add integer sanitizer.
        SANITIZER_FLAGS += -fsanitize=integer
    endif

    CFLAGS_COMMON += $(SANITIZER_FLAGS)
    LDFLAGS += $(SANITIZER_FLAGS)
endif

# Bikini 2.x (https://github.com/partoska/bikini), the INI library, installed
# from its release packages: pkg-config, else the compiler's default paths or
# BIKINI_PREFIX (e.g. an unpacked release archive). Its headers go in with -I,
# not -isystem: Apple's clang searches /usr/local/include before every
# -isystem directory.
ifeq ($(shell pkg-config --exists bikini 2>/dev/null && echo yes),yes)
    BIKINI_CFLAGS := $(shell pkg-config --cflags bikini)
    BIKINI_LIBS := $(shell pkg-config --libs bikini)
else
    BIKINI_CFLAGS := $(if $(BIKINI_PREFIX),-I$(BIKINI_PREFIX)/include)
    BIKINI_LIBS := $(if $(BIKINI_PREFIX),-L$(BIKINI_PREFIX)/lib) -lbikini
endif

# Complete CFLAGS; DEPFLAGS rebuild objects when a header changes.
CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_BUILD) $(BIKINI_CFLAGS)
DEPFLAGS = -MMD -MP

# On Windows (MSYS2/MinGW), link statically so the .exe is self-contained.
ifeq ($(OS),Windows_NT)
    CFLAGS_COMMON += -DCURL_STATICLIB
    LDFLAGS += -static -static-libgcc
    LDFLAGS += $(shell pkg-config --libs --static libcurl 2>/dev/null || echo "-lcurl")
    LDFLAGS += -lws2_32 -lcrypt32 -lbcrypt
else
    LDFLAGS += $(shell pkg-config --libs libcurl 2>/dev/null || echo "-lcurl")
endif

# Target library and binary.
LIBTARGET = $(BUILDDIR)/lib$(LIBRARY).a
LIB_HDRS = $(SRCDIR)/lib/inc/pktypes.h $(SRCDIR)/lib/inc/partoska.h
PCTARGET = $(BUILDDIR)/$(LIBRARY).pc

# pkg-config: the static library brings libcurl and Bikini to every link;
# macOS has no libcurl.pc, its libcurl is named directly.
ifeq ($(shell uname -s 2>/dev/null),Darwin)
    PC_DEPENDS = bikini
    PC_LIBS = -lcurl
else
    PC_DEPENDS = libcurl bikini
    PC_LIBS =
endif
TARGET = $(BINDIR_BUILD)/$(PROJECT)

# Test configuration.
TESTDIR = tests
TESTBINDIR = $(BUILDDIR)/tests
TEST_CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_BUILD) -I$(TESTDIR) $(BIKINI_CFLAGS)
LIB_TEST_CFLAGS = $(TEST_CFLAGS) -I$(SRCDIR)/lib/inc -I$(SRCDIR)/lib/src
LIB_CORE = $(SRCDIR)/lib/src/client.c $(SRCDIR)/lib/src/config.c \
           $(SRCDIR)/lib/src/plat.c $(SRCDIR)/lib/src/throttle.c
CLI_TEST_CFLAGS = $(TEST_CFLAGS) -I$(SRCDIR)/lib/inc -I$(SRCDIR)/cli/src
TEST_BINS = $(TESTBINDIR)/test_base64 \
            $(TESTBINDIR)/test_throttle \
            $(TESTBINDIR)/test_config \
            $(TESTBINDIR)/test_session \
            $(TESTBINDIR)/test_arg \
            $(TESTBINDIR)/test_ini

# Phony targets.
.PHONY: all clean install install-strip uninstall help debug release tests test

# Default target.
all: $(TARGET)

# Build target.
$(TARGET): $(CLI_OBJS) $(LIBTARGET) | $(BINDIR_BUILD)
	@echo "Linking $(PROJECT)..."
	$(CC) $(CLI_OBJS) $(LIBTARGET) -o $@ $(BIKINI_LIBS) $(LDFLAGS)
	@echo "Build complete: $@"

# Library archive.
$(LIBTARGET): $(LIB_OBJS)
	@echo "Archiving lib$(LIBRARY)..."
	$(AR) $(ARFLAGS) $@ $(LIB_OBJS)

# Compile source files.
$(OBJDIR)/lib/%.o: $(SRCDIR)/lib/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(LIB_INCLUDES) $(DEPFLAGS) -c $< -o $@

$(OBJDIR)/3rdparty/%.o: $(SRCDIR)/3rdparty/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(LIB_INCLUDES) $(DEPFLAGS) -c $< -o $@

$(OBJDIR)/cli/%.o: $(SRCDIR)/cli/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(CLI_INCLUDES) $(DEPFLAGS) -c $< -o $@

# Create build directories.
$(BINDIR_BUILD):
	@mkdir -p $(BINDIR_BUILD)

$(TESTBINDIR):
	@mkdir -p $(TESTBINDIR)

$(TESTBINDIR)/test_base64: $(TESTDIR)/test_base64.c $(SRCDIR)/lib/src/base64.c | $(TESTBINDIR)
	$(CC) $(LIB_TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_throttle: $(TESTDIR)/test_throttle.c $(SRCDIR)/lib/src/throttle.c \
                             $(SRCDIR)/lib/src/plat.c | $(TESTBINDIR)
	$(CC) $(LIB_TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_config: $(TESTDIR)/test_config.c $(LIB_CORE) | $(TESTBINDIR)
	$(CC) $(LIB_TEST_CFLAGS) $^ -o $@ $(BIKINI_LIBS)

$(TESTBINDIR)/test_session: $(TESTDIR)/test_session.c $(SRCDIR)/lib/src/session.c \
                            $(LIB_CORE) | $(TESTBINDIR)
	$(CC) $(LIB_TEST_CFLAGS) $^ -o $@ $(BIKINI_LIBS)

$(TESTBINDIR)/test_arg: $(TESTDIR)/test_arg.c $(SRCDIR)/cli/src/arg.c \
                        $(SRCDIR)/cli/src/plat.c $(SRCDIR)/cli/src/log.c | $(TESTBINDIR)
	$(CC) $(CLI_TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_ini: $(TESTDIR)/test_ini.c $(SRCDIR)/cli/src/ini.c $(SRCDIR)/cli/src/plat.c \
                        $(SRCDIR)/cli/src/log.c | $(TESTBINDIR)
	$(CC) $(CLI_TEST_CFLAGS) $^ -o $@ $(BIKINI_LIBS)

# Clean build artifacts.
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILDDIR)
	@echo "Clean complete."

# The pkg-config file, for the installation prefix.
$(PCTARGET): $(SRCDIR)/lib/partoska.pc.in | $(BINDIR_BUILD)
	sed -e 's|$${pcfiledir}/@PL_PC_PREFIX@|$(PREFIX)|' \
	    -e 's|$${prefix}/@CMAKE_INSTALL_LIBDIR@|$(LIBDIR)|' \
	    -e 's|$${prefix}/@CMAKE_INSTALL_INCLUDEDIR@|$(INCLUDEDIR)|' \
	    -e 's|@PROJECT_VERSION@|$(VERSION)|' \
	    -e 's|@PL_PC_REQUIRES@|Requires: $(PC_DEPENDS)|' \
	    -e 's|@PL_PC_LIBS@|$(if $(PC_LIBS), $(PC_LIBS))|' \
	    $< > $@

# Install the tool, the library, its headers and pkg-config file.
install: $(TARGET) $(PCTARGET)
	@echo "Installing $(PROJECT) and lib$(LIBRARY) to $(PREFIX)..."
	install -d $(BINDIR) $(LIBDIR) $(INCLUDEDIR) $(PKGCONFIGDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(PROJECT)
	install -m 644 $(LIBTARGET) $(LIBDIR)/lib$(LIBRARY).a
	install -m 644 $(LIB_HDRS) $(INCLUDEDIR)/
	install -m 644 $(PCTARGET) $(PKGCONFIGDIR)/$(LIBRARY).pc
	@echo "Installation complete."

# Install, with the tool stripped.
install-strip: install
	$(STRIP) $(BINDIR)/$(PROJECT)

# Remove the installed files.
uninstall:
	@echo "Uninstalling $(PROJECT) and lib$(LIBRARY) from $(PREFIX)..."
	rm -f $(BINDIR)/$(PROJECT) $(LIBDIR)/lib$(LIBRARY).a
	rm -f $(INCLUDEDIR)/pktypes.h $(INCLUDEDIR)/partoska.h
	rm -f $(PKGCONFIGDIR)/$(LIBRARY).pc
	@echo "Uninstall complete."

# Convenience targets for build types.
debug:
	@$(MAKE) BUILD_TYPE=debug

release:
	@$(MAKE) BUILD_TYPE=release

tests: $(TEST_BINS)

test: tests
	@failed=0; \
	for t in $(TEST_BINS); do \
	    $$t || failed=1; \
	done; \
	if [ $$failed -eq 0 ]; then echo "All tests passed."; else echo "Some tests FAILED."; exit 1; fi

# Help target.
help:
	@echo "Makefile for p6a - Build targets:"
	@echo ""
	@echo "  make                        Build release version (default)."
	@echo "  make debug                  Build debug version."
	@echo "  make release                Build release version."
	@echo "  make clean                  Remove build artifacts."
	@echo "  make install                Install tool, library, headers and pkg-config file to $(PREFIX)."
	@echo "  make install-strip          Install, with the tool stripped."
	@echo "  make uninstall              Remove the installed files."
	@echo "  make tests                  Build unit test binaries."
	@echo "  make test                   Build and run unit tests."
	@echo "  make help                   Show this help message."
	@echo ""
	@echo "Build options (set as environment variables or make arguments):"
	@echo ""
	@echo "  BUILD_TYPE=release|debug    Build type (default: release)."
	@echo "  SANITIZERS=0|1              Enable sanitizers (default: 0)."
	@echo "  VERBOSE_DEBUG=0|1           Enable verbose debug logging (default: 0)."
	@echo "  PREFIX=/path                Installation prefix (default: /usr/local)."
	@echo "  BIKINI_PREFIX=/path         Bikini install without pkg-config (e.g. /usr/local)."
	@echo "  CC=compiler                 C compiler (default: cc)."
	@echo ""
	@echo "Examples:"
	@echo ""
	@echo "  make BUILD_TYPE=debug SANITIZERS=1"
	@echo "  make BUILD_TYPE=debug VERBOSE_DEBUG=1"
	@echo "  make PREFIX=/usr install"
	@echo "  make clean && make"
	@echo ""

-include $(DEPS)
