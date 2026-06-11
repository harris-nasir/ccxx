#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

namespace ccxx
{
  namespace fs = std::filesystem;

  namespace color
  {
    inline constexpr auto RESET  = "\033[0m";
    inline constexpr auto CYAN   = "\033[36m";
    inline constexpr auto GREEN  = "\033[32m";
    inline constexpr auto YELLOW = "\033[33m";
    inline constexpr auto RED    = "\033[31m";
  } // namespace color

  enum class binary_type : std::uint8_t
  {
    EXECUTABLE,
    LIBRARY
  };

  enum class source_style : std::uint8_t
  {
    SEPARATE,
    HEADER_ONLY,
    MODULE
  };

  struct options
  {
    std::string project_name;
    fs::path project_root{};
    binary_type type{binary_type::EXECUTABLE};
    source_style style{source_style::SEPARATE};
    std::string cxx_std{"23"};
    bool init_tests{true};
    bool init_git{true};
    bool force{false};
    std::string namespace_name;
  };

  class file
  {
  public:
    explicit file(fs::path path) : file_path_(std::move(path))
    {
      fs::create_directories(file_path_.parent_path());
      file_stream_.open(file_path_, std::ios::trunc);
      if (!file_stream_)
      {
        std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET << ": failed to open file " << file_path_
                  << " for writing.\n";
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
        std::cerr << ccxx::color::RED << "error" << ccxx::color::RESET << ": failed to open file " << file_path_
                  << " for reading.\n";
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
} // namespace ccxx
