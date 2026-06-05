#pragma once

#include <mutex>
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
  Renderer(u32 width, u32 height);
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;
  ~Renderer();

  /// Radius of each body.
  /// Size is specified relative to view frustum size.
  f32 body_radius {1.0f};

  /// Diameter of view frustum.
  f32 frustum_size {10.0f};

  /// Renders the current state to the render target.
  ///
  /// This method must be called within an existing OpenGL context.
  /// If the current state is empty (i.e. bodies have not been loaded yet) nothing will be rendered.
  auto render() -> void;

  /// Clears the current render target.
  ///
  /// Clearing the screen should usually be done before rendering the current state.
  ///
  /// @param color The color with which the render target should be cleared.
  auto clear(glm::vec4 color = {0.0f, 0.0f, 0.0f, 0.0f}) -> void;

  /// Loads the bodies to be rendered.
  ///
  /// @param bodies Non-owning view of bodies to be rendered.
  auto load_bodies(std::span<const simulation::Body> bodies) -> void;

private:
  GLuint vertex_array_object_id_;
  GLuint vertex_buffer_object_id_;
  GLuint shader_program_id_;

  u32 width_;
  u32 height_;

  GLsync fence_;
  std::mutex mutex;
  std::span<const simulation::Body> bodies_;
  std::span<simulation::Body> bodies_mapped_;
};

} // namespace nbodysim::rendering
