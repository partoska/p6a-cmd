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
VERSION = 1.7.0
PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

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
STRIP ?= strip

# Source files.
SRCS = $(SRCDIR)/hash.c \
       $(SRCDIR)/base64.c \
       $(SRCDIR)/oauth.c \
       $(SRCDIR)/api.c \
       $(SRCDIR)/arg.c \
       $(SRCDIR)/ini.c \
       $(SRCDIR)/config.c \
       $(SRCDIR)/sync.c \
       $(SRCDIR)/list.c \
       $(SRCDIR)/qr.c \
       $(SRCDIR)/link.c \
       $(SRCDIR)/create.c \
       $(SRCDIR)/update.c \
       $(SRCDIR)/media.c \
       $(SRCDIR)/download.c \
       $(SRCDIR)/logger.c \
       $(SRCDIR)/fs.c \
       $(SRCDIR)/wdir.c \
       $(SRCDIR)/rng.c \
       $(SRCDIR)/throttle.c \
       $(SRCDIR)/main.c \
       $(SRCDIR)/3rdparty/cJSON/cJSON.c

OBJS = $(SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Include directories.
INCLUDES = -I$(SRCDIR) \
           -I$(SRCDIR)/3rdparty/cJSON \
           -I$(SRCDIR)/3rdparty/curl/include

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

# Complete CFLAGS.
CFLAGS = $(CFLAGS_COMMON) $(CFLAGS_BUILD) $(INCLUDES)

# On Windows (MSYS2/MinGW), link statically so the .exe is self-contained.
ifeq ($(OS),Windows_NT)
    CFLAGS_COMMON += -DCURL_STATICLIB
    LDFLAGS += -static -static-libgcc
    LDFLAGS += $(shell pkg-config --libs --static libcurl 2>/dev/null || echo "-lcurl")
    LDFLAGS += -lws2_32 -lcrypt32
else
    LDFLAGS += $(shell pkg-config --libs libcurl 2>/dev/null || echo "-lcurl")
endif

# Target binary.
TARGET = $(BINDIR_BUILD)/$(PROJECT)

# Test configuration.
TESTDIR = tests
TESTBINDIR = $(BUILDDIR)/tests
TEST_CFLAGS = $(CFLAGS_BUILD) -I$(SRCDIR) -I$(TESTDIR)
TEST_BINS = $(TESTBINDIR)/test_base64 \
            $(TESTBINDIR)/test_hash \
            $(TESTBINDIR)/test_arg \
            $(TESTBINDIR)/test_throttle \
            $(TESTBINDIR)/test_wdir \
            $(TESTBINDIR)/test_ini \
            $(TESTBINDIR)/test_config

# Phony targets.
.PHONY: all clean install uninstall help debug release tests test

# Default target.
all: $(TARGET)

# Build target.
$(TARGET): $(OBJS) | $(BINDIR_BUILD)
	@echo "Linking $(PROJECT)..."
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@mkdir -p $(dir $@)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directories.
$(OBJDIR):
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/3rdparty/cJSON

$(BINDIR_BUILD):
	@mkdir -p $(BINDIR_BUILD)

$(TESTBINDIR):
	@mkdir -p $(TESTBINDIR)

$(TESTBINDIR)/test_base64: $(TESTDIR)/test_base64.c $(SRCDIR)/base64.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_hash: $(TESTDIR)/test_hash.c $(SRCDIR)/hash.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_arg: $(TESTDIR)/test_arg.c $(SRCDIR)/arg.c $(SRCDIR)/logger.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_throttle: $(TESTDIR)/test_throttle.c $(SRCDIR)/throttle.c $(SRCDIR)/logger.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_wdir: $(TESTDIR)/test_wdir.c $(SRCDIR)/wdir.c $(SRCDIR)/fs.c \
                         $(SRCDIR)/config.c $(SRCDIR)/ini.c $(SRCDIR)/logger.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_ini: $(TESTDIR)/test_ini.c $(SRCDIR)/ini.c $(SRCDIR)/fs.c \
                        $(SRCDIR)/logger.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

$(TESTBINDIR)/test_config: $(TESTDIR)/test_config.c $(SRCDIR)/config.c $(SRCDIR)/ini.c \
                           $(SRCDIR)/fs.c $(SRCDIR)/logger.c | $(TESTBINDIR)
	$(CC) $(TEST_CFLAGS) $^ -o $@

# Clean build artifacts.
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILDDIR)
	@echo "Clean complete."

# Install binary.
install: $(TARGET)
	@echo "Installing $(PROJECT) to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(PROJECT)
	@echo "Installation complete."

# Install and strip binary.
install-strip: $(TARGET)
	@echo "Installing $(PROJECT) to $(BINDIR) (stripped)..."
	install -d $(BINDIR)
	install -m 755 -s $(TARGET) $(BINDIR)/$(PROJECT)
	@echo "Installation complete."

# Uninstall binary.
uninstall:
	@echo "Uninstalling $(PROJECT) from $(BINDIR)..."
	rm -f $(BINDIR)/$(PROJECT)
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
	@echo "  make install                Install binary to $(BINDIR)."
	@echo "  make install-strip          Install and strip binary."
	@echo "  make uninstall              Remove installed binary."
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
	@echo "  CC=compiler                 C compiler (default: cc)."
	@echo ""
	@echo "Examples:"
	@echo ""
	@echo "  make BUILD_TYPE=debug SANITIZERS=1"
	@echo "  make BUILD_TYPE=debug VERBOSE_DEBUG=1"
	@echo "  make PREFIX=/usr install"
	@echo "  make clean && make"
	@echo ""
