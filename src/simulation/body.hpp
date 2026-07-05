#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "platform/types.hpp"

namespace nbodysim::simulation {

// Alignments are due to:
// - std430 layout for SSBO in shaders, which must match with alignment of this struct
// - avoidance of false sharing when processing vector of this struct multi-threaded
struct alignas(std::hardware_constructive_interference_size) Body {
  alignas(16) glm::vec3 position;
  f32 radius;
  alignas(16) glm::vec3 velocity;
  f32 mass;
  alignas(16) glm::vec3 acceleration;
  alignas(16) glm::vec4 color;
};

static_assert(alignof(Body) >= 64);
static_assert(sizeof(Body) == 64);

} // namespace nbodysim::simulation
