#pragma once

#include <mutex>
#include <span>
#include <vector>

#include <glm/vec4.hpp>

#include "simulation/body.hpp"
#include "simulation/initialization.hpp"
#include "simulation/thread_pool.hpp"

namespace nbodysim::simulation {

class Simulation {
public:
  /// Creates an N-body simulation.
  ///
  /// @param setup Initial state of bodies for simulation.
  Simulation(const std::vector<Body>& bodies = {});

  Simulation(const Simulation&) = delete;
  Simulation& operator=(const Simulation&) = delete;
  Simulation(Simulation&&) = delete;
  Simulation& operator=(Simulation&&) = delete;
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
  /// Calculates the next step in the simulation for a contiguous partition.
  ///
  /// @param begin Begin of partition (inclusive).
  /// @param end End of partition (exclusive).
  auto step_partition(usize begin, usize end) -> void;

  std::vector<Body> bodies_;
  std::mutex mutex_;
  ThreadPool thread_pool_;
  usize num_threads_;
};

} // namespace nbodysim::simulation
