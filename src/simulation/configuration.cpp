#include "simulation/configuration.hpp"

#include <numbers>
#include <random>

#include <glm/geometric.hpp>

namespace nbodysim::simulation {

namespace {

constexpr auto SQRT_THREE {std::numbers::sqrt3_v<float>};
constexpr auto INV_SQRT_THREE {std::numbers::inv_sqrt3_v<float>};
constexpr auto FIVE_OVER_FOUR {5.0f / 4.0f};
constexpr auto PI {std::numbers::pi_v<float>};

std::vector<Body> initialize_euler_configuration(f32 G, f32 radius, f32 mass) {
  auto bodies {std::vector<Body> {3}};

  auto& body_2 {bodies[2]};
  auto& body_1 {bodies[1]};
  auto& body_3 {bodies[0]};

  const auto velocity {std::sqrt(FIVE_OVER_FOUR * ((G * mass) / radius))};

  body_1.position = {radius, 0.0f, 0.0f};
  body_1.velocity = {0.0f, velocity, 0.0f};
  body_1.mass = mass;
  body_1.color = {1.0f, 1.0f, 1.0f, 1.0f};

  body_2.position = -body_1.position;
  body_2.velocity = -body_1.velocity;
  body_2.mass = mass;
  body_2.color = {1.0f, 1.0f, 1.0f, 1.0f};

  body_3.position = {0.0f, 0.0f, 0.0f};
  body_3.velocity = {0.0f, 0.0f, 0.0f};
  body_3.mass = mass;
  body_3.color = {1.0f, 1.0f, 1.0f, 1.0f};

  return bodies;
}

std::vector<Body> initialize_lagrange_configuration(f32 G, f32 radius, f32 mass) {
  auto bodies {std::vector<Body> {3}};

  auto& body_1 {bodies[0]};
  auto& body_2 {bodies[1]};
  auto& body_3 {bodies[2]};

  const auto velocity {std::sqrt(INV_SQRT_THREE * ((G * mass) / radius))};

  body_1.position = {0.0f, radius, 0.0f};
  body_1.velocity = {-velocity, 0.0f, 0.0f};
  body_1.mass = mass;
  body_1.color = {1.0f, 1.0f, 1.0f, 1.0f};

  body_2.position = {-0.5f * SQRT_THREE * radius, -0.5f * radius, 0.0f};
  body_2.velocity = {0.5f * velocity, -0.5f * SQRT_THREE * velocity, 0.0f};
  body_2.mass = mass;
  body_2.color = {1.0f, 1.0f, 1.0f, 1.0f};

  body_3.position = {0.5f * SQRT_THREE * radius, -0.5f * radius, 0.0f};
  body_3.velocity = {0.5f * velocity, 0.5f * SQRT_THREE * velocity, 0.0f};
  body_3.mass = mass;
  body_3.color = {1.0f, 1.0f, 1.0f, 1.0f};

  return bodies;
}

std::vector<Body> initialize_plummer_configuration(f32 G, f32 radius, f32 mass, u32 count) {
  auto bodies {std::vector<Body> {count}};

  auto device {std::random_device {}};
  auto generator = std::mt19937 {device()};
  auto distribution = std::uniform_real_distribution {0.0f, 1.0f};

  const auto M {static_cast<f32>(count) * mass};
  const auto a {radius};

  for (auto& body : bodies) {
    // Sample position
    const auto r {a / std::sqrt(glm::pow(distribution(generator), -2.0f / 3.0f) - 1.0f)};
    const auto theta {std::acos(2.0f * distribution(generator) - 1.0f)};
    const auto phi {2.0f * PI * distribution(generator)};

    body.position.x = r * std::sin(theta) * std::cos(phi);
    body.position.y = r * std::sin(theta) * std::sin(phi);
    body.position.z = r * std::cos(theta);

    body.mass = mass;
    body.color = {1.0f, 1.0f, 1.0f, 1.0f};

    // Sample velocity (via rejection sampling)
    const auto v_max {std::sqrt((2.0f * G * M) / std::sqrt(a * a + r * r))};
    while (true) {
      const auto v {v_max * distribution(generator)};
      const auto accept {distribution(generator) <= glm::pow((1.0f - (v * v) / (v_max * v_max)), 7.0f / 2.0f)};

      if (accept) {
        const auto theta {std::acos(2.0f * distribution(generator) - 1.0f)};
        const auto phi {2.0f * PI * distribution(generator)};

        body.velocity.x = v * std::sin(theta) * std::cos(phi);
        body.velocity.y = v * std::sin(theta) * std::sin(phi);
        body.velocity.z = v * std::cos(theta);

        break;
      }
    }
  }

  // Center positions and velocities
  const auto [position_center, velocity_center] {[&]() {
    auto position_center {glm::vec3 {0.0f}};
    auto velocity_center {glm::vec3 {0.0f}};

    for (const auto& body : bodies) {
      position_center += body.position;
      velocity_center += body.velocity;
    }

    position_center /= M;
    velocity_center /= M;

    return std::tuple {position_center, velocity_center};
  }()};

  for (auto& body : bodies) {
    body.position -= position_center;
    body.velocity -= velocity_center;
  }

  return bodies;
}

} // namespace

std::vector<Body> initialize_configuration(f32 G, const Configuration& configuration) {
  switch (configuration.type) {
  case ConfigurationType::EULER_THREE_BODY:
    return initialize_euler_configuration(G, configuration.radius, configuration.mass);
  case ConfigurationType::LAGRANGE_THREE_BODY:
    return initialize_lagrange_configuration(G, configuration.radius, configuration.mass);
  case ConfigurationType::PLUMMER_N_BODY:
    return initialize_plummer_configuration(G, configuration.radius, configuration.mass, configuration.count);
  }
}

} // namespace nbodysim::simulation
