#pragma once

#include <concepts>
#include <vector>

#include <glad/glad.h>

#include "platform/types.hpp"

namespace nbodysim::utils {

/// Compiles a GLSL shader from the provided source code.
///
/// This method must be called within an existing OpenGL context.
///
/// @param shader_type Type of shader to create (GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, ...).
/// @param shader Null-terminated string containing the GLSL source code.
/// @return ID of the compiled shader.
inline auto compile_shader(GLuint shader_type, const char* shader) -> GLuint {
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

/// Links multiple compiled shaders into a single shader program.
///
/// This method must be called within an existing OpenGL context.
/// The individual shaders are detached and deleted after linking.
///
/// @param shader_ids One or more IDs of compiled shaders to be linked into the program.
/// @return ID of the shader program.
template <std::integral... Args>
requires(sizeof...(Args) > 0)
inline auto link_shaders(Args... shader_ids) -> GLuint {
  const auto shader_program_id {glCreateProgram()};

  (glAttachShader(shader_program_id, shader_ids), ...);
  glLinkProgram(shader_program_id);

  GLint log_size;
  glGetProgramiv(shader_program_id, GL_INFO_LOG_LENGTH, &log_size);
  if (log_size > 0) {
    std::vector<char> log(static_cast<u32>(log_size + 1));
    glGetProgramInfoLog(shader_program_id, log_size, nullptr, log.data());
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1,
                         log.data());
  }

  ((glDetachShader(shader_program_id, shader_ids), glDeleteShader(shader_ids)), ...);

  return shader_program_id;
}

} // namespace nbodysim::utils
