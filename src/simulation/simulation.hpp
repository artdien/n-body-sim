#pragma once

#include <span>
#include <vector>

#include <glm/vec4.hpp>

#include "platform/types.hpp"
#include "simulation/body.hpp"

namespace nbodysim::simulation {

class Simulation {
public:
  /// Creates an N-body simulation.
  ///
  /// @param N Number of bodies this simulation should contain.
  Simulation(u32 N);

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