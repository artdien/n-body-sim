#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "platform/input.hpp"
#include "platform/window.hpp"
#include "rendering/renderer.hpp"
#include "simulation/initialization.hpp"
#include "simulation/simulation.hpp"
#include "ui/menu.hpp"

using namespace nbodysim::platform;
using namespace nbodysim::rendering;
using namespace nbodysim::simulation;
using namespace nbodysim::ui;

namespace {

auto load_n_body_setup(Simulation* simulation, Renderer* renderer, InitializationSetup setup) {
  simulation->initialize_setup(setup);
  renderer->load_bodies(simulation->bodies());

  // Plummer model creates lots of bodies that are more spread out.
  // The frustum size is therefore larger for this setup.
  renderer->frustum_size = setup == InitializationSetup::PLUMMER_N_BODY ? 20.0f : 3.0f;
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

int main() {
  constexpr auto window_width = 1920u;
  constexpr auto window_height = 1080u;

  Window window {"N-Body Simulation", window_width, window_height};
  Simulation simulation {};
  Renderer renderer {window_width, window_height};

  auto initial_n_body_setup {InitializationSetup::PLUMMER_N_BODY};
  const auto on_n_body_setup_changed {[&](auto setup) { load_n_body_setup(&simulation, &renderer, setup); }};
  on_n_body_setup_changed(initial_n_body_setup); // invoke callback here for initial loading

  Menu menu {{.body_radius = &renderer.body_radius, .frustum_size = &renderer.frustum_size},
             {.n_body_setup = &initial_n_body_setup, .on_n_body_setup_changed = on_n_body_setup_changed}};

  window.open([&](MouseInput mouse, KeyboardInput keyboard) {
    process_input(&menu, &renderer, mouse, keyboard);

    simulation.step();

    renderer.clear();
    renderer.render();

    menu.display();
  });

  return 0;
}
