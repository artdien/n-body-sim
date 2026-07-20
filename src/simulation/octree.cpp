#include "simulation/octree.hpp"

namespace nbodysim::simulation {

OctreeNodePool::OctreeNodePool(usize capacity) : current_ {0uz} {
  nodes_.resize(capacity);
}

auto OctreeNodePool::allocate_node(const glm::vec3& center, f32 inradius) -> usize {
  if (current_ >= nodes_.size()) {
    nodes_.resize(2 * nodes_.size());
  }

  nodes_[current_] = {
      .body_idx = -1,
      .center = center,
      .inradius = inradius,
      .center_of_mass = {0.0f, 0.0f, 0.0f},
      .total_mass = 0.0f,
      .children = {-1, -1, -1, -1, -1, -1, -1, -1},
  };

  return current_++;
};

auto OctreeNodePool::deallocate() -> void {
  current_ = 0uz;
}

} // namespace nbodysim::simulation
