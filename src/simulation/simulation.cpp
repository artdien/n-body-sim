#include "simulation/simulation.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace nbodysim::simulation {

Simulation::Simulation(u32 N) : bodies_ {N} {
  // Hard-coded example for testing
  auto& b1 {bodies_[0]};
  auto& b2 {bodies_[1]};

  b1.position = {-3.0, 0.0, 0.0, 0.0};
  b2.position = {+3.0, 0.0, 0.0, 0.0};
}

auto Simulation::step() -> void {
  // Hard-coded example for testing
  auto& b1 {bodies_[0]};
  auto& b2 {bodies_[1]};

  const auto rotation {glm::rotate(glm::mat4 {1.0f}, glm::radians(1.0f), glm::vec3 {0.0f, 0.0f, 1.0f})};
  b1.position = rotation * b1.position;
  b2.position = rotation * b2.position;
}

auto Simulation::bodies() const -> std::span<const Body> {
  return bodies_;
}

} // namespace nbodysim::simulation