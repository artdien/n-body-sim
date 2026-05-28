#pragma once

#include <glm/vec3.hpp>

#include "platform/types.hpp"

namespace nbodysim::ui {

class Menu {
public:
  /// Constructs a menu.
  ///
  /// @param body_radius Radius of the rendered bodies.
  /// @param body_color Color of the rendered bodies.
  /// @param visible Flag to denote whether menu is currently visible or not.
  Menu(f32* body_radius, glm::vec3* body_color, bool visible = false);

  Menu(const Menu&) = delete;
  Menu& operator=(const Menu&) = delete;
  Menu(Menu&&) = default;
  Menu& operator=(Menu&&) = default;
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
  f32* body_radius_;
  glm::vec3* body_color_;
  bool visible_;
};

} // namespace nbodysim::ui