#include "simulation/simulation.hpp"

#include <glm/geometric.hpp>

#include "platform/types.hpp"
#include "simulation/initialization.hpp"

namespace nbodysim::simulation {

namespace {

constexpr auto G {6.6743e-11f};
constexpr auto dt {60.0f};
constexpr auto eps {0.1f};

} // namespace

Simulation::Simulation(InitializationSetup setup) {
  bodies_ = initialize_bodies(setup);
}

auto Simulation::step() -> void {
  for (auto& body : bodies_) {
    body.position = body.position + body.velocity * dt + 0.5f * body.acceleration * dt * dt;
  }

  for (auto& body : bodies_) {
    body.velocity = body.velocity + 0.5f * body.acceleration * dt;
    body.acceleration = glm::vec4 {0.0};
  }

  for (usize i = 0; i < bodies_.size(); ++i) {
    for (usize j = i + 1; j < bodies_.size(); ++j) {
      const auto direction {bodies_[j].position - bodies_[i].position};
      const auto distance {glm::dot(direction, direction) + eps * eps};
      const auto acceleration {G / (glm::pow(distance, 1.5f)) * direction};
      bodies_[i].acceleration += acceleration;
      bodies_[j].acceleration -= acceleration;
    }
  }

  for (auto& body : bodies_) {
    body.velocity += 0.5f * body.acceleration * dt;
  }
}

auto Simulation::bodies() const -> std::span<const Body> {
  return bodies_;
}

} // namespace nbodysim::simulation