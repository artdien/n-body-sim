#include "simulation/simulation.hpp"

#include <algorithm>
#include <ranges>
#include <tuple>

#include <glm/geometric.hpp>

#include "platform/types.hpp"
#include "simulation/octree.hpp"

namespace nbodysim::simulation {

namespace {

auto determine_initial_center_and_inradius(std::span<const Body> bodies) -> std::tuple<glm::vec3, f32> {
  auto inradius {0.0f};
  auto center {glm::vec3 {0.0f, 0.0f, 0.0f}};
  const auto weight {1.0f / bodies.size()};

  std::ranges::for_each(bodies, [&](auto& body) {
    center += weight * body.position;
    const auto difference {glm::abs(center - body.position)};
    inradius = std::max({difference.x, difference.y, difference.z, inradius});
  });

  // Increase inradius slightly to account for numerical errors
  inradius *= 1.05f;

  return {center, inradius};
}

auto determine_node_center_and_inradius(usize octant, const glm::vec3& center, f32 inradius)
    -> std::tuple<glm::vec3, f32> {
  const auto child_inradius {0.5f * inradius};
  const auto child_center = glm::vec3 {
      octant & 1uz ? center.x + 0.5f * child_inradius : center.x - 0.5f * child_inradius,
      octant & 2uz ? center.y + 0.5f * child_inradius : center.y - 0.5f * child_inradius,
      octant & 4uz ? center.z + 0.5f * child_inradius : center.z - 0.5f * child_inradius,
  };

  return {child_center, child_inradius};
}

auto determine_octant(const glm::vec3& center, const glm::vec3& position) -> usize {
  auto octant {0uz};

  octant |= position.x >= center.x ? 1uz : 0uz;
  octant |= position.y >= center.y ? 2uz : 0uz;
  octant |= position.z >= center.z ? 4uz : 0uz;

  return octant;
}

} // namespace

SimulationCPU::SimulationCPU(const SimulationParametersCPU& parameters, const std::vector<Body>& bodies)
    : parameters_ {parameters}, bodies_ {bodies}, thread_pool_ {parameters.thread_count} {
  const auto flags {GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT};

  glCreateBuffers(1, &buffer_id_);
  glNamedBufferStorage(buffer_id_, bodies_.size() * sizeof(Body), nullptr, flags);

  const auto buffer {glMapNamedBufferRange(buffer_id_, 0, bodies_.size() * sizeof(Body), flags)};
  buffer_ = std::span {reinterpret_cast<Body*>(buffer), bodies_.size()};
}

SimulationCPU::~SimulationCPU() {
  glDeleteBuffers(1, &buffer_id_);
}

auto SimulationCPU::step() -> void {
  const auto bodies_per_thread {bodies_.size() / parameters_.thread_count};
  const auto remainder {bodies_.size() % parameters_.thread_count};

  if (parameters_.use_barnes_hut) {
    construct_barnes_hut_tree();
  }

  auto partition_begin {0uz};
  auto partition_end {0uz};

  std::ranges::for_each(std::views::iota(0uz, parameters_.thread_count), [&, this](auto i) {
    partition_end += bodies_per_thread + (i < remainder ? 1uz : 0uz);
    thread_pool_.schedule([=, this] { step_partition(partition_begin, partition_end); });
    partition_begin = partition_end;
  });

  thread_pool_.wait_until_inactive();

  std::ranges::copy(bodies_, buffer_.begin());
}

auto SimulationCPU::buffer_id() const -> GLuint {
  return buffer_id_;
}

auto SimulationCPU::count() const -> usize {
  return buffer_.size();
}

auto SimulationCPU::construct_barnes_hut_tree() -> void {
  const auto [center, inradius] {determine_initial_center_and_inradius(bodies_)};

  node_pool_.deallocate();
  root_node_idx_ = node_pool_.allocate_node(center, inradius);

  std::ranges::for_each(std::views::iota(0uz, bodies_.size()),
                        [this](auto i) { insert_octree_node(root_node_idx_, i); });
}

auto SimulationCPU::insert_octree_node(usize node_idx, usize body_idx) -> void {
  // Case 1:
  // No body exists in this node yet, simply assign the body to this node.
  // Is also the base case for the recursion.
  if (node(node_idx).total_mass == 0.0f) {
    node(node_idx).body_idx = static_cast<i32>(body_idx);
    node(node_idx).center_of_mass = body(body_idx).position;
    node(node_idx).total_mass = body(body_idx).mass;

    return;
  }

  // Case 2:
  // This node has already a body assigned, therefore there is only one node in this octant.
  // Adding another one requires subdivision of this octant.
  if (node(node_idx).body_idx != -1) {
    const auto assigned_body_idx {static_cast<usize>(node(node_idx).body_idx)};
    node(node_idx).body_idx = -1;

    insert_octree_child_node(node_idx, assigned_body_idx);
    insert_octree_child_node(node_idx, body_idx);
  }
  // Case 3:
  // This octant is already subdivided.
  // Therefore simply continue the recursion.
  else {
    insert_octree_child_node(node_idx, body_idx);
  }

  const auto weighted_center {node(node_idx).total_mass * node(node_idx).center_of_mass +
                              body(body_idx).mass * body(body_idx).position};
  node(node_idx).total_mass += body(body_idx).mass;
  node(node_idx).center_of_mass = (1.0f / node(node_idx).total_mass) * weighted_center;
}

auto SimulationCPU::insert_octree_child_node(usize node_idx, usize body_idx) -> void {
  const auto octant {determine_octant(node(node_idx).center, body(body_idx).position)};

  if (node(node_idx).children[octant] == -1) {
    const auto [center,
                inradius] {determine_node_center_and_inradius(octant, node(node_idx).center, node(node_idx).inradius)};
    const auto child_idx {node_pool_.allocate_node(center, inradius)};
    node(node_idx).children[octant] = static_cast<i32>(child_idx);
  }

  insert_octree_node(static_cast<usize>(node(node_idx).children[octant]), body_idx);
}

auto SimulationCPU::step_partition(usize begin, usize end) -> void {
  std::ranges::for_each(std::views::iota(begin, end), [this](auto i) {
    body(i).position +=
        body(i).velocity * parameters_.dt + 0.5f * body(i).acceleration * parameters_.dt * parameters_.dt;
    body(i).velocity += 0.5f * body(i).acceleration * parameters_.dt;
    body(i).acceleration = glm::vec4 {0.0};
  });

  thread_pool_.barrier();

  if (parameters_.use_barnes_hut) {
    std::ranges::for_each(std::views::iota(begin, end),
                          [this](auto i) { calculate_acceleration_barnes_hut(root_node_idx_, i, parameters_.theta); });
  } else {
    std::ranges::for_each(std::views::iota(begin, end), [this](auto i) { calculate_acceleration_all_pairs(i); });
  }

  std::ranges::for_each(std::views::iota(begin, end),
                        [this](auto i) { body(i).velocity += 0.5f * body(i).acceleration * parameters_.dt; });
}

auto SimulationCPU::calculate_acceleration_all_pairs(usize body_idx) -> void {
  std::ranges::for_each(bodies_, [&](auto& b) {
    const auto direction {b.position - body(body_idx).position};
    const auto distance {glm::dot(direction, direction) + parameters_.eps * parameters_.eps};
    const auto acceleration {(parameters_.G * b.mass) / (glm::pow(distance, 1.5f)) * direction};

    body(body_idx).acceleration += acceleration;
  });
}

auto SimulationCPU::calculate_acceleration_barnes_hut(usize node_idx, usize body_idx, f32 theta) -> void {
  if (node(node_idx).body_idx == static_cast<i32>(body_idx)) {
    return;
  }

  const auto direction {node(node_idx).center_of_mass - body(body_idx).position};
  const auto distance {glm::length(direction) + parameters_.eps * parameters_.eps};

  if (node(node_idx).body_idx != -1 || (node(node_idx).inradius / distance < theta)) {
    body(body_idx).acceleration += (parameters_.G * node(node_idx).total_mass / glm::pow(distance, 3.0f)) * direction;
    return;
  }

  std::ranges::for_each(node(node_idx).children | std::views::filter([](auto c) { return c != -1; }),
                        [&](auto i) { calculate_acceleration_barnes_hut(static_cast<usize>(i), body_idx, theta); });
}

} // namespace nbodysim::simulation
