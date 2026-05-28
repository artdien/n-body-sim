#include "rendering/renderer.hpp"

#include <algorithm>
#include <string_view>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "simulation/body.hpp"

namespace nbodysim::rendering {

namespace {

constexpr auto VERTEX_SHADER = std::string_view {
#include "shaders/shader.vert"
};

constexpr auto FRAGMENT_SHADER = std::string_view {
#include "shaders/shader.frag"
};

auto compile_shader(GLuint shader_type, const char* shader) -> GLuint {
  const auto shader_id {glCreateShader(shader_type)};
  glShaderSource(shader_id, 1, &shader, nullptr);
  glCompileShader(shader_id);

  GLint log_size;
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_size);
  if (log_size > 0) {
    std::vector<char> log(static_cast<u32>(log_size + 1));
    glGetShaderInfoLog(shader_id, log_size, nullptr, log.data());
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                         log.data());
  }

  return shader_id;
}

auto link_shaders(GLuint vertex_shader_id, GLuint fragment_shader_id) -> GLuint {
  const auto shader_program_id {glCreateProgram()};
  glAttachShader(shader_program_id, vertex_shader_id);
  glAttachShader(shader_program_id, fragment_shader_id);
  glLinkProgram(shader_program_id);

  GLint log_size;
  glGetProgramiv(shader_program_id, GL_INFO_LOG_LENGTH, &log_size);
  if (log_size > 0) {
    std::vector<char> log(static_cast<u32>(log_size + 1));
    glGetProgramInfoLog(shader_program_id, log_size, nullptr, log.data());
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                         log.data());
  }

  glDetachShader(shader_program_id, vertex_shader_id);
  glDeleteShader(vertex_shader_id);

  glDetachShader(shader_program_id, fragment_shader_id);
  glDeleteShader(fragment_shader_id);

  return shader_program_id;
}

} // namespace

Renderer::Renderer(u32 width, u32 height, std::span<const simulation::Body> bodies)
    : width_ {width}, height_ {height}, bodies_ {bodies} {
  const auto vertex_shader_id {compile_shader(GL_VERTEX_SHADER, VERTEX_SHADER.data())};
  const auto fragment_shader_id {compile_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER.data())};
  shader_program_id_ = link_shaders(vertex_shader_id, fragment_shader_id);

  const auto flags {GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT};
  glCreateBuffers(1, &vertex_buffer_object_id_);
  glNamedBufferStorage(vertex_buffer_object_id_, bodies.size() * sizeof(simulation::Body), nullptr, flags);
  bodies_mapped_ = std::span {reinterpret_cast<simulation::Body*>(glMapNamedBufferRange(
                                  vertex_buffer_object_id_, 0, bodies.size() * sizeof(simulation::Body), flags)),
                              bodies.size()};

  glCreateVertexArrays(1, &vertex_array_object_id_);
  glBindVertexArray(vertex_array_object_id_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_buffer_object_id_);
  glBindVertexArray(0);

  glViewport(0, 0, width, height);
}

Renderer::~Renderer() {
  glDeleteProgram(shader_program_id_);
  glUnmapNamedBuffer(vertex_buffer_object_id_);
  glDeleteBuffers(1, &vertex_buffer_object_id_);
  glDeleteVertexArrays(1, &vertex_array_object_id_);
  glDeleteSync(fence_);
}

auto Renderer::render() -> void {
  const auto aspect_ratio {static_cast<f32>(width_) / height_};
  const auto frustum_size_half {0.5f * frustum_size_};
  const auto projection {glm::ortho(-frustum_size_half * aspect_ratio, frustum_size_half * aspect_ratio,
                                    -frustum_size_half, frustum_size_half)};

  if (fence_) {
    if (const auto result {glClientWaitSync(fence_, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED)};
        result == GL_WAIT_FAILED) {
      glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR, 0, GL_DEBUG_SEVERITY_HIGH, -1,
                           "Waiting on fence failed");
    }
  }

  std::ranges::copy(bodies_, bodies_mapped_.begin());

  glUseProgram(shader_program_id_);
  glBindVertexArray(vertex_array_object_id_);

  glUniform1f(glGetUniformLocation(shader_program_id_, "body_radius"), body_radius);
  glUniform3fv(glGetUniformLocation(shader_program_id_, "body_color"), 1, glm::value_ptr(body_color));
  glUniformMatrix4fv(glGetUniformLocation(shader_program_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, bodies_.size());

  glBindVertexArray(0);
  glUseProgram(0);

  glDeleteSync(fence_);
  fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

auto Renderer::clear(glm::vec4 color) -> void {
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);
}

auto Renderer::adjust_frustum_size(f32 value) -> void {
  frustum_size_ = std::max(0.01f, frustum_size_ + value);
}

} // namespace nbodysim::rendering
