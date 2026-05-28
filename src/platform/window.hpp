#pragma once

#define GLFW_INCLUDE_NONE

#include <functional>
#include <string>

#include <GLFW/glfw3.h>

#include "platform/input.hpp"
#include "platform/types.hpp"

namespace nbodysim::platform {

class Window {
public:
  /// Constructs window.
  ///
  /// Constructing a window does not open it automatically.
  /// The open method must be called for that.
  ///
  /// @param title Title of the window.
  /// @param width Width of the window.
  /// @param height Height of the window.
  Window(std::string title, u32 width, u32 height);

  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  Window(Window&&) = default;
  Window& operator=(Window&&) = default;
  ~Window();

  /// Opens a window and runs it indefinitely until it is closed.
  ///
  /// @param execute_per_frame A function which will be executed once per frame.
  ///                          Typically this function should contain update and rendering logic.
  auto open(std::function<void(MouseInput, KeyboardInput)> execute_per_frame) -> void;

private:
  GLFWwindow* window_;
  std::string title_;
  u32 width_;
  u32 height_;
};

} // namespace nbodysim::platform