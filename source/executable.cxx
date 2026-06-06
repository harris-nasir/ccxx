#include "executable.hxx"
#include <print>

namespace ccxx
{
  void create_executable_project(const fs::path& root, const options& opts)
  {
    std::println("creating {}executable{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, opts.project_name,
                 opts.cxx_std, fs::absolute(root).string());

    {
      file main_file(root / "source/main.cxx");
      main_file.writeln("#include \"defines.hxx\"");
      if (opts.cxx_std >= "23")
      {
        main_file.writeln("#include <print>")
            .writeln("")
            .writeln("auto main() -> i32")
            .writeln("{")
            .writeln("  std::print(\"Hello, world!\");")
            .writeln("}");
      }
      else
      {
        main_file.writeln("#include <iostream>")
            .writeln("")
            .writeln("auto main() -> i32")
            .writeln("{")
            .writeln("  std::cout << \"Hello, world!\";")
            .writeln("}");
      }
    }

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
    }

    {
      file cmake_file(root / "CMakeLists.txt");
      cmake_file.writeln("cmake_minimum_required(VERSION 4.2.3)")
          .writeln("project(" + opts.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
          .writeln("set(CMAKE_CXX_STANDARD " + opts.cxx_std + ")")
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
} // namespace ccxx
