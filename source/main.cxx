#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <span>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace
{
  class arguments
  {
  public:
    arguments(int argc, char** argv)
    {
      arguments_ = std::span<char*>(argv, static_cast<size_t>(argc));

      for (size_t index = 1; index < arguments_.size(); ++index)
      {
        std::string_view current = arguments_[index];

        if (current.starts_with('-'))
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
      return options_.contains(option);
    }

    [[nodiscard]]
    auto get(std::string_view option) const -> std::string
    {
      auto iterator = options_.find(option);
      if (iterator == options_.end())
      {
        return {};
      }

      return std::string{iterator->second};
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
  };

  enum class binary_type : std::uint8_t
  {
    EXECUTABLE,
    LIBRARY
  };

  struct options
  {

    std::string project_name;
    std::filesystem::path project_root{};
    bool use_headers{false};
    binary_type binary_type{binary_type::EXECUTABLE};
  };

  auto create_options_from_arguments(const arguments& arguments) -> options
  {
    options options{};

    if (arguments.has("--name") or arguments.has("-n"))
    {
      options.project_name = arguments.get("--name");
      if (options.project_name.empty())
      {
        options.project_name = arguments.get("-n");
      }
    }
    else
    {
      const auto& position_arguments = arguments.positional();
      if (!position_arguments.empty())
      {
        options.project_name = position_arguments[0];
      }
    }

    if (arguments.has("--type") or arguments.has("-t"))
    {
      std::string type = arguments.get("--type");
      if (type.empty())
      {
        type = arguments.get("-t");
      }

      if (type == "executable" or type == "exe")
      {
        options.binary_type = binary_type::EXECUTABLE;
      }
      else if (type == "library" or type == "lib")
      {
        options.binary_type = binary_type::LIBRARY;
      }
    }

    if (arguments.has("--path") or arguments.has("-p"))
    {
      options.project_root = arguments.get("--path");
      if (options.project_root.empty())
      {
        options.project_root = arguments.get("-p");
      }
    }

    if (arguments.has("--use-headers") or arguments.has("-h"))
    {
      options.use_headers = true;
    }

    return options;
  }

  namespace fs = std::filesystem;

  class file
  {
  public:
    explicit file(fs::path path) : file_path_(std::move(path))
    {
      fs::create_directories(file_path_.parent_path());
      file_stream_.open(file_path_, std::ios::trunc);
      if (!file_stream_)
      {
        std::cerr << "Failed to open file " << file_path_ << " for writing.\n";
      }
    }

    file(const file&)                    = delete;
    file(file&&)                         = delete;
    auto operator=(const file&) -> file& = delete;
    auto operator=(file&&) -> file&      = delete;

    ~file()
    {
      if (file_stream_.is_open())
      {
        file_stream_.close();
      }
    }

    auto write(const std::string& content) -> file&
    {
      if (file_stream_)
      {
        file_stream_ << content;
      }
      return *this;
    }

    auto writeln(const std::string& content) -> file&
    {
      if (file_stream_)
      {
        file_stream_ << content << "\n";
      }
      return *this;
    }

    auto clear() -> bool
    {
      file_stream_.close();
      file_stream_.open(file_path_, std::ios::trunc);
      return file_stream_.good();
    }

    [[nodiscard]] auto read() const -> std::optional<std::string>
    {
      file_stream_.close();
      std::ifstream read_file(file_path_);
      if (!read_file)
      {
        std::cerr << "Failed to open file " << file_path_ << " for reading.\n";
        return std::nullopt;
      }
      std::string content((std::istreambuf_iterator<char>(read_file)), std::istreambuf_iterator<char>());
      return content;
    }

    [[nodiscard]] auto get_path() const -> fs::path { return file_path_; }

  private:
    fs::path file_path_;
    mutable std::ofstream file_stream_;
  };
} // namespace

auto main(int argc, char** argv) -> std::int32_t
{
  arguments arguments(argc, argv);

  options options = create_options_from_arguments(arguments);
  if (options.project_name.empty())
  {
    std::cerr << "Project name is required. Use -n or --name to specify the project name.\n";
    return -1;
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
      std::cerr << "Project root directory already exists " << project_root << "\n";
      return -1;
    }

    if (!fs::create_directory(project_root))
    {
      std::cerr << "Failed to create project root directory " << project_root << "\n";
      return -1;
    }
  }

  if (options.binary_type == binary_type::EXECUTABLE)
  {
    std::println("Creating executable \'{}\'", options.project_name);
  }
  else if (options.binary_type == binary_type::LIBRARY)
  {
    std::println("Creating library \'{}\'", options.project_name);
  }

  if (!fs::create_directory(project_root / "source"))
  {
    std::cerr << "Failed to create project root directory " << project_root / "source" << "\n";
    return -1;
  }

  if (options.binary_type == binary_type::EXECUTABLE)
  {
    {
      file main_file(project_root / "source/main.cxx");
      main_file.writeln("#include <print>")
          .writeln("")
          .writeln("auto main() -> int")
          .writeln("{")
          .writeln("  std::println(\"Hello, world!\");")
          .writeln("}");
    }

    {
      // FIX: crashes when trying to create cmake file
      file cmake_file(project_root / "CMakeLists.txt");
      cmake_file.writeln("cmake_minimum_required(VERSION 4.0.0)")
          .writeln("project(" + options.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
          .writeln("set(CMAKE_CXX_STANDARD 23)")
          .writeln("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
          .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
          .writeln("")
          .writeln("file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS")
          .writeln("  \"${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx\")")
          .writeln("")
          .writeln("add_executable(${PROJECT_NAME} ${SOURCES})")
          .writeln("");
    }
  }
  else if (options.binary_type == binary_type::LIBRARY)
  {
    {
      file main_file(project_root / "source/main.cxx");
      main_file.writeln("#include \"" + options.project_name + "/" + options.project_name + ".hxx\"")
          .writeln("")
          .writeln("auto main() -> int")
          .writeln("{")
          .writeln("  greet();")
          .writeln("}");
    }

    {
      file header_file(project_root / "source" / options.project_name / (options.project_name + ".hxx"));
      header_file.write("#pragma once").writeln("").writeln("void greet();");

      file source_file(project_root / "source" / options.project_name / (options.project_name + ".cxx"));
      source_file.writeln("#include \"" + options.project_name + ".hxx\"")
          .writeln("#include <print>")
          .writeln("")
          .writeln("void greet()")
          .writeln("{")
          .writeln("  std::println(\"Hello, world!\");")
          .writeln("}");
    }

    {
      file cmake_file(project_root / "CMakeLists.txt");
      cmake_file.writeln("cmake_minimum_required(VERSION 4.0.0)")
          .writeln("project(" + options.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
          .writeln("set(CMAKE_CXX_STANDARD 23)")
          .writeln("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
          .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
          .writeln("")
          .writeln("file(GLOB_RECURSE LIBRARY_SOURCES CONFIGURE_DEPENDS")
          .writeln("  \"${CMAKE_CURRENT_SOURCE_DIR}/source/${PROJECT_NAME}/*.cxx\")")
          .writeln("")
          .writeln("add_library(lib${PROJECT_NAME} ${LIBRARY_SOURCES})")
          .writeln("")
          .writeln("file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS")
          .writeln("  \"${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx\")")
          .writeln("")
          .writeln("add_executable(${PROJECT_NAME} ${SOURCES})")
          .writeln("target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})")
          .writeln("");
    }
  }

  {
    file clangd_file(project_root / ".clangd");
    clangd_file.writeln("CompileFlags:")
        .writeln("  CompilationDatabase: ./build")
        .writeln("  Add:")
        .writeln("    - -std=c++23")
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
        .writeln("        readability-identifier-length,")
        .writeln("        misc-non-private-member-variables-in-classes,")
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
    file clang_format_file(project_root / ".clang-format");
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
    file readme_file(project_root / "README.md");
    readme_file.writeln("# Build Instructions")
        .writeln("")
        .writeln("```console")
        .writeln("cmake -S . -B build")
        .writeln("cmake --build build")
        .writeln("```")
        .writeln("")
        .writeln("# Dependencies")
        .writeln("")
        .writeln("Add dependencies using git submodules.")
        .writeln("")
        .writeln("```console")
        .writeln("git submodule add <repository_url> <path/to/dependency>")
        .writeln("```");
  }
}
