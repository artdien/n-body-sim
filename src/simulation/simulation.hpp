#pragma once

#include <span>
#include <vector>

#include <glad/glad.h>

#include "simulation/body.hpp"
#include "simulation/octree.hpp"
#include "simulation/thread_pool.hpp"

namespace nbodysim::simulation {

class Simulation {
public:
  /// Creates an N-body simulation.
  ///
  /// @param setup Initial state of bodies for simulation.
  Simulation(const std::vector<Body>& bodies);

  Simulation(const Simulation&) = delete;
  Simulation(Simulation&&) = delete;
  auto operator=(const Simulation&) -> Simulation& = delete;
  auto operator=(Simulation&&) -> Simulation& = delete;
  ~Simulation();

  /// Calculates the next step in the simulation.
  auto step() -> void;

  /// Returns the ID for the SSBO containing the current state of bodies.
  ///
  /// @return ID for SSBO.
  auto buffer_id() const -> GLuint;

  /// Returns the number of bodies in the current simulation.
  ///
  /// @return Number of bodies.
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

} // namespace nbodysim::simulation
