#include <chrono>
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

auto load_n_body_setup(Simulation* simulation, Renderer* renderer, InitializationSetup setup) {
  simulation->initialize_setup(setup);
  renderer->load_bodies(simulation->bodies());

  // Plummer model creates lots of bodies that are more spread out.
  // The frustum size is therefore larger for this setup.
  renderer->frustum_size = setup == InitializationSetup::PLUMMER_N_BODY ? 100.0f : 3.0f;
  renderer->body_radius = 0.1f;
}

auto process_input(Menu* menu, Renderer* renderer, const MouseInput& mouse, const KeyboardInput& keyboard) {
  if (keyboard.pressed_key == "m") {
    menu->toggle();
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

  Window window {window_width, window_height};
  Simulation simulation {};
  Renderer renderer {window_width, window_height};

  auto initial_n_body_setup {InitializationSetup::PLUMMER_N_BODY};
  const auto on_n_body_setup_changed {[&](auto setup) { load_n_body_setup(&simulation, &renderer, setup); }};
  on_n_body_setup_changed(initial_n_body_setup); // invoke callback here for initial loading

  Menu menu {{.body_radius = &renderer.body_radius, .frustum_size = &renderer.frustum_size},
             {.n_body_setup = &initial_n_body_setup, .on_n_body_setup_changed = on_n_body_setup_changed}};

  auto previous_time {std::chrono::steady_clock::now()};
  auto threshold {0.0};

  window.open([&](MouseInput mouse, KeyboardInput keyboard) {
    const auto current_time {std::chrono::steady_clock::now()};
    const auto elapsed_time {std::chrono::round<std::chrono::microseconds>(current_time - previous_time).count() /
                             1000.0};
    previous_time = current_time;
    threshold += elapsed_time;

    window.set_title(std::format("{} ({:.2f}ms)", WINDOW_TITLE, elapsed_time));

    process_input(&menu, &renderer, mouse, keyboard);

    while (threshold >= STEP_UPDATE_INTERVAL_MILLISECONDS) {
      simulation.step();
      threshold -= STEP_UPDATE_INTERVAL_MILLISECONDS;
    }

    renderer.clear();
    renderer.render();

    menu.display();
  });

  return 0;
}
