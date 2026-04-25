#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <span>

namespace fs = std::filesystem;

namespace
{
  struct Options
  {
    std::string name;
    fs::path path;
  };

  auto create_options(std::span<char*> arguments) -> std::optional<Options>
  {
    if (arguments.empty())
    {
      return std::nullopt;
    }

    if (arguments.size() != 2)
    {
      std::cerr << "Provide a name for the binary \n";
      return std::nullopt;
    }

    return Options{
        .name = arguments[1],
        .path = fs::current_path(),
    };
  }
} // namespace

auto main(int argc, char** argv) -> int
{
  auto options = create_options(std::span<char*>(argv, static_cast<size_t>(argc)));
  if (!options)
  {
    return -1;
  }

  fs::path project_root{};

  if (options->name == ".")
  {
    options->name = fs::current_path().filename().string();
    project_root  = options->path;

    std::println("Creating binary \'{}\'", options->name);
  }
  else
  {
    project_root = options->path / options->name;
    if (fs::exists(project_root))
    {
      std::cerr << "Project root directory already exists " << project_root << "\n";
      return -1;
    }

    std::println("Creating binary \'{}\'", options->name);

    if (!fs::create_directory(project_root))
    {
      std::cerr << "Failed to create project root directory " << project_root << "\n";
      return -1;
    }
  }

  if (!fs::create_directory(project_root / "source"))
  {
    std::cerr << "Failed to create project root directory " << project_root / "source" << "\n";
    return -1;
  }

  {
    std::ofstream main_file(project_root / "source/main.cxx");
    main_file << R"(
    #include <print>

    auto main() -> int
    {
      std::println("Hello, world!");
    }
  )";
  }

  {
    std::ofstream cmake_file(project_root / "CMakeLists.txt");
    cmake_file <<
        R"(cmake_minimum_required(VERSION 3.26)
    project()" << options->name
               << R"( VERSION 0.1.0 LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx")

    add_executable(${PROJECT_NAME} ${SOURCES})
    )";
  }

  {
    std::ofstream clangd_file(project_root / ".clangd");
    clangd_file << R"(
      CompileFlags:
        CompilationDatabase: ./build
        Add:
          - -std=c++23
          - -Wall
          - -Wextra
          - -Wpedantic
          - -Wconversion
          - -Wunused-parameter
          - -fvisibility=hidden
          - -march=native

      Diagnostics:
        UnusedIncludes: Strict
        ClangTidy:
          Add:
            [
              modernize-*,
              performance-*,
              readability-*,
              bugprone-*,
              cppcoreguidelines-*,
              portability-*,
              misc-*,
              clang-analyzer-*,
            ]
          Remove:
            [
              cppcoreguidelines-avoid-const-or-ref-data-members,
              readability-identifier-length,
              misc-non-private-member-variables-in-classes,
            ]
          CheckOptions:
            readability-identifier-naming.VariableCase: lower_case
            readability-identifier-naming.FunctionCase: lower_case
            readability-identifier-naming.ClassCase: CamelCase
            readability-identifier-naming.StructCase: CamelCase
            readability-identifier-naming.NamespaceCase: lower_case
            readability-identifier-naming.ConstCase: CamelCase
            cppcoreguidelines-avoid-magic-numbers.IgnoredValues: "0,1,-1"
            modernize-use-trailing-return.CheckReturnVoid: true
            readability-identifier-naming.ConstantCase: UPPER_CASE
            GlobalConstantCase: UPPER_CASE

      InlayHints:
        Enabled: true
        ParameterNames: true
        DeducedTypes: true
        Designators: true

      Completion:
        AllScopes: true

      Index:
        Background: Skip
        StandardLibrary: true
        External: None

      Style:
        FullyQualifiedNamespaces: false
)";
  }

  {
    std::ofstream readme_file(project_root / "README.md");
    readme_file << R"(
      # Build Instructions

      ```console
      cmake -S . -B build
      cmake --build build
      ```

      # Dependencies

      Add dependencies using git submodules.

      ```console
      git submodule add <repository_url> <path/to/dependency>
      ```
    )";
  }

  {
    std::ofstream clang_format_file(project_root / ".clang-format");
    clang_format_file << R"(
      BasedOnStyle: LLVM
      IndentWidth: 2
      TabWidth: 2
      Language: Cpp
      AccessModifierOffset: -2
      UseTab: Never
      ColumnLimit: 100
      BreakBeforeBraces: Allman
      AllowShortIfStatementsOnASingleLine: true
      SortIncludes: CaseSensitive
      ConstructorInitializerAllOnOneLineOrOnePerLine: true
      IncludeBlocks: Preserve
      AlwaysBreakTemplateDeclarations: true
      AlwaysBreakAfterDefinitionReturnType: None
      DerivePointerAlignment: false
      BraceWrapping:
        AfterClass: false
        AfterControlStatement: false
        AfterEnum: false
        AfterFunction: false
        AfterNamespace: false
        AfterObjCDeclaration: false
        AfterStruct: false
        AfterUnion: false
        BeforeCatch: false
        BeforeElse: false
        IndentBraces: false
        SplitEmptyFunction: false
        SplitEmptyNamespace: false
        SplitEmptyRecord: false
      PointerAlignment: Left
      AllowShortLambdasOnASingleLine: true
      AlignConsecutiveAssignments: true
      AlignTrailingComments: true
      SpaceBeforeAssignmentOperators: true
      SpaceBeforeRangeBasedForLoopColon: true
      SpaceInEmptyBlock: false
      NamespaceIndentation: All
      BreakBeforeBinaryOperators: All
      BreakBeforeTernaryOperators: true
      IndentPPDirectives: AfterHash
      Standard: Latest
    )";
  }
}
