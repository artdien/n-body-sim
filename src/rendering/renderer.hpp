#pragma once

#include <span>

#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "platform/types.hpp"
#include "simulation/body.hpp"

namespace nbodysim::rendering {

class Renderer {
public:
  /// Constructs a renderer.
  ///
  /// @param width Width of the window.
  /// @param height Height of the window.
  /// @param bodies Bodies which should be rendered.
  Renderer(u32 width, u32 height, std::span<const simulation::Body> bodies);
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = default;
  Renderer& operator=(Renderer&&) = default;
  ~Renderer();

  glm::vec3 body_color {1.0f, 1.0f, 1.0f};
  f32 body_radius {1.0f};

  /// Renders the current state to the render target.
  ///
  /// This method must be called within an existing OpenGL context.
  /// The current state consists of all bodies passed when constructing the renderer.
  auto render() -> void;

  /// Clears the current render target.
  ///
  /// Clearing the screen should usually be done before rendering the current state.
  ///
  /// @param color The color with which the render target should be cleared.
  auto clear(glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.0f}) -> void;

  /// Adjusts the current size of the view frustum.
  ///
  /// A positive value increases the frustum size, a negative decreases it.
  ///
  /// @param value The amount by which the frustum size should be adjusted.
  /// @note The frustum size is floored at a minimum value of 0.01.
  auto adjust_frustum_size(f32 value = 1.0f) -> void;

private:
  GLuint vertex_array_object_id_;
  GLuint vertex_buffer_object_id_;
  GLuint shader_program_id_;

  u32 width_;
  u32 height_;
  f32 frustum_size_ {10.0f};

  GLsync fence_ {nullptr};
  std::span<const simulation::Body> bodies_;
  std::span<simulation::Body> bodies_mapped_;
};

} // namespace nbodysim::rendering