#include "rendering/renderer.hpp"

#include <string_view>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "utils/gl.hpp"

namespace nbodysim::rendering {

namespace {

constexpr auto VERTEX_SHADER = std::string_view {
#include "shaders/shader.vert"
};

constexpr auto FRAGMENT_SHADER = std::string_view {
#include "shaders/shader.frag"
};

} // namespace

Renderer::Renderer(u32 width, u32 height) : width_ {width}, height_ {height}, fence_ {nullptr} {
  const auto vertex_shader_id {utils::compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER.data())};
  const auto fragment_shader_id {utils::compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER.data())};
  shader_program_id_ = utils::link_shaders(vertex_shader_id, fragment_shader_id);

  glViewport(0, 0, width, height);

  // We use an SSBO with programmable vertex pulling for drawing and therefore do not really need a VAO.
  // However, since the OpenGL specification requires a bound VAO to make a draw call,
  // we will bind here a "dummy" VAO to satisfy this requirement.
  glCreateVertexArrays(1, &vertex_array_object_id_);
  glBindVertexArray(vertex_array_object_id_);
}

Renderer::~Renderer() {
  glDeleteProgram(shader_program_id_);
  glDeleteVertexArrays(1, &vertex_array_object_id_);
  glDeleteSync(fence_);
}

auto Renderer::render(GLuint buffer_id, usize count) -> void {
  const auto aspect_ratio {static_cast<f32>(width_) / height_};
  const auto frustum_size_half {0.5f * frustum_size};
  const auto projection {glm::ortho(-frustum_size_half * aspect_ratio, frustum_size_half * aspect_ratio,
                                    -frustum_size_half, frustum_size_half)};

  if (fence_) {
    if (const auto result {glClientWaitSync(fence_, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED)};
        result == GL_WAIT_FAILED) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, -1,
                           "Waiting on fence failed");
    }
  }

  glUseProgram(shader_program_id_);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_id);

  glUniform1f(glGetUniformLocation(shader_program_id_, "body_radius"), body_radius);
  glUniformMatrix4fv(glGetUniformLocation(shader_program_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count);

  glDeleteSync(fence_);
  fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

auto Renderer::clear(glm::vec4 color) -> void {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace nbodysim::rendering
