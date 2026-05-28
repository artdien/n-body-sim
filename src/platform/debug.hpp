#pragma once

#include <glad/glad.h>

namespace nbodysim::platform {

#if !defined(NDEBUG)
constexpr bool DEBUG_BUILD = true;
#else
constexpr bool DEBUG_BUILD = false;
#endif

/// Callback to use in conjunction with OpenGL debug messaging.
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message,
                    void const* user_param);

} // namespace nbodysim::platform