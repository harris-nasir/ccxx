# ccxx

A C++ project scaffold generator. Creates a ready-to-build C++ project
with CMake, clangd, and clang-format configuration.

## Build

```console
cmake -S . -B build
cmake --build build
```

## Usage

```console
ccxx <project_name> [options]
ccxx .                        # use current directory as project root
```

### Options

| Option | Description |
|--------|-------------|
| `-n, --name <name>` | Project name (or provide as positional argument) |
| `-p, --path <path>` | Project root directory (default: current directory) |
| `-t, --type <type>` | Binary type: `exe`, `executable`, `lib`, or `library` |
| `-h` | Enable header generation (library mode) |

If no type is specified, defaults to executable.

## Generated layout

```
<project>/
├── CMakeLists.txt
├── .clangd
├── .clang-format
├── README.md
└── source/
    └── main.cxx
```

For library projects, a `source/<project>/` directory is created with
header and source files, and CMake is configured to build both the
library and a test executable.
