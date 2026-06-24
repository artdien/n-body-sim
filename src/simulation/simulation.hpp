#pragma once

#include <concepts>
#include <span>
#include <vector>

#include <glad/glad.h>

#include "simulation/body.hpp"
#include "simulation/octree.hpp"
#include "simulation/thread_pool.hpp"

namespace nbodysim::simulation {

template <typename S>
concept Simulatable = requires(S s, const S cs) {
  /// Calculates the next step in the simulation.
  { s.step() } -> std::same_as<void>;

  /// Returns the ID for the SSBO containing the current state of bodies.
  ///
  /// @return ID for SSBO.
  { cs.buffer_id() } -> std::same_as<GLuint>;

  /// Returns the number of bodies in the current simulation.
  ///
  /// @return Number of bodies.
  { cs.bodies_count() } -> std::same_as<usize>;
};

class SimulationCPU {
public:
  /// Creates an N-body simulation.
  ///
  /// @param setup Initial state of bodies for simulation.
  SimulationCPU(const std::vector<Body>& bodies);

  SimulationCPU(const SimulationCPU&) = delete;
  SimulationCPU(SimulationCPU&&) = delete;
  auto operator=(const SimulationCPU&) -> SimulationCPU& = delete;
  auto operator=(SimulationCPU&&) -> SimulationCPU& = delete;
  ~SimulationCPU();

  auto step() -> void;
  auto buffer_id() const -> GLuint;
  auto bodies_count() const -> usize;

private:
  std::vector<Body> bodies_;

  GLuint buffer_id_;
  std::span<Body> buffer_;

  usize root_node_idx_;
  OctreeNodePool node_pool_;

  ThreadPool thread_pool_;
  usize num_threads_;

  auto construct_barnes_hut_tree() -> void;
  auto insert_octree_node(usize node_idx, usize body_idx) -> void;
  auto insert_octree_child_node(usize node_idx, usize body_idx) -> void;
  auto node(usize node_idx) -> OctreeNode& {
    return node_pool_[node_idx];
  }

  auto step_partition(usize begin, usize end) -> void;
  auto calculate_acceleration_all_pairs(usize body_idx) -> void;
  auto calculate_acceleration_barnes_hut(usize node_idx, usize body_idx, f32 theta) -> void;
  auto body(usize body_idx) -> Body& {
    return bodies_[body_idx];
  }
};

static_assert(Simulatable<SimulationCPU>);

using Simulation = std::variant<SimulationCPU>;

} // namespace nbodysim::simulation
