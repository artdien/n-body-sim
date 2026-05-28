#pragma once

#include <glm/vec4.hpp>

namespace nbodysim::simulation {

// Struct is used in SSBO with std430 layout, therefore alignment must match the one in shader.
struct Body {
  alignas(16) glm::vec4 position;
  alignas(16) glm::vec4 velocity;
  alignas(16) glm::vec4 acceleration;
};

static_assert(alignof(Body) == 16);
static_assert(sizeof(Body) == 48);

} // namespace nbodysim::simulation
