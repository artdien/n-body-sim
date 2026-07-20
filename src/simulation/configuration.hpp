#pragma once

#include <vector>

#include "platform/types.hpp"
#include "simulation/body.hpp"

namespace nbodysim::simulation {

enum class SimulationConfigurationType {
  EULER_THREE_BODY,
  LAGRANGE_THREE_BODY,
  PLUMMER_N_BODY,
};

struct SimulationConfiguration {
  /// Type of configuration.
  SimulationConfigurationType type {SimulationConfigurationType::PLUMMER_N_BODY};

  /// Initial distance of bodies from origin.
  /// For Plummer model, this is the Plummer radius.
  f32 radius {100.0f};

  /// Mass of each body in the configuration.
  f32 mass {1.0f};

  /// Number of bodies (only applicable when type is Plummer model).
  u32 count {1000};
};

/// Initializes a specific configuration for simulation.
///
/// The number of initialized bodies depends on the configuration.
///
/// @param G Gravitational constant.
/// @param configuration Configuration for the initialized bodies.
auto initialize_bodies(f32 G, const SimulationConfiguration& configuration) -> std::vector<Body>;

} // namespace nbodysim::simulation
