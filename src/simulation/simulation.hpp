#pragma once

#include <concepts>
#include <span>
#include <vector>

#include <glad/glad.h>

#include "platform/types.hpp"
#include "simulation/body.hpp"
#include "simulation/octree.hpp"
#include "simulation/thread_pool.hpp"

namespace nbodysim::simulation {

struct SimulationParameters {
  /// Gravitational constant.
  f32 G {1.0f};

  /// Size of time step per simulation step.
  f32 dt {0.01f};

  /// Softening factor to avoid direct collision between bodies.
  f32 eps {0.01f};
};

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

struct SimulationParametersCPU {
  /// General simulation parameters.
  SimulationParameters general;

  /// Flag whether to use the Barnes-Hut algorithm for simulation.
  bool use_barnes_hut {true};

  /// Threshold when to apply Barnes-Hut appoximation.
  f32 theta {0.5f};

  /// Number of threads to use for simulation.
  u32 thread_count {std::max(1u, std::thread::hardware_concurrency() - 1)};
};

class SimulationCPU {
public:
  /// Creates an N-body simulation.
  ///
  /// @param parameters Parameters for simulation.
  /// @param bodies Initial state of bodies for simulation.
  SimulationCPU(const SimulationParametersCPU& parameters, const std::vector<Body>& bodies);

  SimulationCPU(const SimulationCPU&) = delete;
  SimulationCPU(SimulationCPU&&) = delete;
  auto operator=(const SimulationCPU&) -> SimulationCPU& = delete;
  auto operator=(SimulationCPU&&) -> SimulationCPU& = delete;
  ~SimulationCPU();

  auto step() -> void;
  auto buffer_id() const -> GLuint;
  auto bodies_count() const -> usize;

private:
  SimulationParametersCPU parameters_;
  std::vector<Body> bodies_;

  GLuint buffer_id_;
  std::span<Body> buffer_;

  usize root_node_idx_;
  OctreeNodePool node_pool_;

  ThreadPool thread_pool_;

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

struct SimulationParametersGPU {
  /// General simulation parameters.
  SimulationParameters general;

  /// Number of work groups to dispatch on GPU for simulation.
  /// Each work group calculates the simulation step in parallel for a number of bodies.
  u32 dispatch_size {8u};

  /// Tile size determines for how many bodies the simulation step is calculated in parallel within a work group.
  /// If dispatch_size times tile_size is less than the number of bodies in the simulation,
  /// the remaining bodies will be distributed across the work groups.
  /// In this case some or all work groups will calculate more bodies than specified via the tile size.
  u32 tile_size {128u};
};

class SimulationGPU {
public:
  /// Creates an N-body simulation.
  ///
  /// @param parameters Parameters for simulation.
  /// @param bodies Initial state of bodies for simulation.
  SimulationGPU(const SimulationParametersGPU& parameters, const std::vector<Body>& bodies);

  SimulationGPU(const SimulationGPU&) = delete;
  SimulationGPU(SimulationGPU&&) = delete;
  auto operator=(const SimulationGPU&) -> SimulationGPU& = delete;
  auto operator=(SimulationGPU&&) -> SimulationGPU& = delete;
  ~SimulationGPU();

  auto step() -> void;
  auto buffer_id() const -> GLuint;
  auto bodies_count() const -> usize;

private:
  SimulationParametersGPU parameters_;

  GLuint buffer_id_;
  std::span<Body> buffer_;

  GLuint update_position_program_id_;
  GLuint update_acceleration_program_id_;

  usize bodies_count_;
};

static_assert(Simulatable<SimulationCPU>);
static_assert(Simulatable<SimulationGPU>);

using Simulation = std::variant<SimulationCPU, SimulationGPU>;

} // namespace nbodysim::simulation
