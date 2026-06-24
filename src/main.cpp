#include <chrono>
#include <memory>
#include <string_view>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "rendering/renderer.hpp"
#include "simulation/initialization.hpp"
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

auto initial_n_body_setup {InitializationSetup::PLUMMER_N_BODY};

auto adjust_render_settings(Renderer* renderer, InitializationSetup setup) {
  // Plummer model creates lots of bodies that are more spread out.
  // The frustum size is therefore larger for this setup.
  renderer->frustum_size = setup == InitializationSetup::PLUMMER_N_BODY ? 100.0f : 3.0f;
  renderer->body_radius = 0.1f;
}

auto process_input(Menu* menu, Renderer* renderer, Window* window, const MouseInput& mouse,
                   const KeyboardInput& keyboard) {
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

  auto window {Window {window_width, window_height}};
  auto renderer {Renderer {window_width, window_height}};
  auto simulation {std::make_unique<Simulation>(initialize_bodies(initial_n_body_setup))};

  adjust_render_settings(&renderer, initial_n_body_setup);

  const auto on_n_body_setup_changed {[&](auto setup) {
    simulation.reset(new Simulation {initialize_bodies(setup)});
    adjust_render_settings(&renderer, setup);
  }};

  auto menu {Menu {{.body_radius = &renderer.body_radius, .frustum_size = &renderer.frustum_size},
                   {.n_body_setup = &initial_n_body_setup, .on_n_body_setup_changed = on_n_body_setup_changed}}};

  auto previous_time {std::chrono::steady_clock::now()};
  auto threshold {0.0};

  window.open([&](MouseInput mouse, KeyboardInput keyboard) {
    const auto current_time {std::chrono::steady_clock::now()};
    const auto elapsed_time {std::chrono::round<std::chrono::microseconds>(current_time - previous_time).count() /
                             1000.0};
    previous_time = current_time;
    threshold += elapsed_time;

    window.set_title(std::format("{} ({:.2f}ms)", WINDOW_TITLE, elapsed_time));

    process_input(&menu, &renderer, &window, mouse, keyboard);

    while (threshold >= STEP_UPDATE_INTERVAL_MILLISECONDS) {
      simulation->step();
      threshold -= STEP_UPDATE_INTERVAL_MILLISECONDS;
    }

    renderer.clear();
    renderer.render(simulation->buffer_id(), simulation->bodies_count());

    menu.display();
  });

  return 0;
}
