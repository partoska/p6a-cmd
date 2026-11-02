# Contributing to p6a

Thank you for your interest in contributing! Here's everything you need to get started.

## Getting Started

### Prerequisites

- C99-capable compiler (GCC or Clang)
- CMake 3.20+
- libcurl with development headers
- [Bikini](https://github.com/partoska/bikini) 2.x, the INI library: install its
  release package (`.deb`, `.rpm` or `.pkg`), or unpack the pinned
  release archive with `.github/scripts/fetch-bikini.sh <platform> deps/bikini`
  and point the build at it (`cmake -DCMAKE_PREFIX_PATH=$PWD/deps/bikini`,
  `make BIKINI_PREFIX=$PWD/deps/bikini`)
- Git

### Building

```bash
git clone https://github.com/partoska/p6a-cmd.git
cd p6a-cmd

# Debug build with sanitizers (recommended for development)
./build.sh -d -s

# The binary will be at: build/src/cli/p6a
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
  - Library (`src/lib`): types `PascalCase` with `PK` prefix (e.g., `PKEvent`, `PKClient`), functions `camelCase` with `pk` prefix (e.g., `pkEventList`), macros `UPPER_SNAKE_CASE` with `PK_` prefix
  - Command line tool (`src/cli`): the same with `PKC`, `pkc` and `PKC_` (e.g., `PKCArg`, `pkcSync`, `PKC_INFO`); it uses the library's types (`PKInt`, `PKChar`) and only its public headers (`partoska.h`, `pktypes.h`)
- **Library rules**: Library calls run within a `PKClient`, return a `PKStatus` (`partoska.h`) and record the details with `pkFail()`; the library never prints (log through `pkLog()`), keeps no global state and never calls `exit()`.

## Adding a New Command

When adding a new command, update all of the following:

1. `src/cli/CMakeLists.txt` — add `src/<command>.c` to the source list
2. `Makefile` — add `$(SRCDIR)/cli/src/<command>.c` to `CLI_SRCS`
3. `src/cli/src/main.c` — add the command dispatch
4. `src/cli/src/arg.c` / `src/cli/src/arg.h` — add argument parsing
5. `src/lib/inc/partoska.h` and `src/lib/src/api.c` — add the API call, when the command needs a new endpoint
6. `README.md` — add to the commands table and add a full `### <command>` section
7. `assets/installer/README.linux.txt`
8. `assets/installer/README.macos.txt`
9. `assets/installer/README.windows.txt`

## Reporting Issues

Please use the GitHub issue tracker. Bug reports are most useful when they include:

- The version of `p6a` (`p6a --version`)
- Your operating system and architecture
- The exact command you ran
- The full output, including any error messages

## License

By contributing, you agree that your contributions will be licensed under the [MIT License](LICENSE.txt).
