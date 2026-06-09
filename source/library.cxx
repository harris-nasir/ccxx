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

      {
        file cmake_file(root / "CMakeLists.txt");
        cmake_file.writeln("cmake_minimum_required(VERSION 3.30)")
            .writeln("project(" + opts.project_name + " VERSION 0.1.0 LANGUAGES CXX)")
            .writeln("")
            .writeln("set(CMAKE_EXPORT_COMPILE_COMMANDS ON)")
            .writeln("include(GNUInstallDirs)")
            .writeln("")
            .writeln("option(" + option_prefix + "_BUILD_TESTS \"Build tests\" ON)")
            .writeln("option(" + option_prefix + "_INSTALL \"Generate install rules\" ON)")
            .writeln("option(" + option_prefix + "_WARNINGS_AS_ERRORS \"Treat warnings as errors\" OFF)")
            .writeln("")
            .writeln("include(cmake/CompilerWarnings.cmake)")
            .writeln("")
            .writeln("add_subdirectory(" + opts.project_name + ")")
            .writeln("")
            .writeln("if(" + option_prefix + "_INSTALL)")
            .writeln("    install(TARGETS " + opts.project_name)
            .writeln("        EXPORT  " + opts.project_name + "Targets")
            .writeln("        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}")
            .writeln("        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}")
            .writeln("        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}")
            .writeln("        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}")
            .writeln("    )")
            .writeln("")
            .writeln("    install(DIRECTORY " + opts.project_name + "/include/" + opts.namespace_name + "/")
            .writeln("        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/" + opts.namespace_name)
            .writeln("        FILES_MATCHING PATTERN \"*.hxx\"")
            .writeln("    )")
            .writeln("")
            .writeln("    include(CMakePackageConfigHelpers)")
            .writeln("")
            .writeln("    configure_package_config_file(")
            .writeln("        cmake/" + opts.project_name + "Config.cmake.in")
            .writeln("        \"${CMAKE_CURRENT_BINARY_DIR}/" + opts.project_name + "Config.cmake\"")
            .writeln("        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/" + opts.project_name)
            .writeln("    )")
            .writeln("")
            .writeln("    write_basic_package_version_file(")
            .writeln("        \"${CMAKE_CURRENT_BINARY_DIR}/" + opts.project_name + "ConfigVersion.cmake\"")
            .writeln("        VERSION ${PROJECT_VERSION}")
            .writeln("        COMPATIBILITY SameMajorVersion")
            .writeln("    )")
            .writeln("")
            .writeln("    install(FILES")
            .writeln("        \"${CMAKE_CURRENT_BINARY_DIR}/" + opts.project_name + "Config.cmake\"")
            .writeln("        \"${CMAKE_CURRENT_BINARY_DIR}/" + opts.project_name + "ConfigVersion.cmake\"")
            .writeln("        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/" + opts.project_name)
            .writeln("    )")
            .writeln("")
            .writeln("    install(EXPORT " + opts.project_name + "Targets")
            .writeln("        FILE      " + opts.project_name + "Targets.cmake")
            .writeln("        NAMESPACE " + opts.namespace_name + "::")
            .writeln("        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/" + opts.project_name)
            .writeln("    )")
            .writeln("endif()")
            .writeln("")
            .writeln("if(" + option_prefix + "_BUILD_TESTS)")
            .writeln("    enable_testing()")
            .writeln("    add_subdirectory(tests)")
            .writeln("endif()")
            .writeln("");
      }

      {
        file config_file(root / "cmake" / (opts.project_name + "Config.cmake.in"));
        config_file.writeln("@PACKAGE_INIT@")
            .writeln("")
            .writeln("include(\"${CMAKE_CURRENT_LIST_DIR}/" + opts.project_name + "Targets.cmake\")")
            .writeln("")
            .writeln("check_required_components(" + opts.project_name + ")")
            .writeln("");
      }

      {
        file sub_cmake(root / opts.project_name / "CMakeLists.txt");
        sub_cmake.writeln("set(LIBRARY_NAME " + opts.project_name + ")")
            .writeln("add_library(${LIBRARY_NAME} STATIC)")
            .writeln("add_library(${PROJECT_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})")
            .writeln("")
            .writeln("target_sources(${LIBRARY_NAME} PRIVATE")
            .writeln("    " + opts.project_name + ".cxx")
            .writeln(")")
            .writeln("")
            .writeln("target_include_directories(${LIBRARY_NAME} PUBLIC")
            .writeln("    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")
            .writeln("    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
            .writeln(")")
            .writeln("")
            .writeln("set_target_properties(${LIBRARY_NAME} PROPERTIES")
            .writeln("    CXX_STANDARD " + opts.cxx_std)
            .writeln("    CXX_STANDARD_REQUIRED ON")
            .writeln("    CXX_EXTENSIONS OFF")
            .writeln(")")
            .writeln("")
            .writeln("set_project_warnings(${LIBRARY_NAME})")
            .writeln("");
      }

      {
        file source_file(root / opts.project_name / (opts.project_name + ".cxx"));
        source_file.writeln("#include \"" + opts.namespace_name + "/" + opts.project_name + ".hxx\"")
            .writeln("")
            .writeln("#include <print>")
            .writeln("")
            .writeln("namespace " + opts.namespace_name)
            .writeln("{")
            .writeln("  void greet() { std::println(\"Hello, world!\"); }")
            .writeln("} // namespace " + opts.namespace_name)
            .writeln("");
      }

      {
        file header_file(root / opts.project_name / "include" / opts.namespace_name / (opts.project_name + ".hxx"));
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
        file defines_file(root / opts.project_name / "include" / opts.namespace_name / "defines.hxx");
        defines_file.writeln("#pragma once")
            .writeln("")
            .writeln("using u8  = unsigned char;")
            .writeln("using u16 = unsigned short;")
            .writeln("using u32 = unsigned int;")
            .writeln("using u64 = unsigned long long;")
            .writeln("")
            .writeln("using i8  = signed char;")
            .writeln("using i16 = signed short;")
            .writeln("using i32 = signed int;")
            .writeln("using i64 = signed long long;")
            .writeln("")
            .writeln("using f32 = float;")
            .writeln("using f64 = double;")
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
            .writeln("static_assert(sizeof(f64) == 8, \"f64 must be 8 bytes\");")
            .writeln("");
      }

      {
        file test_cmake(root / "tests" / "CMakeLists.txt");
        test_cmake.writeln("add_executable(" + opts.project_name + "_test")
            .writeln("    " + opts.project_name + "/" + opts.project_name + ".test.cxx")
            .writeln(")")
            .writeln("")
            .writeln("target_link_libraries(" + opts.project_name + "_test PRIVATE " + opts.namespace_name
                     + "::" + opts.project_name + ")")
            .writeln("")
            .writeln("add_test(NAME " + opts.project_name + "_test COMMAND " + opts.project_name + "_test)")
            .writeln("");
      }

      {
        file test_source(root / "tests" / opts.project_name / (opts.project_name + ".test.cxx"));
        test_source.writeln("#include <" + opts.namespace_name + "/" + opts.project_name + ".hxx>")
            .writeln("")
            .writeln("auto main() -> int { " + opts.namespace_name + "::greet(); }")
            .writeln("");
      }
    }
  }
} // namespace ccxx
