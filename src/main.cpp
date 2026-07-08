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
constexpr auto STEP_UPDATE_INTERVAL_MILLISECONDS {1000.0 / 60.0};

auto process_input(Menu* menu, Renderer* renderer, Window* window, const MouseInput& mouse,
                   const KeyboardInput& keyboard) -> void {
  if (keyboard.pressed_key == "m") {
    menu->toggle();
  }
  if (keyboard.pressed_key == "esc") {
    window->close();
  }
  if (mouse.scroll_direction == ScrollDirection::UP) {
    renderer->frustum_size -= 1.0f;
  }
  if (mouse.scroll_direction == ScrollDirection::DOWN) {
    renderer->frustum_size += 1.0f;
  }
}

} // namespace

int main(int argc, char* argv[]) {
  const auto window_width {parse_cli_argument(argc, argv, "-width").value_or(1920u)};
  const auto window_height {parse_cli_argument(argc, argv, "-height").value_or(1080u)};

  auto parameters_cpu {SimulationParametersCPU {}};
  auto parameters_gpu {SimulationParametersGPU {}};
  auto configuration {Configuration {.type = ConfigurationType::PLUMMER_N_BODY, .count = 1000}};
  auto bodies {initialize_configuration(parameters_gpu.general.G, configuration)};

  auto window {Window {window_width, window_height}};
  auto renderer {Renderer {window_width, window_height}};
  auto simulation {std::make_unique<Simulation>(std::in_place_type<SimulationGPU>, parameters_gpu, bodies)};
  auto menu {Menu {&renderer, &simulation, &parameters_cpu, &parameters_gpu, &configuration}};

  renderer.frustum_size = 100.0f;
  renderer.body_radius = 0.1f;

  auto threshold {0.0};

  window.open([&](MouseInput mouse, KeyboardInput keyboard, double elapsed_time) {
    threshold += elapsed_time;

    window.set_title(std::format("{} ({:.2f}ms)", WINDOW_TITLE, elapsed_time));

    process_input(&menu, &renderer, &window, mouse, keyboard);

    while (threshold >= STEP_UPDATE_INTERVAL_MILLISECONDS) {
      std::visit([](Simulatable auto& s) { s.step(); }, *simulation);
      threshold -= STEP_UPDATE_INTERVAL_MILLISECONDS;
    }

    renderer.clear();
    renderer.render(std::visit([](Simulatable auto& s) { return s.buffer_id(); }, *simulation),
                    std::visit([](Simulatable auto& s) { return s.bodies_count(); }, *simulation));

    menu.display();
  });

  return 0;
}
