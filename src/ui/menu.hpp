#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "rendering/renderer.hpp"
#include "simulation/configuration.hpp"
#include "simulation/simulation.hpp"

namespace nbodysim::ui {

class Menu {
public:
  /// Constructs a menu.
  ///
  /// All parameters except the visible flag can be directly changed via the menu,
  /// hence they are passed as pointers.
  ///
  /// @param visible Flag to denote whether menu is currently visible or not.
  Menu(std::unique_ptr<simulation::Simulation>* simulation, simulation::SimulationParametersCPU* parameters_cpu,
       simulation::SimulationParametersGPU* parameters_gpu, simulation::SimulationConfiguration* configuration,
       rendering::RenderingSettings* settings, bool visible = false);

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
  std::unique_ptr<simulation::Simulation>* simulation_;
  simulation::SimulationParametersCPU* parameters_cpu_;
  simulation::SimulationParametersGPU* parameters_gpu_;
  simulation::SimulationConfiguration* configuration_;
  rendering::RenderingSettings* settings_;
  bool visible_;
};

} // namespace nbodysim::ui
