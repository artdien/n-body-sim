#include "simulation/initialization.hpp"

#include <cmath>
#include <numbers>
#include <random>

#include <glm/geometric.hpp>

#include "platform/types.hpp"
#include "simulation/body.hpp"

namespace nbodysim::simulation {

namespace {

constexpr auto G {6.6743e-11f};
constexpr auto SQRT_THREE = std::numbers::sqrt3_v<float>;
constexpr auto INV_SQRT_THREE = std::numbers::inv_sqrt3_v<float>;
constexpr auto PI = std::numbers::pi_v<float>;

std::vector<Body> initialize_euler_three_body() {
  auto bodies = std::vector<Body> {3};

  auto& body_1 {bodies[0]};
  auto& body_2 {bodies[1]};
  auto& body_3 {bodies[2]};

  body_1.position = {0.0f, 0.0f, 0.0f, 0.0f};
  body_1.velocity = {0.0f, 0.0f, 0.0f, 0.0f};

  body_2.position = {1.0f, 0.0f, 0.0f, 0.0f};
  body_2.velocity = {0.0f, std::sqrt((5.0f * G) / (4 * glm::length(body_2.position))), 0.0f, 0.0f};

  body_3.position = -body_2.position;
  body_3.velocity = -body_2.velocity;

  return bodies;
}

std::vector<Body> initialize_lagrange_three_body() {
  auto bodies = std::vector<Body> {3};

  auto& body_1 {bodies[0]};
  auto& body_2 {bodies[1]};
  auto& body_3 {bodies[2]};

  const auto velocity {std::sqrt(G * INV_SQRT_THREE)};

  body_1.position = {0.0f, 1.0f, 0.0f, 0.0f};
  body_1.velocity = {-velocity, 0.0f, 0.0f, 0.0f};

  body_2.position = {-0.5f * SQRT_THREE, -0.5f, 0.0f, 0.0f};
  body_2.velocity = {0.5f * velocity, -0.5f * SQRT_THREE * velocity, 0.0f, 0.0f};

  body_3.position = {0.5f * SQRT_THREE, -0.5f, 0.0f, 0.0f};
  body_3.velocity = {0.5f * velocity, 0.5f * SQRT_THREE * velocity, 0.0f, 0.0f};

  return bodies;
}

std::vector<Body> initialize_plummer_n_body(u32 n) {
  auto bodies = std::vector<Body> {n};

  auto device {std::random_device {}};
  auto generator = std::mt19937 {device()};
  auto distribution = std::uniform_real_distribution {0.0f, 0.99999f};

  const auto a {1.0f};
  const auto M = n; // every body is assumed to have mass equal to one

  for (auto& body : bodies) {
    const auto r {a / std::sqrt((1.0f / glm::pow(distribution(generator), 2.0f / 3.0f) - 1.0f))};
    const auto phi {2.0f * PI * distribution(generator)};
    const auto theta {std::acos(2.0f * distribution(generator) - 1.0f)};

    body.position.x = r * std::sin(theta) * std::cos(phi);
    body.position.y = r * std::sin(theta) * std::sin(phi);
    body.position.z = r * std::cos(theta);

    const auto v_max {std::sqrt((2.0f * G * M) / std::sqrt(r * r + a * a))};
    while (true) {
      const auto v {v_max * distribution(generator)};

      if (distribution(generator) < glm::pow((1.0f - v * v / v_max * v_max), 7.0f / 2.0f)) {
        const auto phi {2.0f * PI * distribution(generator)};
        const auto theta {std::acos(2.0f * distribution(generator) - 1.0f)};

        body.velocity.x = v * std::sin(theta) * std::cos(phi);
        body.velocity.y = v * std::sin(theta) * std::sin(phi);
        body.velocity.z = v * std::cos(theta);
        break;
      }
    }
  }

  const auto [position_center, velocity_center] = [&]() {
    auto position_center = glm::vec4 {0.0f};
    auto velocity_center = glm::vec4 {0.0f};

    for (const auto& body : bodies) {
      position_center += body.position;
      velocity_center += body.velocity;
    }

    position_center /= M;
    velocity_center /= M;

    return std::tuple {position_center, velocity_center};
  }();

  for (auto& body : bodies) {
    body.position -= position_center;
    body.velocity -= velocity_center;
  }

  return bodies;
}

} // namespace

std::vector<Body> initialize_bodies(InitializationSetup setup) {
  switch (setup) {
  case InitializationSetup::EULER_THREE_BODY:
    return initialize_euler_three_body();
  case InitializationSetup::LAGRANGE_THREE_BODY:
    return initialize_lagrange_three_body();
  case InitializationSetup::PLUMMER_N_BODY:
    return initialize_plummer_n_body(100);
  }
}

} // namespace nbodysim::simulation