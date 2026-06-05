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
  /// @param setup Initial state of bodies for simulation.
  Simulation(const std::vector<Body>& bodies = {});

  Simulation(const Simulation&) = delete;
  Simulation& operator=(const Simulation&) = delete;
  Simulation(Simulation&&) = default;
  Simulation& operator=(Simulation&&) = default;
  ~Simulation() = default;

  /// Calculates the next step in the simulation.
  ///
  /// The calculation depends on the chosen integrator.
  auto step() -> void;

  /// Initializes simulation with a pre-defined setup.
  ///
  /// This method discards the current simulation state
  /// and resets simulation parameters.
  ///
  /// @param setup Setup for initialization.
  /// @note The number of initialized bodies depends on the setup.
  auto initialize_setup(InitializationSetup setup) -> void;

  /// Gets a constant view of the current state of all bodies.
  ///
  /// @return Constant view of current simulation state.
  auto bodies() const -> std::span<const Body>;

private:
  std::vector<Body> bodies_;
};

} // namespace nbodysim::simulation