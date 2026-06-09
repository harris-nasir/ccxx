#include "module.hxx"
#include "common.hxx"
#include <print>

namespace ccxx
{
  void create_module_project(const fs::path& root, const options& opts)
  {
    std::println("creating {}module{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, opts.project_name, opts.cxx_std,
                 fs::absolute(root).string());

    {
      file main_file(root / "source/main.cxx");
      main_file.writeln("import " + opts.project_name + ";")
          .writeln("")
          .writeln("auto main() -> i32")
          .writeln("{")
          .writeln("  greet();")
          .writeln("}");
    }

    {
      file defines_file(root / "source/defines.ixx");
      defines_file.writeln("module;")
          .writeln("")
          .writeln("export module defines;")
          .writeln("")
          .writeln("export using u8  = unsigned char;")
          .writeln("export using u16 = unsigned short;")
          .writeln("export using u32 = unsigned int;")
          .writeln("export using u64 = unsigned long long;")
          .writeln("")
          .writeln("export using i8  = signed char;")
          .writeln("export using i16 = signed short;")
          .writeln("export using i32 = signed int;")
          .writeln("export using i64 = signed long long;")
          .writeln("")
          .writeln("export using f32 = float;")
          .writeln("export using f64 = double;")
          .writeln("")
          .writeln("static_assert(sizeof(u8)  == 1, \"u8 must be 1 byte\");")
          .writeln("static_assert(sizeof(u16) == 2, \"u16 must be 2 bytes\");")
          .writeln("static_assert(sizeof(u32) == 4, \"u32 must be 4 bytes\");")
          .writeln("static_assert(sizeof(u64) == 8, \"u64 must be 8 bytes\");")
          .writeln("")
          .writeln("static_assert(sizeof(i8)  == 1, \"i8 must be 1 byte\");")
          .writeln("static_assert(sizeof(i16) == 2, \"i16 must be 2 bytes\");")
          .writeln("static_assert(sizeof(i32) == 4, \"i32 must be 4 bytes\");")
          .writeln("static_assert(sizeof(i64) == 8, \"i64 must be 8 bytes\");")
          .writeln("")
          .writeln("static_assert(sizeof(f32) == 4, \"f32 must be 4 bytes\");")
          .writeln("static_assert(sizeof(f64) == 8, \"f64 must be 8 bytes\");");
    }

    {
      file module_file(root / "source" / (opts.project_name + ".ixx"));
      module_file.writeln("module;").writeln("");
      if (opts.cxx_std >= "23")
      {
        module_file.writeln("#include <print>")
            .writeln("")
            .writeln("export module " + opts.project_name + ";")
            .writeln("export import defines;")
            .writeln("")
            .writeln("export void greet()")
            .writeln("{")
            .writeln("  std::print(\"Hello, world!\");")
            .writeln("}");
      }
      else
      {
        module_file.writeln("#include <iostream>")
            .writeln("")
            .writeln("export module " + opts.project_name + ";")
            .writeln("export import defines;")
            .writeln("")
            .writeln("export void greet()")
            .writeln("{")
            .writeln("  std::cout << \"Hello, world!\";")
            .writeln("}");
      }
    }

    {
      file cmake_file(root / "CMakeLists.txt");
      cmake_file.writeln("cmake_minimum_required(VERSION 4.3.0)")
          .writeln("project(" + opts.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
          .writeln("")
          .writeln("set(CMAKE_CXX_STANDARD " + opts.cxx_std + ")")
          .writeln("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
          .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
          .writeln("")
          .writeln("include(cmake/CompilerWarnings.cmake)")
          .writeln("")
          .writeln("file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS \"source/*.cxx\")")
          .writeln("file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS \"source/*.ixx\")")
          .writeln("")
          .writeln("add_executable(${PROJECT_NAME})")
          .writeln("")
          .writeln("target_sources(${PROJECT_NAME}")
          .writeln("  PRIVATE")
          .writeln("  ${SOURCES}")
          .writeln("")
          .writeln("  PRIVATE")
          .writeln("  FILE_SET CXX_MODULES")
          .writeln("  FILES ${MODULE_SOURCES}")
          .writeln(")")
          .writeln("")
          .writeln("set_project_warnings(${PROJECT_NAME})")
          .writeln("");
    }
  }
} // namespace ccxx
