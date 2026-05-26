#include "module.hxx"
#include <print>

namespace ccxx
{
  void create_module_project(const fs::path& root, const options& opts)
  {
    std::println("creating module project \'{}\' (c++{}, {})", opts.project_name, opts.cxx_std,
                 fs::absolute(root).string());

    {
      file main_file(root / "source/main.cxx");
      main_file.writeln("import " + opts.project_name + ";")
          .writeln("")
          .writeln("auto main() -> int")
          .writeln("{")
          .writeln("  greet();")
          .writeln("}");
    }

    {
      file module_file(root / "source" / (opts.project_name + ".ixx"));
      module_file.writeln("module;")
          .writeln("#include <cstdio>")
          .writeln("")
          .writeln("export module " + opts.project_name + ";")
          .writeln("")
          .writeln("export void greet()")
          .writeln("{")
          .writeln("  std::puts(\"Hello, world!\");")
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
          .writeln("");
    }
  }
} // namespace ccxx
