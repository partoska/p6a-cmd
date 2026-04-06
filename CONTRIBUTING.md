# Contributing to p6a

Thank you for your interest in contributing! Here's everything you need to get started.

## Getting Started

### Prerequisites

- C99-capable compiler (GCC or Clang)
- CMake 3.13+
- libcurl with development headers
- Git

### Building

```bash
git clone https://github.com/partoska/p6a-cmd.git
cd p6a-cmd

# Debug build with sanitizers (recommended for development)
./build.sh -d -s

# The binary will be at: build/src/p6a
```

### Build options

| Flag | Description |
|------|-------------|
| `-d` | Debug build (includes debug symbols) |
| `-s` | Enable address and undefined behavior sanitizers |
| `-t` | Build and register unit tests |
| `-v` | Verbose debug logging (`DEBUG_SLOW` macro) |
| `-j N` | Use N parallel build jobs |

### Running tests

```bash
./build.sh -t
./test.sh
```

## Submitting Changes

1. Fork the repository and create a branch from `main`.
2. Make your changes — keep commits focused and atomic.
3. Run a debug build with sanitizers (`./build.sh -d -s`) and verify it builds cleanly.
4. Run the tests (`./test.sh`) and verify they all pass.
5. Format your code with `clang-format` (configuration is in `.clang-format`).
6. Open a pull request with a clear description of what you changed and why.

## Code Style

- **Standard**: C99 with POSIX extensions (`-D_XOPEN_SOURCE=700`)
- **Formatter**: `clang-format` (GNU style — run it before committing)
- **Naming**:
  - Public types: `PascalCase` with `PL` prefix (e.g., `PLEvent`, `PLCfg`)
  - Public functions: `camelCase` with `pl` prefix (e.g., `plApiEventList`)
  - Macros: `UPPER_SNAKE_CASE` with `PL_` prefix
- **Warnings**: All warnings are treated as errors (`-Wall -Werror -Wextra`) — fix them, don't suppress them.
- **Error handling**: Functions return `PLInt` status codes defined in `types.h`. Do not `exit()` from library code.

## Adding a New Command

When adding a new command, update all of the following:

1. `CMakeLists.txt` — add `src/<command>.c` to the source list
2. `Makefile` — add `$(SRCDIR)/<command>.c` to `SRCS`
3. `src/main.c` — add the command dispatch
4. `src/arg.c` / `src/arg.h` — add argument parsing
5. `README.md` — add to the commands table and add a full `### <command>` section
6. `assets/installer/README.linux.txt`
7. `assets/installer/README.macos.txt`
8. `assets/installer/README.windows.txt`

## Reporting Issues

Please use the GitHub issue tracker. Bug reports are most useful when they include:

- The version of `p6a` (`p6a --version`)
- Your operating system and architecture
- The exact command you ran
- The full output, including any error messages

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE.txt).
