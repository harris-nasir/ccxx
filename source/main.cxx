#include "common.hxx"
#include "executable.hxx"
#include "library.hxx"
#include "module.hxx"
#include "tui.hxx"

#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace fs = std::filesystem;

namespace
{
  constexpr auto VERSION = "0.3.1";

  class arguments
  {
  public:
    arguments(int argc, char** argv, std::initializer_list<std::pair<std::string_view, std::string_view>> aliases = {})
    {
      arguments_ = std::span<char*>(argv, static_cast<size_t>(argc));

      for (const auto& [from, to] : aliases)
      {
        alias_map_[from] = to;
        alias_map_[to]   = from;
      }

      bool end_of_options = false;

      for (size_t index = 1; index < arguments_.size(); ++index)
      {
        std::string_view current = arguments_[index];

        if (!end_of_options && current == "--")
        {
          end_of_options = true;
          continue;
        }

        if (!end_of_options && current.starts_with('-'))
        {
          const auto equals_position = current.find('=');
          if (equals_position != std::string_view::npos)
          {
            options_[current.substr(0, equals_position)] = current.substr(equals_position + 1);
            continue;
          }

          if ((index + 1) < arguments_.size())
          {
            std::string_view next = arguments_[index + 1];

            if (!next.starts_with('-'))
            {
              options_[current] = next;
              ++index;
              continue;
            }
          }

          options_[current] = {};
        }
        else
        {
          positional_arguments_.push_back(current);
        }
      }
    }

    [[nodiscard]]
    auto has(std::string_view option) const -> bool
    {
      if (options_.contains(option))
      {
        return true;
      }

      const auto it = alias_map_.find(option);
      return it != alias_map_.end() && options_.contains(it->second);
    }

    [[nodiscard]]
    auto get(std::string_view option) const -> std::string
    {
      {
        const auto it = options_.find(option);
        if (it != options_.end())
        {
          return std::string{it->second};
        }
      }

      const auto alias_it = alias_map_.find(option);
      if (alias_it != alias_map_.end())
      {
        const auto it = options_.find(alias_it->second);
        if (it != options_.end())
        {
          return std::string{it->second};
        }
      }

      return {};
    }

    [[nodiscard]]
    auto positional() const -> const std::vector<std::string_view>&
    {
      return positional_arguments_;
    }

  private:
    std::span<char*> arguments_;
    std::unordered_map<std::string_view, std::string_view> options_;
    std::vector<std::string_view> positional_arguments_;
    std::unordered_map<std::string_view, std::string_view> alias_map_;
  };

  auto create_options_from_arguments(const arguments& arguments) -> ccxx::options
  {
    ccxx::options options{};

    if (arguments.has("--name"))
    {
      options.project_name = arguments.get("--name");
    }
    else
    {
      const auto& position_arguments = arguments.positional();
      if (!position_arguments.empty())
      {
        options.project_name = position_arguments[0];
      }
    }

    if (arguments.has("--type"))
    {
      std::string type = arguments.get("--type");

      if (type == "executable" or type == "exe")
      {
        options.type = ccxx::binary_type::EXECUTABLE;
      }
      else if (type == "library" or type == "lib")
      {
        options.type = ccxx::binary_type::LIBRARY;
      }
      else
      {
        std::cerr << ccxx::color::CYAN << "error" << ccxx::color::RESET << ": unknown type \'" << type << "\'.";
        std::exit(-1);
      }
    }

    if (arguments.has("--path"))
    {
      options.project_root = arguments.get("--path");
    }

    if (arguments.has("--style"))
    {
      std::string style = arguments.get("--style");

      if (style == "header-only" or style == "header")
      {
        options.style = ccxx::source_style::HEADER_ONLY;
      }
      else if (style == "module" or style == "modules")
      {
        options.style = ccxx::source_style::MODULE;
      }
      else if (style == "flat")
      {
        options.style = ccxx::source_style::FLAT;
      }
      else if (style == "separate" or style == "seperate")
      {
        options.style = ccxx::source_style::SEPARATE;
      }
      else
      {
        std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET << ": unknown style \'" << style << "\'.";
        std::exit(-1);
      }
    }

    if (arguments.has("--std"))
    {
      options.cxx_std = arguments.get("--std");
    }

    if (arguments.has("--tests"))
    {
      auto tests_val = arguments.get("--tests");
      if (tests_val == "no" || tests_val == "false" || tests_val == "0")
      {
        options.init_tests = false;
      }
    }

    if (arguments.has("--git"))
    {
      options.init_git = true;
    }

    if (arguments.has("--force"))
    {
      options.force = true;
    }

    if (arguments.has("--namespace"))
    {
      options.namespace_name = arguments.get("--namespace");
    }

    return options;
  }
} // namespace

auto main(int argc, char** argv) -> std::int32_t
{
  arguments arguments(argc, argv,
                      {
                          {"-n", "--name"},
                          {"-t", "--type"},
                          {"-p", "--path"},
                          {"-N", "--namespace"},
                          {"-S", "--style"},
                          {"-h", "--help"},
                          {"-v", "--version"},
                          {"-s", "--std"},
                          {"-g", "--git"},
                          {"-f", "--force"},
                          {"-T", "--tests"},
                      });

  if (arguments.has("--help"))
  {
    std::println("Generate a C++ project scaffold with CMake, clangd, and clang-format config.");
    std::println();
    std::println("Usage: ccxx [options] [<project-name>]");
    std::println();
    std::println("{}Options:{}", ccxx::color::CYAN, ccxx::color::RESET);
    std::println("  {:30}{}", "-n, --name <name>", "Project name (also accepts first positional argument)");
    std::println("  {:30}{}", "-t, --type <type>", "Project type: exe/executable, lib/library");
    std::println("  {:30}{}", "-p, --path <dir>", "Output directory (default: current directory)");
    std::println("  {:30}{}", "-s, --std <num>", "C++ standard (20, 23, 26; default: 23)");
    std::println("  {:30}{}", "-N, --namespace <name>", "Namespace for library code (default: project name)");
    std::println("  {:30}{}", "-S, --style <style>",
                 "Source style: flat, separate, module, header-only (default: separate)");
    std::println("  {:30}{}", "-g, --git", "Initialize git repository (branch: main)");
    std::println("  {:30}{}", "-T, --tests", "Generate test infrastructure (default: yes)");
    std::println("  {:30}{}", "-f, --force", "Overwrite existing project directory");
    std::println("  {:30}{}", "-h, --help", "Show this help message");
    std::println("  {:30}{}", "-v, --version", "Show version");
    std::println();
    std::println("{}Interactive Wizard:{} Run ccxx without arguments to launch the interactive project setup wizard.",
                 ccxx::color::CYAN, ccxx::color::RESET);
    std::println();
    std::println("{}Examples:{}", ccxx::color::CYAN, ccxx::color::RESET);
    std::println("  {:47}{}", "ccxx myapp", "Simple executable (c++23)");
    std::println("  {:47}{}", "ccxx -n myapp --type exe", "Explicit executable");
    std::println("  {:47}{}", "ccxx -n myapp --type exe --style module", "Module-based executable");
    std::println("  {:47}{}", "ccxx -n mylib -t lib -p ~/projects", "Library project at custom path");
    std::println("  {:47}{}", "ccxx -n mylib -t lib --style header-only", "Header-only library");
    std::println("  {:47}{}", "ccxx -n myapp --std 20 -g", "C++20 project with git init");
    return 0;
  }

  if (arguments.has("--version"))
  {
    std::println("ccxx version {}", VERSION);
    return 0;
  }

  ccxx::options options = create_options_from_arguments(arguments);
  if (options.project_name.empty())
  {
    options = ccxx::run_wizard();

    if (options.project_name.empty())
    {
      std::cerr << ccxx::color::CYAN << "info" << ccxx::color::RESET << ": cancelled.\n";
      return 0;
    }
  }

  if (options.type == ccxx::binary_type::LIBRARY && options.style == ccxx::source_style::MODULE)
  {
    std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET
              << ": \'module\' style is not supported for library projects.\n";
    return -1;
  }

  if (options.type == ccxx::binary_type::LIBRARY && options.style == ccxx::source_style::FLAT)
  {
    std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET
              << ": \'flat\' style is not supported for library projects.\n";
    return -1;
  }

  if (options.type == ccxx::binary_type::EXECUTABLE && options.style == ccxx::source_style::HEADER_ONLY)
  {
    std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET
              << ": \'header-only\' style is not supported for executable projects.\n";
    return -1;
  }

  if (options.type == ccxx::binary_type::EXECUTABLE && options.style == ccxx::source_style::FLAT && options.init_tests)
  {
    std::cerr << ccxx::color::YELLOW << "warning" << ccxx::color::RESET
              << ": \'flat\' style does not have a library target to test against; tests are skipped.\n";
    options.init_tests = false;
  }

  if (options.namespace_name.empty())
  {
    options.namespace_name = options.project_name;
  }

  fs::path project_root{};

  if (options.project_name == ".")
  {
    options.project_name = fs::current_path().filename().string();
    project_root         = options.project_root;
  }
  else
  {
    project_root = options.project_root / options.project_name;
    if (fs::exists(project_root))
    {
      if (options.force)
      {
        fs::remove_all(project_root);
      }
      else
      {
        std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET << ": project root directory already exists "
                  << project_root << "\n";
        return -1;
      }
    }

    if (!fs::create_directories(project_root))
    {
      std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET << ": failed to create project root directory "
                << project_root << "\n";
      return -1;
    }
  }

  if (options.style == ccxx::source_style::MODULE)
  {
    ccxx::create_module_project(project_root, options);
  }
  else if (options.type == ccxx::binary_type::EXECUTABLE)
  {
    ccxx::create_executable_project(project_root, options);
  }
  else
  {
    ccxx::create_library_project(project_root, options);
  }

  {
    auto to_upper = [](std::string_view s) -> std::string
    {
      std::string result;
      result.reserve(s.size());
      for (auto c : s)
      {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
      }
      return result;
    };

    auto option_prefix = to_upper(options.project_name);

    ccxx::file warnings_file(project_root / "cmake" / "CompilerWarnings.cmake");
    warnings_file.writeln("function(set_project_warnings TARGET_NAME)")
        .writeln("    option(" + option_prefix + "_WARNINGS_AS_ERRORS \"Treat compiler warnings as errors\" OFF)")
        .writeln("")
        .writeln("    if(MSVC)")
        .writeln("        target_compile_options(${TARGET_NAME} PRIVATE")
        .writeln("            /W4")
        .writeln("            /permissive-")
        .writeln("            /utf-8")
        .writeln("        )")
        .writeln("")
        .writeln("        if(" + option_prefix + "_WARNINGS_AS_ERRORS)")
        .writeln("            target_compile_options(${TARGET_NAME} PRIVATE /WX)")
        .writeln("        endif()")
        .writeln("")
        .writeln("    else()")
        .writeln("        target_compile_options(${TARGET_NAME} PRIVATE")
        .writeln("            -Wall")
        .writeln("            -Wextra")
        .writeln("            -Wpedantic")
        .writeln("            -Wconversion")
        .writeln("            -Wsign-conversion")
        .writeln("            -Wshadow")
        .writeln("            -Wnon-virtual-dtor")
        .writeln("            -Wold-style-cast")
        .writeln("            -Wcast-align")
        .writeln("            -Woverloaded-virtual")
        .writeln("            -Wnull-dereference")
        .writeln("            -Wdouble-promotion")
        .writeln("            -Wformat=2")
        .writeln("            -Wimplicit-fallthrough")
        .writeln("            -Wmisleading-indentation")
        .writeln("            -Wno-unknown-pragmas")
        .writeln("        )")
        .writeln("")
        .writeln("        if(CMAKE_CXX_COMPILER_ID STREQUAL \"GNU\")")
        .writeln("            target_compile_options(${TARGET_NAME} PRIVATE")
        .writeln("                -Wduplicated-cond")
        .writeln("                -Wduplicated-branches")
        .writeln("                -Wlogical-op")
        .writeln("                -Wuseless-cast")
        .writeln("            )")
        .writeln("        endif()")
        .writeln("")
        .writeln("        if(CMAKE_CXX_COMPILER_ID MATCHES \"Clang|IntelLLVM\")")
        .writeln("            target_compile_options(${TARGET_NAME} PRIVATE")
        .writeln("                -Wno-c++98-compat")
        .writeln("                -Wno-c++98-compat-pedantic")
        .writeln("            )")
        .writeln("        endif()")
        .writeln("")
        .writeln("        if(" + option_prefix + "_WARNINGS_AS_ERRORS)")
        .writeln("            target_compile_options(${TARGET_NAME} PRIVATE -Werror)")
        .writeln("        endif()")
        .writeln("    endif()")
        .writeln("endfunction()")
        .writeln("");
  }

  {
    ccxx::file clangd_file(project_root / ".clangd");
    clangd_file.writeln("CompileFlags:")
        .writeln("  CompilationDatabase: ./build")
        .writeln("  Add:")
        .writeln("    - -std=c++" + options.cxx_std)
        .writeln("    - -Wall")
        .writeln("    - -Wextra")
        .writeln("    - -Wpedantic")
        .writeln("    - -Wconversion")
        .writeln("    - -Wunused-parameter")
        .writeln("    - -fvisibility=hidden")
        .writeln("    - -march=native")
        .writeln("")
        .writeln("Documentation:")
        .writeln("  CommentFormat: Doxygen")
        .writeln("Diagnostics:")
        .writeln("  UnusedIncludes: Strict")
        .writeln("  ClangTidy:")
        .writeln("    Add:")
        .writeln("      [")
        .writeln("        modernize-*,")
        .writeln("        performance-*,")
        .writeln("        readability-*,")
        .writeln("        bugprone-*,")
        .writeln("        cppcoreguidelines-*,")
        .writeln("        portability-*,")
        .writeln("        misc-*,")
        .writeln("        clang-analyzer-*,")
        .writeln("      ]")
        .writeln("    Remove:")
        .writeln("      [")
        .writeln("        cppcoreguidelines-avoid-const-or-ref-data-members,")
        .writeln("        cppcoreguidelines-avoid-magic-numbers,")
        .writeln("        readability-identifier-length,")
        .writeln("        readability-magic-numbers,")
        .writeln("      ]")
        .writeln("    CheckOptions:")
        .writeln("      GlobalConstantCase: UPPER_CASE")
        .writeln("      readability-identifier-naming.ConstCase: lower_case")
        .writeln("      readability-identifier-naming.ConstantCase: lower_case")
        .writeln("      readability-identifier-naming.GlobalConstantCase: UPPER_CASE")
        .writeln("")
        .writeln("      readability-identifier-naming.VariableCase: lower_case")
        .writeln("      readability-identifier-naming.FunctionCase: lower_case")
        .writeln("      readability-identifier-naming.ParameterCase: lower_case")
        .writeln("")
        .writeln("      readability-identifier-naming.StructCase: lower_case")
        .writeln("      readability-identifier-naming.ClassCase: lower_case")
        .writeln("      readability-identifier-naming.MemberCase: lower_case")
        .writeln("      readability-identifier-naming.PrivateMemberCase: lower_case")
        .writeln("      readability-identifier-naming.PrivateMemberSuffix: _")
        .writeln("")
        .writeln("      readability-identifier-naming.NamespaceCase: lower_case")
        .writeln("      readability-identifier-naming.NamespaceAliasCase: lower_case")
        .writeln("")
        .writeln("      readability-identifier-naming.EnumCase: lower_case")
        .writeln("      readability-identifier-naming.EnumConstantCase: UPPER_CASE")
        .writeln("")
        .writeln("      readability-identifier-naming.MacroDefinitionCase: UPPER_CASE")
        .writeln("      readability-identifier-naming.MacroParameterCase: lower_case")
        .writeln("")
        .writeln("      modernize-use-trailing-return.CheckReturnVoid: true")
        .writeln("      cppcoreguidelines-avoid-magic-numbers.IgnoredValues: \"0,1,-1\"")
        .writeln("")
        .writeln("InlayHints:")
        .writeln("  Enabled: true")
        .writeln("  ParameterNames: true")
        .writeln("  DeducedTypes: true")
        .writeln("  Designators: true")
        .writeln("")
        .writeln("Completion:")
        .writeln("  AllScopes: true")
        .writeln("")
        .writeln("Index:")
        .writeln("  Background: Skip")
        .writeln("  StandardLibrary: true")
        .writeln("  External: None")
        .writeln("")
        .writeln("Style:")
        .writeln("  FullyQualifiedNamespaces: false");
  }

  {
    ccxx::file clang_format_file(project_root / ".clang-format");
    clang_format_file.writeln("BasedOnStyle: LLVM")
        .writeln("IndentWidth: 2")
        .writeln("TabWidth: 2")
        .writeln("Language: Cpp")
        .writeln("AccessModifierOffset: -2")
        .writeln("UseTab: Never")
        .writeln("ColumnLimit: 120")
        .writeln("BreakBeforeBraces: Allman")
        .writeln("AllowShortIfStatementsOnASingleLine: true")
        .writeln("SortIncludes: CaseSensitive")
        .writeln("ConstructorInitializerAllOnOneLineOrOnePerLine: true")
        .writeln("IncludeBlocks: Preserve")
        .writeln("AlwaysBreakTemplateDeclarations: true")
        .writeln("AlwaysBreakAfterDefinitionReturnType: None")
        .writeln("DerivePointerAlignment: false")
        .writeln("BraceWrapping:")
        .writeln("  AfterClass: false")
        .writeln("  AfterControlStatement: false")
        .writeln("  AfterEnum: false")
        .writeln("  AfterFunction: false")
        .writeln("  AfterNamespace: false")
        .writeln("  AfterObjCDeclaration: false")
        .writeln("  AfterStruct: false")
        .writeln("  AfterUnion: false")
        .writeln("  BeforeCatch: false")
        .writeln("  BeforeElse: false")
        .writeln("  IndentBraces: false")
        .writeln("  SplitEmptyFunction: false")
        .writeln("  SplitEmptyNamespace: false")
        .writeln("  SplitEmptyRecord: false")
        .writeln("PointerAlignment: Left")
        .writeln("AllowShortLambdasOnASingleLine: true")
        .writeln("AlignConsecutiveAssignments: true")
        .writeln("AlignTrailingComments: true")
        .writeln("SpaceBeforeAssignmentOperators: true")
        .writeln("SpaceBeforeRangeBasedForLoopColon: true")
        .writeln("SpaceInEmptyBlock: false")
        .writeln("NamespaceIndentation: All")
        .writeln("BreakBeforeBinaryOperators: All")
        .writeln("BreakBeforeTernaryOperators: true")
        .writeln("IndentPPDirectives: AfterHash")
        .writeln("Standard: Latest");
  }

  {
    ccxx::file presets_file(project_root / "CMakePresets.json");
    presets_file.writeln(R"({)")
        .writeln(R"(  "version": 6,)")
        .writeln(R"(  "configurePresets": [)")
        .writeln(R"(    {)")
        .writeln(R"(      "name": "gcc",)")
        .writeln(R"(      "displayName": "GCC",)")
        .writeln(R"(      "description": "Configure with GCC using Ninja",)")
        .writeln(R"(      "generator": "Ninja",)")
        .writeln(R"(      "binaryDir": "${sourceDir}/build",)")
        .writeln(R"(      "cacheVariables": {)")
        .writeln(R"(        "CMAKE_C_COMPILER": "gcc",)")
        .writeln(R"(        "CMAKE_CXX_COMPILER": "g++")")
        .writeln(R"(      })")
        .writeln(R"(    },)")
        .writeln(R"(    {)")
        .writeln(R"(      "name": "clang",)")
        .writeln(R"(      "displayName": "Clang",)")
        .writeln(R"(      "description": "Configure with Clang using Ninja",)")
        .writeln(R"(      "generator": "Ninja",)")
        .writeln(R"(      "binaryDir": "${sourceDir}/build",)")
        .writeln(R"(      "cacheVariables": {)")
        .writeln(R"(        "CMAKE_C_COMPILER": "clang",)")
        .writeln(R"(        "CMAKE_CXX_COMPILER": "clang++")")
        .writeln(R"(      })")
        .writeln(R"(    })")
        .writeln(R"(  ],)")
        .writeln(R"(  "buildPresets": [)")
        .writeln(R"(    {)")
        .writeln(R"(      "name": "gcc",)")
        .writeln(R"(      "configurePreset": "gcc")")
        .writeln(R"(    },)")
        .writeln(R"(    {)")
        .writeln(R"(      "name": "clang",)")
        .writeln(R"(      "configurePreset": "clang")")
        .writeln(R"(    })")
        .writeln(R"(  ])")
        .writeln(R"(})");
  }

  {
    ccxx::file readme_file(project_root / "README.md");
    readme_file.writeln("# Build Instructions")
        .writeln("")
        .writeln("Build with CMake presets (choose your compiler):")
        .writeln("")
        .writeln("```console")
        .writeln("cmake --preset clang")
        .writeln("cmake --build --preset clang")
        .writeln("```")
        .writeln("")
        .writeln("Or without presets:")
        .writeln("")
        .writeln("```console")
        .writeln("cmake -S . -B build")
        .writeln("cmake --build build")
        .writeln("```")
        .writeln("")
        .writeln("Available presets: `clang`, `gcc`")
        .writeln("")
        .writeln("# Dependencies")
        .writeln("")
        .writeln("Add dependencies using git submodules.")
        .writeln("")
        .writeln("```console")
        .writeln("git submodule add <repository_url> <path/to/dependency>")
        .writeln("```")
        .writeln("")
        .writeln("Then include the dependency in your `CMakeLists.txt` with `add_subdirectory` and link it to your "
                 "target with `target_link_libraries`.");
  }

  if (options.init_git)
  {
    {
      ccxx::file gitignore_file(project_root / ".gitignore");
      gitignore_file.writeln(".*").writeln("!.gitignore").writeln("build/");
    }

    std::string command = "git -C \"" + project_root.string() + "\" init -b main";
    if (std::system(command.c_str()) != 0)
    {
      std::cerr << ccxx::color::YELLOW << "warning" << ccxx::color::RESET << ": failed to initialize git repository.\n";
    }
  }
}
