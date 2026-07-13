#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "platform/types.hpp"

namespace nbodysim::rendering {

struct RenderingSettings {
  /// Radius of each body.
  f32 body_radius {1.0f};

  /// Diameter of view frustum.
  f32 frustum_size {10.0f};

  /// Origin of view frustum.
  glm::vec2 frustum_origin {0.0f};
};

class Renderer {
public:
  /// Constructs a renderer.
  ///
  /// @param width Width of the window.
  /// @param height Height of the window.
  Renderer(u32 width, u32 height);

  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  auto operator=(const Renderer&) -> Renderer& = delete;
  auto operator=(Renderer&&) -> Renderer& = delete;
  ~Renderer();

  /// Renders the given SSBO containing bodies to the render target.
  ///
  /// This method must be called within an existing OpenGL context.
  ///
  /// @param buffer_id ID for the SSBO to be rendered.
  /// @param count Number of bodies in the SSBO.
  auto render(GLuint buffer_id, usize count, const RenderingSettings& settings) -> void;

  /// Clears the current render target.
  ///
  /// Clearing the screen should usually be done before rendering the current state.
  ///
  /// @param color The color with which the render target should be cleared.
  auto clear(glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.0f}) -> void;

private:
  GLuint vertex_array_object_id_;
  GLuint shader_program_id_;

  u32 width_;
  u32 height_;

  GLsync fence_;
};

} // namespace nbodysim::rendering
