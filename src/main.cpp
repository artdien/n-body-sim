#include <memory>
#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "rendering/renderer.hpp"
#include "simulation/configuration.hpp"
#include "simulation/simulation.hpp"
#include "ui/menu.hpp"
#include "utils/cli.hpp"

using namespace nbodysim::platform;
using namespace nbodysim::rendering;
using namespace nbodysim::simulation;
using namespace nbodysim::ui;
using namespace nbodysim::utils;

namespace {

constexpr auto WINDOW_TITLE {std::string_view {"N-Body Simulation"}};

auto process_input(Menu* menu, Window* window, RenderingSettings* settings, const MouseInput& mouse, const KeyboardInput& keyboard) -> void {
  if (keyboard.pressed_key == "m") {
    menu->toggle();
  }
  if (keyboard.pressed_key == "esc") {
    window->close();
  }
  if (mouse.scroll_direction == ScrollDirection::UP) {
    settings->frustum_size -= 0.1f * settings->frustum_size;
    settings->frustum_size = std::max(settings->frustum_size, 1.0f);
  }
  if (mouse.scroll_direction == ScrollDirection::DOWN) {
    settings->frustum_size += 0.1f * settings->frustum_size;
  }
  if (mouse.dragging) {
    settings->frustum_origin += settings->frustum_size * mouse.delta;
  }
}

} // namespace

auto main(int argc, char* argv[]) -> int {
  const auto width {parse_cli_argument(argc, argv, "-width").value_or(1920u)};
  const auto height {parse_cli_argument(argc, argv, "-height").value_or(1080u)};

  auto parameters_cpu {SimulationParametersCPU {}};
  auto parameters_gpu {SimulationParametersGPU {}};
  auto configuration {SimulationConfiguration {.type = SimulationConfigurationType::PLUMMER_N_BODY}};
  auto settings {RenderingSettings {}};
  auto bodies {initialize_bodies(parameters_cpu.G, configuration)};

  auto window {Window {width, height}};
  auto renderer {Renderer {width, height}};
  auto simulation {std::make_unique<Simulation>(std::in_place_type<SimulationCPU>, parameters_cpu, bodies)};
  auto menu {Menu {&simulation, &parameters_cpu, &parameters_gpu, &configuration, &settings}};

  window.open([&](const MouseInput& mouse, const KeyboardInput& keyboard, double elapsed_time) {
    window.set_title(std::format("{} ({:.2f}ms)", WINDOW_TITLE, elapsed_time));

    process_input(&menu, &window, &settings, mouse, keyboard);

    std::visit([](Simulatable auto& s) { s.step(); }, *simulation);

    renderer.clear();
    renderer.render(std::visit([](Simulatable auto& s) { return s.buffer_id(); }, *simulation),
                    std::visit([](Simulatable auto& s) { return s.count(); }, *simulation), settings);

    menu.display();
  });

  return 0;
}
