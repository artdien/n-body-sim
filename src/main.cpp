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

int main() {
  constexpr auto window_width = 1920u;
  constexpr auto window_height = 1080u;

  Window window {"N-Body Simulation", window_width, window_height};
  Simulation simulation {InitializationSetup::PLUMMER_N_BODY};
  Renderer renderer {window_width, window_height, simulation.bodies()};
  Menu menu {&renderer.body_radius, &renderer.body_color};

  renderer.body_radius = 0.1f;

  const auto process_input {[&](MouseInput mouse, KeyboardInput keyboard) {
    if (keyboard.pressed_key == "m") {
      menu.toggle();
    }
    if (mouse.scroll_direction == ScrollDirection::UP) {
      renderer.adjust_frustum_size(-1.0f);
    }
    if (mouse.scroll_direction == ScrollDirection::DOWN) {
      renderer.adjust_frustum_size(1.0f);
    }
  }};

  window.open([&](MouseInput mouse, KeyboardInput keyboard) {
    process_input(mouse, keyboard);

    simulation.step();

    renderer.clear();
    renderer.render();

    menu.display();
  });

  return 0;
}