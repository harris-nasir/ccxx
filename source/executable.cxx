#include "executable.hxx"
#include <print>

namespace ccxx
{
  void create_executable_project(const fs::path& root, const options& opts)
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

    auto option_prefix = to_upper(opts.project_name);

    std::println("creating {}executable{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, opts.project_name,
                 opts.cxx_std, fs::absolute(root).string());

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
      file header_file(root / "source" / opts.project_name / (opts.project_name + ".hxx"));
      header_file.writeln("#pragma once")
          .writeln("")
          .writeln("#include \"defines.hxx\"")
          .writeln("")
          .writeln("namespace " + opts.namespace_name)
          .writeln("{")
          .writeln("  void greet();")
          .writeln("}")
          .writeln("");
    }

    {
      auto use_print = opts.cxx_std >= "23";
      file source_file(root / "source" / opts.project_name / (opts.project_name + ".cxx"));
      source_file.writeln("#include \"" + opts.project_name + ".hxx\"").writeln("");

      if (use_print)
      {
        source_file.writeln("#include <print>");
      }
      else
      {
        source_file.writeln("#include <iostream>");
      }

      source_file.writeln("").writeln("namespace " + opts.namespace_name).writeln("{");

      if (use_print)
      {
        source_file.writeln("  void greet() { std::println(\"Hello, world!\"); }");
      }
      else
      {
        source_file.writeln("  void greet() { std::cout << \"Hello, world!\\n\"; }");
      }

      source_file.writeln("} // namespace " + opts.namespace_name).writeln("");
    }

    {
      file sub_cmake(root / "source" / opts.project_name / "CMakeLists.txt");
      sub_cmake.writeln("set(LIBRARY_NAME " + opts.project_name + ")")
          .writeln("add_library(lib${LIBRARY_NAME} STATIC")
          .writeln("    ${LIBRARY_NAME}.cxx")
          .writeln(")")
          .writeln("target_include_directories(lib${LIBRARY_NAME} PUBLIC")
          .writeln("    ${CMAKE_CURRENT_SOURCE_DIR}/..")
          .writeln(")")
          .writeln("set_target_properties(lib${LIBRARY_NAME} PROPERTIES")
          .writeln("    CXX_STANDARD " + opts.cxx_std)
          .writeln("    CXX_STANDARD_REQUIRED ON")
          .writeln("    CXX_EXTENSIONS OFF")
          .writeln(")")
          .writeln("set_project_warnings(lib${LIBRARY_NAME})")
          .writeln("");
    }

    {
      file main_file(root / "source/main.cxx");
      main_file.writeln("#include \"" + opts.project_name + "/" + opts.project_name + ".hxx\"")
          .writeln("")
          .writeln("auto main() -> i32")
          .writeln("{")
          .writeln("  " + opts.namespace_name + "::greet();")
          .writeln("}");
    }

    {
      file cmake_file(root / "CMakeLists.txt");
      cmake_file.writeln("cmake_minimum_required(VERSION 3.30)")
          .writeln("project(" + opts.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
          .writeln("")
          .writeln("set(CMAKE_CXX_STANDARD " + opts.cxx_std + ")")
          .writeln("set(CMAKE_CXX_STANDARD_REQUIRED ON)")
          .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
          .writeln("")
          .writeln("include(cmake/CompilerWarnings.cmake)")
          .writeln("")
          .writeln("add_subdirectory(source/" + opts.project_name + ")")
          .writeln("");

      if (opts.init_tests)
      {
        cmake_file.writeln("option(" + option_prefix + "_BUILD_TESTS \"Build tests\" ON)").writeln("");
      }

      cmake_file.writeln("add_executable(${PROJECT_NAME} source/main.cxx)")
          .writeln("target_link_libraries(${PROJECT_NAME} PRIVATE lib" + opts.project_name + ")")
          .writeln("set_project_warnings(${PROJECT_NAME})")
          .writeln("");

      if (opts.init_tests)
      {
        cmake_file.writeln("if(" + option_prefix + "_BUILD_TESTS)")
            .writeln("    enable_testing()")
            .writeln("    add_subdirectory(tests)")
            .writeln("endif()")
            .writeln("");
      }

      cmake_file.writeln("");
    }

    if (opts.init_tests)
    {
      {
        file test_cmake(root / "tests" / "CMakeLists.txt");
        test_cmake.writeln("add_executable(" + opts.project_name + "_test")
            .writeln("    " + opts.project_name + "/" + opts.project_name + ".test.cxx")
            .writeln(")")
            .writeln("")
            .writeln("target_link_libraries(" + opts.project_name + "_test PRIVATE lib" + opts.project_name + ")")
            .writeln("")
            .writeln("add_test(NAME " + opts.project_name + "_test COMMAND " + opts.project_name + "_test)")
            .writeln("");
      }

      {
        file test_source(root / "tests" / opts.project_name / (opts.project_name + ".test.cxx"));
        test_source.writeln("#include <" + opts.project_name + "/" + opts.project_name + ".hxx>")
            .writeln("")
            .writeln("auto main() -> int { " + opts.namespace_name + "::greet(); }")
            .writeln("");
      }
    }
  }
} // namespace ccxx
