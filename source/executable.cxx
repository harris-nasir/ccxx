#include "executable.hxx"
#include <print>

namespace ccxx
{
  void create_executable_project(const fs::path& root, const options& opts)
  {
    std::println("{}creating{} {}executable{} \'{}\' (c++{}, {})", color::GREEN, color::RESET, color::GREEN, color::RESET,
                 opts.project_name, opts.cxx_std, fs::absolute(root).string());

    {
      file main_file(root / "source/main.cxx");
      main_file.writeln("#include <iostream>")
          .writeln("")
          .writeln("auto main() -> int")
          .writeln("{")
          .writeln("  std::cout << \"Hello, world!\";")
          .writeln("}");
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
