#include "simulation/simulation.hpp"

#include <algorithm>
#include <ranges>

#include <glm/geometric.hpp>

#include "platform/types.hpp"
#include "simulation/initialization.hpp"

namespace nbodysim::simulation {

namespace {

constexpr auto G {6.6743e-11f};
constexpr auto dt {60.0f};
constexpr auto eps {0.1f};

} // namespace

Simulation::Simulation(const std::vector<Body>& bodies) : bodies_ {bodies}, num_threads_ {thread_pool_.capacity()} {}

auto Simulation::step() -> void {
  const auto lock {std::lock_guard {mutex_}};

  const auto size {bodies_.size() / num_threads_};
  const auto remainder {bodies_.size() % num_threads_};

  auto partition_begin {0uz};
  auto partition_end {0uz};

  std::ranges::for_each(std::views::iota(0uz, num_threads_), [&, this](auto i) {
    partition_end += size + (i < remainder ? 1uz : 0uz);
    thread_pool_.schedule([=, this] { step_partition(partition_begin, partition_end); });
    partition_begin = partition_end;
  });

  thread_pool_.wait_until_inactive();
}

auto Simulation::initialize_setup(InitializationSetup setup) -> void {
  const auto lock {std::lock_guard {mutex_}};

  thread_pool_.wait_until_inactive();

  bodies_ = initialize_bodies(setup);
  bodies_.shrink_to_fit();
}

auto Simulation::bodies() const -> std::span<const Body> {
  return bodies_;
}

auto Simulation::step_partition(usize begin, usize end) -> void {
  std::ranges::for_each(std::views::iota(begin, end), [this](auto i) {
    bodies_[i].position += bodies_[i].velocity * dt + 0.5f * bodies_[i].acceleration * dt * dt;
    bodies_[i].velocity += 0.5f * bodies_[i].acceleration * dt;
    bodies_[i].acceleration = glm::vec4 {0.0};
  });

  thread_pool_.barrier();

  std::ranges::for_each(std::views::iota(begin, end), [this](auto i) {
    auto other_bodies {std::views::iota(0uz, bodies_.size()) | std::views::filter([&](auto j) { return i != j; })};

    std::ranges::for_each(other_bodies, [&, this](auto j) {
      const auto direction {bodies_[j].position - bodies_[i].position};
      const auto distance {glm::dot(direction, direction) + eps * eps};
      const auto acceleration {G / (glm::pow(distance, 1.5f)) * direction};

      bodies_[i].acceleration += acceleration;
    });
  });

  std::ranges::for_each(std::views::iota(begin, end),
                        [this](auto i) { bodies_[i].velocity += 0.5f * bodies_[i].acceleration * dt; });
}

} // namespace nbodysim::simulation
