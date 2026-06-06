#include "library.hxx"
#include "common.hxx"
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
      std::println("creating {}header-only library{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, opts.project_name,
                   opts.cxx_std, fs::absolute(root).string());
    }
    else
    {
      std::println("creating {}library{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, opts.project_name,
                   opts.cxx_std, fs::absolute(root).string());
    }

    {
      if (opts.style == source_style::HEADER_ONLY)
      {
        file header_file(root / "source" / (opts.project_name + ".hxx"));
        header_file.writeln("#pragma once")
            .writeln("")
            .writeln("using u8 = unsigned char;")
            .writeln("using u16 = unsigned short;")
            .writeln("using u32 = unsigned int;")
            .writeln("using u64 = unsigned long long;")
            .writeln("")
            .writeln("using i8 = signed char;")
            .writeln("using i16 = signed short;")
            .writeln("using i32 = signed int;")
            .writeln("using i64 = signed long long;")
            .writeln("")
            .writeln("using f32 = float;")
            .writeln("using f64 = double;")
            .writeln("")
            .writeln("static_assert(sizeof(u8) == 1, \"u8 must be 1 byte\");")
            .writeln("static_assert(sizeof(u16) == 2, \"u16 must be 2 bytes\");")
            .writeln("static_assert(sizeof(u32) == 4, \"u32 must be 4 bytes\");")
            .writeln("static_assert(sizeof(u64) == 8, \"u64 must be 8 bytes\");")
            .writeln("")
            .writeln("static_assert(sizeof(i8) == 1, \"i8 must be 1 byte\");")
            .writeln("static_assert(sizeof(i16) == 2, \"i16 must be 2 bytes\");")
            .writeln("static_assert(sizeof(i32) == 4, \"i32 must be 4 bytes\");")
            .writeln("static_assert(sizeof(i64) == 8, \"i64 must be 8 bytes\");")
            .writeln("")
            .writeln("static_assert(sizeof(f32) == 4, \"f32 must be 4 bytes\");")
            .writeln("static_assert(sizeof(f64) == 8, \"f64 must be 8 bytes\");")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet();")
            .writeln("}")
            .writeln("")
            .writeln("#ifdef " + macro_name(opts.project_name, opts.namespace_name))
            .writeln("#include <iostream>")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet()")
            .writeln("  {")
            .writeln("    std::cout << \"Hello, world!\";")
            .writeln("  }")
            .writeln("}")
            .writeln("#endif // " + macro_name(opts.project_name, opts.namespace_name));
      }
      else
      {
        file defines(root / "source/defines.hxx");
        defines.writeln("#pragma once")
            .writeln("")
            .writeln("using u8 = unsigned char;")
            .writeln("using u16 = unsigned short;")
            .writeln("using u32 = unsigned int;")
            .writeln("using u64 = unsigned long long;")
            .writeln("")
            .writeln("using i8 = signed char;")
            .writeln("using i16 = signed short;")
            .writeln("using i32 = signed int;")
            .writeln("using i64 = signed long long;")
            .writeln("")
            .writeln("using f32 = float;")
            .writeln("using f64 = double;")
            .writeln("")
            .writeln("static_assert(sizeof(u8) == 1, \"u8 must be 1 byte\");")
            .writeln("static_assert(sizeof(u16) == 2, \"u16 must be 2 bytes\");")
            .writeln("static_assert(sizeof(u32) == 4, \"u32 must be 4 bytes\");")
            .writeln("static_assert(sizeof(u64) == 8, \"u64 must be 8 bytes\");")
            .writeln("")
            .writeln("static_assert(sizeof(i8) == 1, \"i8 must be 1 byte\");")
            .writeln("static_assert(sizeof(i16) == 2, \"i16 must be 2 bytes\");")
            .writeln("static_assert(sizeof(i32) == 4, \"i32 must be 4 bytes\");")
            .writeln("static_assert(sizeof(i64) == 8, \"i64 must be 8 bytes\");")
            .writeln("")
            .writeln("static_assert(sizeof(f32) == 4, \"f32 must be 4 bytes\");")
            .writeln("static_assert(sizeof(f64) == 8, \"f64 must be 8 bytes\");");

        file header_file(root / "source" / opts.namespace_name / (opts.project_name + ".hxx"));
        header_file.writeln("#pragma once")
            .writeln("")
            .writeln("#include \"../defines.hxx\"")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet();")
            .writeln("}");

        file source_file(root / "source" / opts.namespace_name / (opts.project_name + ".cxx"));
        source_file.writeln("#include \"" + opts.project_name + ".hxx\"")
            .writeln("#include <iostream>")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet()")
            .writeln("  {")
            .writeln("    std::cout << \"Hello, world!\";")
            .writeln("  }")
            .writeln("}");
      }
    }

    {
      if (opts.style != source_style::HEADER_ONLY)
      {
        file cmake_file(root / "CMakeLists.txt");
        cmake_file.writeln("cmake_minimum_required(VERSION 4.2.3)")
            .writeln("project(" + opts.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
            .writeln("set(CMAKE_CXX_STANDARD " + opts.cxx_std + ")")
            .writeln("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
            .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
            .writeln("")
            .writeln("file(GLOB_RECURSE LIBRARY_SOURCES CONFIGURE_DEPENDS")
            .writeln("  \"${CMAKE_CURRENT_SOURCE_DIR}/source/" + opts.namespace_name + "/*.cxx\")")
            .writeln("")
            .writeln("add_library(${PROJECT_NAME} STATIC ${LIBRARY_SOURCES})")
            .writeln("");
      }
    }
  }
} // namespace ccxx
