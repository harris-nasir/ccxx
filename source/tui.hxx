#pragma once

#include "common.hxx"

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <ftxui/component/app.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

namespace ccxx
{
  namespace
  {
    using namespace ftxui;

    constexpr auto ORANGE = Color::Palette256(214);

    struct wizard_state
    {
      int current_step = 0;
      std::string project_name;
      int project_type = 0;
      int source_style = 0;
      int cxx_std      = 1;
      int init_git     = 0;
      int init_tests   = 0;
    };

    constexpr auto STEP_COUNT = std::size_t{7};

    struct step_info
    {
      std::string_view title;
      std::string_view prompt;
    };

    constexpr auto STEPS = std::array<step_info, STEP_COUNT>{{
        {.title = "Project Name", .prompt = "Enter the project name"},
        {.title = "Project Type", .prompt = "Select the project type"},
        {.title = "Source Style", .prompt = "Select the source style"},
        {.title = "C++ Standard", .prompt = "Select the C++ standard"},
        {.title = "Tests", .prompt = "Generate test infrastructure?"},
        {.title = "Git", .prompt = "Initialize a git repository?"},
    }};

    auto step_value(const wizard_state& state, std::size_t index) -> std::string
    {
      switch (index)
      {
      case 0:
        return state.project_name;
      case 1:
        return state.project_type == 0 ? "executable" : "library";
      case 2:
        if (state.project_type == 0)
        {
          return state.source_style == 0 ? "separate" : "module";
        }
        return state.source_style == 0 ? "separate" : "header-only";
      case 3:
        switch (state.cxx_std)
        {
        case 0:
          return "20";
        case 1:
          return "23";
        default:
          return "26";
        }
      case 4:
        return state.init_git == 0 ? "yes" : "no";
      case 5:
        return state.init_tests == 0 ? "yes" : "no";
      default:
        return {};
      }
    }

    auto make_progress_list(const wizard_state& state) -> Element
    {
      Elements items;
      for (std::size_t i = 0; i < STEP_COUNT; ++i)
      {
        Element line;

        if (i < static_cast<std::size_t>(state.current_step))
        {
          auto value = text(step_value(state, i)) | bold | ftxui::color(ORANGE);
          line       = hbox({
              text(" \u2713 ") | ftxui::color(ORANGE),
              text(STEPS.at(i).title),
              text(": ") | dim,
              value,
          });
        }
        else if (i == static_cast<std::size_t>(state.current_step))
        {
          line = hbox({
              text(" \u2192 ") | ftxui::color(ORANGE) | bold,
              text(STEPS.at(i).title) | ftxui::color(ORANGE) | bold,
          });
        }
        else
        {
          line = hbox({
              text("   "),
              text(STEPS.at(i).title) | dim,
          });
        }

        items.push_back(line);
      }
      return vbox(std::move(items));
    }

    auto make_summary(const wizard_state& state) -> Element
    {
      auto item = [&](std::string_view label, const std::string& value) -> Element
      {
        return hbox({
            text("  "),
            text(label) | bold,
            text(": ") | dim,
            text(value) | ftxui::color(ORANGE),
        });
      };

      Elements lines;
      lines.push_back(item("Name", state.project_name));
      lines.push_back(item("Type", state.project_type == 0 ? "executable" : "library"));

      if (state.project_type == 0)
      {
        lines.push_back(item("Style", state.source_style == 0 ? "separate (.cxx, .hxx)" : "module (.cxx, .ixx)"));
      }
      else
      {
        lines.push_back(item("Style", state.source_style == 0 ? "separate (.cxx, .hxx)" : "header-only (.hxx)"));
      }

      switch (state.cxx_std)
      {
      case 0:
        lines.push_back(item("Standard", "C++20"));
        break;
      case 1:
        lines.push_back(item("Standard", "C++23"));
        break;
      default:
        lines.push_back(item("Standard", "C++26"));
        break;
      }
      lines.push_back(item("Git", state.init_git == 0 ? "yes" : "no"));
      lines.push_back(item("Tests", state.init_tests == 0 ? "yes" : "no"));

      return vbox(std::move(lines));
    }

    auto make_footer(const wizard_state& state) -> Element
    {
      auto enter = text("[Enter]") | bold | ftxui::color(ORANGE);
      auto esc   = text("[Esc]") | bold;

      if (state.current_step == 0)
      {
        return hbox({enter, text(" Confirm"), filler(), esc, text(" Cancel")});
      }

      return hbox({enter, text(" Confirm"), text("  \u2022  "), esc, text(" Go back")});
    }

    auto build_result(const wizard_state& state) -> options
    {
      options result{};
      result.project_name = state.project_name;

      if (result.project_name.empty())
      {
        return result;
      }

      result.binary_type = (state.project_type == 0) ? binary_type::EXECUTABLE : binary_type::LIBRARY;

      if (state.project_type == 0)
      {
        result.style = (state.source_style == 0) ? source_style::SEPARATE : source_style::MODULE;
      }
      else
      {
        result.style = (state.source_style == 0) ? source_style::SEPARATE : source_style::HEADER_ONLY;
      }

      switch (state.cxx_std)
      {
      case 0:
        result.cxx_std = "20";
        break;
      case 1:
        result.cxx_std = "23";
        break;
      default:
        result.cxx_std = "26";
        break;
      }

      result.init_tests = (state.init_tests == 0);
      result.init_git   = (state.init_git == 0);
      return result;
    }

  } // namespace

  inline auto run_wizard() -> options
  {
    using namespace ftxui;

    auto screen = App::Fullscreen();
    wizard_state state{};

    auto name_input = Input(&state.project_name, "my_project");

    auto type_entries = std::vector<std::string>{"executable", "library"};
    auto type_toggle  = Toggle(&type_entries, &state.project_type);

    auto exe_style_entries = std::vector<std::string>{"separate (.cxx, .hxx)", "module (.cxx, .ixx)"};
    auto lib_style_entries = std::vector<std::string>{"separate (.cxx, .hxx)", "header-only (.hxx)"};
    auto exe_style_toggle  = Toggle(&exe_style_entries, &state.source_style);
    auto lib_style_toggle  = Toggle(&lib_style_entries, &state.source_style);
    auto style_toggle_tab  = Container::Tab({exe_style_toggle, lib_style_toggle}, &state.project_type);

    auto std_entries = std::vector<std::string>{"20", "23", "26"};
    auto std_toggle  = Toggle(&std_entries, &state.cxx_std);

    auto tests_entries = std::vector<std::string>{"yes", "no"};
    auto tests_toggle  = Toggle(&tests_entries, &state.init_tests);

    auto git_entries = std::vector<std::string>{"yes", "no"};
    auto git_toggle  = Toggle(&git_entries, &state.init_git);

    auto generate_section = Renderer([&] -> Element { return make_summary(state); });

    auto step_tab = Container::Tab(
        {
            name_input,
            type_toggle,
            style_toggle_tab,
            std_toggle,
            tests_toggle,
            git_toggle,
            generate_section,
        },
        &state.current_step);

    auto progress_panel = Renderer([&] -> Element { return make_progress_list(state); });

    auto content_area = Renderer(
        step_tab,
        [&] -> Element
        {
          Element input_area;
          switch (state.current_step)
          {
          case 0:
            input_area = name_input->Render() | size(WIDTH, GREATER_THAN, 35);
            break;
          case 6:
            input_area = generate_section->Render();
            break;
          default:
            input_area = step_tab->Render() | hcenter;
            break;
          }

          auto is_last_step = (state.current_step == static_cast<int>(STEP_COUNT) - 1);

          Elements content;
          if (!is_last_step)
          {
            content.push_back(text(STEPS.at(static_cast<std::size_t>(state.current_step)).prompt) | bold | hcenter);
          }
          content.push_back(input_area);

          return vbox(std::move(content));
        });

    auto footer_panel = Renderer([&] -> Element { return make_footer(state); });

    auto main_container = Container::Vertical({
        content_area,
    });

    main_container |= CatchEvent(
        [&](const Event& event) -> bool
        {
          if (event == Event::Return)
          {
            if (state.current_step == 0 && state.project_name.empty())
            {
              return true;
            }

            if (state.current_step == 1)
            {
              state.source_style = 0;
            }

            if (state.current_step >= static_cast<int>(STEP_COUNT) - 1)
            {
              screen.Exit();
              return true;
            }

            ++state.current_step;
            return true;
          }

          if (event == Event::Escape)
          {
            if (state.current_step > 0)
            {
              --state.current_step;
              return true;
            }

            state.project_name.clear();
            screen.Exit();
            return true;
          }

          return false;
        });

    auto main_renderer
        = Renderer(main_container,
                   [&] -> Element
                   {
                     Elements top;
                     top.push_back(text("CCXX Project Generator") | bold | hcenter | ftxui::color(ORANGE));

                     if (state.current_step < static_cast<int>(STEP_COUNT) - 1)
                     {
                       top.push_back(separator());
                       top.push_back(progress_panel->Render());
                     }

                     top.push_back(separator());
                     top.push_back(content_area->Render() | flex);
                     top.push_back(footer_panel->Render() | hcenter);

                     return vbox(std::move(top)) | border;
                   });

    screen.Loop(main_renderer);

    return build_result(state);
  }

} // namespace ccxx
