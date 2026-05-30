#pragma once

#include <span>
#include <vector>

#include <glm/vec4.hpp>

#include "simulation/body.hpp"
#include "simulation/initialization.hpp"

namespace nbodysim::simulation {

class Simulation {
public:
  /// Creates an N-body simulation.
  ///
  /// @param setup Initial setup of bodies for the simulation.
  /// @note The number of initialized bodies depends on the setup.
  Simulation(InitializationSetup setup);

  Simulation(const Simulation&) = delete;
  Simulation& operator=(const Simulation&) = delete;
  Simulation(Simulation&&) = default;
  Simulation& operator=(Simulation&&) = default;
  ~Simulation() = default;

  /// Calculates the next step in the simulation.
  ///
  /// The calculation depends on the chosen integrator.
  auto step() -> void;

  /// Gets a constant view of the current state of all bodies.
  auto bodies() const -> std::span<const Body>;

private:
  std::vector<Body> bodies_;
};

} // namespace nbodysim::simulation