#include "library.hxx"
#include <cctype>
#include <print>

namespace ccxx
{
  void create_library_project(const fs::path& root, const options& opts)
  {
    auto macro_name = [](std::string_view project, std::string_view ns) -> std::string
    {
      std::string result;
      result.reserve(project.size() + 1 + ns.size() + 15);
      for (auto c : project)
      {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
      }
      result += '_';
      for (auto c : ns)
      {
        result.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
      }
      result += "_IMPLEMENTATION";
      return result;
    };

    if (opts.style == source_style::HEADER_ONLY)
    {
      std::println("{}creating{} {}header-only library{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, color::YELLOW, color::RESET,
                   opts.project_name, opts.cxx_std, fs::absolute(root).string());
    }
    else
    {
      std::println("{}creating{} {}library{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, color::CYAN, color::RESET,
                   opts.project_name, opts.cxx_std, fs::absolute(root).string());
    }

    {
      if (opts.style == source_style::HEADER_ONLY)
      {
        file header_file(root / "source" / opts.namespace_name / (opts.project_name + ".hxx"));
        header_file.writeln("#pragma once")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet();")
            .writeln("}")
            .writeln("")
            .writeln("#ifdef " + macro_name(opts.project_name, opts.namespace_name))
            .writeln("#include <cstdio>")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet()")
            .writeln("  {")
            .writeln("    std::puts(\"Hello, world!\");")
            .writeln("  }")
            .writeln("}")
            .writeln("#endif // " + macro_name(opts.project_name, opts.namespace_name));
      }
      else
      {
        file header_file(root / "source" / opts.namespace_name / (opts.project_name + ".hxx"));
        header_file.writeln("#pragma once")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet();")
            .writeln("}");

        file source_file(root / "source" / opts.namespace_name / (opts.project_name + ".cxx"));
        source_file.writeln("#include \"" + opts.project_name + ".hxx\"")
            .writeln("#include <cstdio>")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet()")
            .writeln("  {")
            .writeln("    std::puts(\"Hello, world!\");")
            .writeln("  }")
            .writeln("}");
      }
    }

    {
      file cmake_file(root / "CMakeLists.txt");
      cmake_file.writeln("cmake_minimum_required(VERSION 4.2.3)")
          .writeln("project(" + opts.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
          .writeln("set(CMAKE_CXX_STANDARD " + opts.cxx_std + ")")
          .writeln("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
          .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
          .writeln("");

      if (opts.style == source_style::HEADER_ONLY)
      {
        cmake_file.writeln("add_library(${PROJECT_NAME} INTERFACE)")
            .writeln("target_include_directories(${PROJECT_NAME} INTERFACE source)")
            .writeln("");
      }
      else
      {
        cmake_file.writeln("file(GLOB_RECURSE LIBRARY_SOURCES CONFIGURE_DEPENDS")
            .writeln("  \"${CMAKE_CURRENT_SOURCE_DIR}/source/" + opts.namespace_name + "/*.cxx\")")
            .writeln("")
            .writeln("add_library(${PROJECT_NAME} STATIC ${LIBRARY_SOURCES})")
            .writeln("");
      }
    }
  }
} // namespace ccxx
