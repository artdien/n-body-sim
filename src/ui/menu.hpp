#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "rendering/renderer.hpp"
#include "simulation/configuration.hpp"
#include "simulation/simulation.hpp"

namespace nbodysim::ui {

// struct SetupMenu {
//   simulation::ConfigurationType* n_body_setup {nullptr};
//   std::function<void(simulation::Configuration)> on_n_body_setup_changed {};
// };
//
// struct VisualizationMenu {
//   f32* body_radius {nullptr};
//   f32* frustum_size {nullptr};
// };

class Menu {
public:
  /// Constructs a menu.
  ///
  /// @param visualization_menu Fields for menu section "Visualization".
  /// @param setup_menu Fields for menu section "Visualization".
  /// @param visible Flag to denote whether menu is currently visible or not.
  /// @note Fields with default values in menu section structs are not displayed.
  Menu(rendering::Renderer* renderer, std::unique_ptr<simulation::Simulation>* simulation,
       simulation::Configuration* configuration, bool visible = false);

  Menu(const Menu&) = delete;
  Menu(Menu&&) = delete;
  auto operator=(const Menu&) -> Menu& = delete;
  auto operator=(Menu&&) -> Menu& = delete;
  ~Menu() = default;

  /// Displays the menu.
  ///
  /// This method must be called within an existing OpenGL context.
  /// Does not display if current visibility is set to false.
  /// In this case it should be toggled beforehand.
  auto display() -> void;

  /// Toggle the visibility of the menu.
  auto toggle() -> void;

private:
  rendering::Renderer* renderer_;
  std::unique_ptr<simulation::Simulation>* simulation_;
  simulation::Configuration* configuration_;
  bool visible_;
};

} // namespace nbodysim::ui
