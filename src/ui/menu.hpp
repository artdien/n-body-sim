#pragma once

#include <functional>

#include <glm/vec3.hpp>

#include "platform/types.hpp"
#include "simulation/initialization.hpp"

namespace nbodysim::ui {

struct SetupMenu {
  simulation::InitializationSetup* n_body_setup {nullptr};
  std::function<void(simulation::InitializationSetup)> on_n_body_setup_changed {};
};

struct VisualizationMenu {
  f32* body_radius {nullptr};
  f32* frustum_size {nullptr};
};

class Menu {
public:
  /// Constructs a menu.
  ///
  /// @param visualization_menu Fields for menu section "Visualization".
  /// @param setup_menu Fields for menu section "Visualization".
  /// @param visible Flag to denote whether menu is currently visible or not.
  /// @note Fields with default values in menu section structs are not displayed.
  Menu(const VisualizationMenu& visualization_menu, SetupMenu setup_menu, bool visible = false);

  Menu(const Menu&) = delete;
  Menu& operator=(const Menu&) = delete;
  Menu(Menu&&) = delete;
  Menu& operator=(Menu&&) = delete;
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
  VisualizationMenu visualization_menu_;
  SetupMenu setup_menu_;
  bool visible_;
};

} // namespace nbodysim::ui
