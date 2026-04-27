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
    main_file << "#include <print>" << "\n";
    main_file << "" << "\n";
    main_file << "auto main() -> int" << "\n";
    main_file << "{" << "\n";
    main_file << "  std::println(\"Hello, world!\");" << "\n";
    main_file << "}" << "\n";
  }

  {
    std::ofstream cmake_file(project_root / "CMakeLists.txt");
    cmake_file << "cmake_minimum_required(VERSION 3.26)" << "\n";
    cmake_file << "project(" << options->name << " VERSION 0.1.0 LANGUAGES CXX)" << "\n";
    cmake_file << "set(CMAKE_CXX_STANDARD 23)" << "\n";
    cmake_file << "set(CMAKE_CXX_STANDARD_REQUIRED ON)" << "\n";
    cmake_file << "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)" << "\n";
    cmake_file << "\n";
    cmake_file << "file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS" << "\n";
    cmake_file << "  \"${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx\")" << "\n";
    cmake_file << "\n";
    cmake_file << "add_executable(${PROJECT_NAME} ${SOURCES})" << "\n";
  }

  {
    std::ofstream clangd_file(project_root / ".clangd");
    clangd_file << "CompileFlags:" << "\n";
    clangd_file << "  CompilationDatabase: ./build" << "\n";
    clangd_file << "  Add:" << "\n";
    clangd_file << "    - -std=c++23" << "\n";
    clangd_file << "    - -Wall" << "\n";
    clangd_file << "    - -Wextra" << "\n";
    clangd_file << "    - -Wpedantic" << "\n";
    clangd_file << "    - -Wconversion" << "\n";
    clangd_file << "    - -Wunused-parameter" << "\n";
    clangd_file << "    - -fvisibility=hidden" << "\n";
    clangd_file << "    - -march=native" << "\n";
    clangd_file << "" << "\n";
    clangd_file << "Diagnostics:" << "\n";
    clangd_file << "  UnusedIncludes: Strict" << "\n";
    clangd_file << "  ClangTidy:" << "\n";
    clangd_file << "    Add:" << "\n";
    clangd_file << "      [" << "\n";
    clangd_file << "        modernize-*," << "\n";
    clangd_file << "        performance-*," << "\n";
    clangd_file << "        readability-*," << "\n";
    clangd_file << "        bugprone-*," << "\n";
    clangd_file << "        cppcoreguidelines-*," << "\n";
    clangd_file << "        portability-*," << "\n";
    clangd_file << "        misc-*," << "\n";
    clangd_file << "        clang-analyzer-*," << "\n";
    clangd_file << "      ]" << "\n";
    clangd_file << "    Remove:" << "\n";
    clangd_file << "      [" << "\n";
    clangd_file << "        cppcoreguidelines-avoid-const-or-ref-data-members," << "\n";
    clangd_file << "        readability-identifier-length," << "\n";
    clangd_file << "        misc-non-private-member-variables-in-classes," << "\n";
    clangd_file << "      ]" << "\n";
    clangd_file << "    CheckOptions:" << "\n";
    clangd_file << "      readability-identifier-naming.VariableCase: lower_case" << "\n";
    clangd_file << "      readability-identifier-naming.FunctionCase: lower_case" << "\n";
    clangd_file << "      readability-identifier-naming.ClassCase: CamelCase" << "\n";
    clangd_file << "      readability-identifier-naming.StructCase: CamelCase" << "\n";
    clangd_file << "      readability-identifier-naming.NamespaceCase: lower_case" << "\n";
    clangd_file << "      readability-identifier-naming.ConstCase: CamelCase" << "\n";
    clangd_file << "      cppcoreguidelines-avoid-magic-numbers.IgnoredValues: \"0,1,-1\"" << "\n";
    clangd_file << "      modernize-use-trailing-return.CheckReturnVoid: true" << "\n";
    clangd_file << "      readability-identifier-naming.ConstantCase: UPPER_CASE" << "\n";
    clangd_file << "      GlobalConstantCase: UPPER_CASE" << "\n";
    clangd_file << "" << "\n";
    clangd_file << "InlayHints:" << "\n";
    clangd_file << "  Enabled: true" << "\n";
    clangd_file << "  ParameterNames: true" << "\n";
    clangd_file << "  DeducedTypes: true" << "\n";
    clangd_file << "  Designators: true" << "\n";
    clangd_file << "" << "\n";
    clangd_file << "Completion:" << "\n";
    clangd_file << "  AllScopes: true" << "\n";
    clangd_file << "" << "\n";
    clangd_file << "Index:" << "\n";
    clangd_file << "  Background: Skip" << "\n";
    clangd_file << "  StandardLibrary: true" << "\n";
    clangd_file << "  External: None" << "\n";
    clangd_file << "" << "\n";
    clangd_file << "Style:" << "\n";
    clangd_file << "  FullyQualifiedNamespaces: false" << "\n";
  }

  {
    std::ofstream readme_file(project_root / "README.md");
    readme_file << "# Build Instructions" << "\n";
    readme_file << "" << "\n";
    readme_file << "```console" << "\n";
    readme_file << "cmake -S . -B build" << "\n";
    readme_file << "cmake --build build" << "\n";
    readme_file << "```" << "\n";
    readme_file << "" << "\n";
    readme_file << "# Dependencies" << "\n";
    readme_file << "" << "\n";
    readme_file << "Add dependencies using git submodules." << "\n";
    readme_file << "" << "\n";
    readme_file << "```console" << "\n";
    readme_file << "git submodule add <repository_url> <path/to/dependency>" << "\n";
    readme_file << "```" << "\n";
  }

  {
    std::ofstream clang_format_file(project_root / ".clang-format");
    clang_format_file << "BasedOnStyle: LLVM" << "\n";
    clang_format_file << "IndentWidth: 2" << "\n";
    clang_format_file << "TabWidth: 2" << "\n";
    clang_format_file << "Language: Cpp" << "\n";
    clang_format_file << "AccessModifierOffset: -2" << "\n";
    clang_format_file << "UseTab: Never" << "\n";
    clang_format_file << "ColumnLimit: 100" << "\n";
    clang_format_file << "BreakBeforeBraces: Allman" << "\n";
    clang_format_file << "AllowShortIfStatementsOnASingleLine: true" << "\n";
    clang_format_file << "SortIncludes: CaseSensitive" << "\n";
    clang_format_file << "ConstructorInitializerAllOnOneLineOrOnePerLine: true" << "\n";
    clang_format_file << "IncludeBlocks: Preserve" << "\n";
    clang_format_file << "AlwaysBreakTemplateDeclarations: true" << "\n";
    clang_format_file << "AlwaysBreakAfterDefinitionReturnType: None" << "\n";
    clang_format_file << "DerivePointerAlignment: false" << "\n";
    clang_format_file << "BraceWrapping:" << "\n";
    clang_format_file << "  AfterClass: false" << "\n";
    clang_format_file << "  AfterControlStatement: false" << "\n";
    clang_format_file << "  AfterEnum: false" << "\n";
    clang_format_file << "  AfterFunction: false" << "\n";
    clang_format_file << "  AfterNamespace: false" << "\n";
    clang_format_file << "  AfterObjCDeclaration: false" << "\n";
    clang_format_file << "  AfterStruct: false" << "\n";
    clang_format_file << "  AfterUnion: false" << "\n";
    clang_format_file << "  BeforeCatch: false" << "\n";
    clang_format_file << "  BeforeElse: false" << "\n";
    clang_format_file << "  IndentBraces: false" << "\n";
    clang_format_file << "  SplitEmptyFunction: false" << "\n";
    clang_format_file << "  SplitEmptyNamespace: false" << "\n";
    clang_format_file << "  SplitEmptyRecord: false" << "\n";
    clang_format_file << "PointerAlignment: Left" << "\n";
    clang_format_file << "AllowShortLambdasOnASingleLine: true" << "\n";
    clang_format_file << "AlignConsecutiveAssignments: true" << "\n";
    clang_format_file << "AlignTrailingComments: true" << "\n";
    clang_format_file << "SpaceBeforeAssignmentOperators: true" << "\n";
    clang_format_file << "SpaceBeforeRangeBasedForLoopColon: true" << "\n";
    clang_format_file << "SpaceInEmptyBlock: false" << "\n";
    clang_format_file << "NamespaceIndentation: All" << "\n";
    clang_format_file << "BreakBeforeBinaryOperators: All" << "\n";
    clang_format_file << "BreakBeforeTernaryOperators: true" << "\n";
    clang_format_file << "IndentPPDirectives: AfterHash" << "\n";
    clang_format_file << "Standard: Latest" << "\n";
  }
}
