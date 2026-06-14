#pragma once

#include <array>
#include <vector>

#include <glm/vec3.hpp>

#include "platform/types.hpp"

namespace nbodysim::simulation {

struct OctreeNode {
  i32 body_idx;
  glm::vec3 center;
  f32 size;
  glm::vec3 center_of_mass;
  f32 total_mass;

  std::array<i32, 8> children;
};

class OctreeNodePool {
public:
  OctreeNodePool(usize capacity = 1uz);

  OctreeNodePool(const OctreeNodePool&) = delete;
  OctreeNodePool& operator=(const OctreeNodePool&) = delete;
  OctreeNodePool(OctreeNodePool&&) = delete;
  OctreeNodePool& operator=(OctreeNodePool&&) = delete;

  /// Allocates a node in the next free space of the underling pool.
  ///
  /// If the underlying pool is full it will be enlarged before allocating the node.
  ///
  /// @param center Center of the allocated node.
  /// @param size Size of the allocated node.
  /// @return Index of the allocated node in the pool.
  auto allocate_node(const glm::vec3& center = {0.0f, 0.0f, 0.0f}, f32 size = 0.0f) -> usize;

  /// Deallocates all nodes in the entire pool.
  ///
  /// Allocating new node will override existing ones.
  /// The underlying memory reserved for the pool is not freed.
  auto deallocate() -> void;

  /// Resizes the underlying pool.
  ///
  /// If the new capacity is smaller than the current size of the pool
  /// the pool will be reduced to the first n elements, where n is the new capacity.
  ///
  /// @param capacity New capacity of the pool.
  auto resize(usize capacity) -> void;

  /// Frees the underlying memory for the pool.
  auto free() -> void;

  auto operator[](usize i) -> OctreeNode& {
    return nodes_[i];
  }
  auto operator[](usize i) const -> const OctreeNode& {
    return nodes_[i];
  }

private:
  usize current_;
  std::vector<OctreeNode> nodes_;
};

} // namespace nbodysim::simulation
