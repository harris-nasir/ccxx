# ccxx

A C++ project scaffold generator. Creates ready-to-build C++ projects with
CMake, clangd, and clang-format configuration.

Supports executable and library projects with multiple source styles:
separate headers and sources, header-only libraries (stb-style), and
C++20 module-based executables.

---

## Quick start

```console
# Build ccxx itself
cmake -S . -B build
cmake --build build

# Generate a new project
ccxx myapp
cd myapp
cmake -S . -B build && cmake --build build
./build/myapp
```

---

## Usage

```console
ccxx [options] [<project-name>]
ccxx .                              # use current directory as project root
```

### Options

| Option                    | Description                                                           |
|---------------------------|-----------------------------------------------------------------------|
| `-n, --name <name>`       | Project name (or provide as positional argument)                      |
| `-t, --type <type>`       | Project type: `exe` / `executable`, `lib` / `library`                 |
| `-p, --path <dir>`        | Output directory (default: current directory)                         |
| `-s, --std <num>`         | C++ standard: `20`, `23`, `26` (default: `23`)                        |
| `-N, --namespace <name>`  | Namespace for library code (default: project name)                    |
| `--style <style>`         | Source style: `separate`, `module`, `header-only`                     |
| `-g, --git`               | Initialize git repository (branch: main)                              |
| `-f, --force`             | Overwrite existing project directory                                  |
| `-h, --help`              | Show help message                                                     |

### Types and styles

The `--type` flag selects the project kind, and `--style` refines the
source layout within that kind.

| `--type`   | `--style`      | Result                                 |
|------------|----------------|----------------------------------------|
| `exe`      | `separate`     | Executable with plain `.cxx` sources   |
| `exe`      | `module`       | Executable using C++20 modules (`.ixx`)|
| `lib`      | `separate`     | Static library with `.hxx` / `.cxx`    |
| `lib`      | `header-only`  | Header-only library (stb-style)        |

Legacy type aliases (`static`, `static-lib`, `shared`, `shared-lib`,
`dynamic`) are accepted and map to `lib`. The legacy flag `--with` is
accepted as an alias for `--style`.

### Examples

```console
# Default executable (c++23)
ccxx myapp

# Module-based executable
ccxx -n myapp --type exe --style module

# Static library with separate sources
ccxx -n mylib --type lib

# Header-only library (stb-style, single header with IMPLEMENTATION guard)
ccxx -n mylib --type lib --style header-only

# Library with custom namespace
ccxx -n mylib --type lib --style header-only -N myns

# Executable at a specific path with C++20 and git init
ccxx -n myapp --std 20 -g -p ~/projects

# Overwrite an existing project with all flags
ccxx -n myapp --type lib --style header-only -N xyz -s 20 -g -f
```

---

## Generated layouts

### Executable (`--type exe`, default style)

```
<project>/
├── CMakeLists.txt
├── .clangd
├── .clang-format
├── README.md
└── source/
    └── main.cxx
```

### Module executable (`--type exe --style module`)

```
<project>/
├── CMakeLists.txt
├── .clangd
├── .clang-format
├── README.md
└── source/
    ├── main.cxx
    └── <project>.ixx
```

### Library (`--type lib`, default style)

```
<project>/
├── CMakeLists.txt
├── .clangd
├── .clang-format
├── README.md
└── source/
    └── <namespace>/
        ├── <project>.hxx
        └── <project>.cxx
```

### Header-only library (`--type lib --style header-only`)

```
<project>/
├── CMakeLists.txt
├── .clangd
├── .clang-format
├── README.md
└── source/
    └── <namespace>/
        └── <project>.hxx
```

---

## Output details

Generated CMakeLists.txt use `cmake_minimum_required(VERSION 4.2.3)`,
set `CMAKE_CXX_STANDARD` to the requested standard, and enable
`CMAKE_EXPORT_COMPILE_COMMANDS` for clangd integration.

- **Executable projects** use `add_executable()` with `GLOB_RECURSE`.
- **Module projects** use `target_sources()` with
  `FILE_SET CXX_MODULES PRIVATE`.
- **Library projects** use `add_library(... STATIC ...)` with
  `GLOB_RECURSE`.
- **Header-only projects** use `add_library(... INTERFACE)` with
  `target_include_directories(... INTERFACE source)`.

The `.clangd` configuration includes strict clang-tidy checks with the
project's coding conventions (Allman braces, 2-space indent, left pointer
alignment, trailing return types).

If `--git` is provided, a `.gitignore` is generated and the repository is
initialized with `main` as the default branch.
