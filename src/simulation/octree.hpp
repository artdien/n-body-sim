#pragma once

#include <array>
#include <vector>

#include <glm/vec3.hpp>

#include "platform/types.hpp"

namespace nbodysim::simulation {

struct OctreeNode {
  i32 body_idx;
  glm::vec3 center;
  f32 inradius;
  glm::vec3 center_of_mass;
  f32 total_mass;

  std::array<i32, 8> children;
};

class OctreeNodePool {
public:
  OctreeNodePool(usize capacity = 1uz);

  OctreeNodePool(const OctreeNodePool&) = delete;
  OctreeNodePool(OctreeNodePool&&) = delete;
  auto operator=(const OctreeNodePool&) -> OctreeNodePool& = delete;
  auto operator=(OctreeNodePool&&) -> OctreeNodePool& = delete;

  /// Allocates a node in the next free space of the underling pool.
  ///
  /// If the underlying pool is full it will be enlarged before allocating the node.
  ///
  /// @param center Center of cube for the allocated node.
  /// @param inradius Inradius of cube for the allocated node.
  /// @return Index of the allocated node in the pool.
  auto allocate_node(const glm::vec3& center = {0.0f, 0.0f, 0.0f}, f32 inradius = 0.0f) -> usize;

  /// Deallocates all nodes in the entire pool.
  ///
  /// Allocating new node will override existing ones.
  /// The underlying memory reserved for the pool is not freed.
  auto deallocate() -> void;

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
