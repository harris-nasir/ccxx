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
| `-p, --path <dir>` | Output directory (default: current directory) |
| `-t, --type <type>` | Binary type: `exe`, `executable`, `lib`, or `library` |
| `-s, --std <num>` | C++ standard: `20`, `23`, `26` (default: `23`) |
| `-N, --namespace <name>` | Namespace for library code (default: project name) |
| `-H, --use-headers` | Generate header/source pair for library type |
| `-g, --git` | Initialize git repository (branch: main) |
| `-f, --force` | Overwrite existing project directory |
| `-h, --help` | Show help message |

### Examples

```console
ccxx myapp
ccxx -n myapp -s 20 -g
ccxx -n mylib -t lib -H
ccxx -n mylib -t lib -N myns --std 26
ccxx . -f
```

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

For library projects, a `source/<namespace>/` directory is created with
header and source files, and CMake is configured to build both the
library and a test executable.

When `--git` is used, a `.gitignore` is generated and the repository is
initialized with `main` as the default branch.
