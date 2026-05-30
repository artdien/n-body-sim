#pragma once

#include <vector>

#include "simulation/body.hpp"

namespace nbodysim::simulation {

enum class InitializationSetup {
  EULER_THREE_BODY,
  LAGRANGE_THREE_BODY,
  PLUMMER_N_BODY,
};

/// Initializes a number of bodies for simulation.
///
/// @param setup Setup for the initialized bodies.
/// @note The number of initialized bodies depends on the setup.
std::vector<Body> initialize_bodies(InitializationSetup setup);

} // namespace nbodysim::simulation