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
  /// @param width Width of the window.
  /// @param height Height of the window.
  /// @param title Title of the window.
  Window(u32 width, u32 height, const std::string& title = "");

  Window(const Window&) = delete;
  Window(Window&&) = delete;
  auto operator=(const Window&) -> Window& = delete;
  auto operator=(Window&&) -> Window& = delete;
  ~Window();

  /// Opens a window and runs it indefinitely until it is closed.
  ///
  /// @param execute_per_frame A function which will be executed once per frame.
  ///                          Typically this function should contain update and rendering logic.
  auto open(std::function<void(MouseInput, KeyboardInput)> execute_per_frame) -> void;

  /// Sets the title of the window.
  ///
  /// @param title Window title.
  auto set_title(const std::string& title) -> void;

private:
  GLFWwindow* window_;
  u32 width_;
  u32 height_;
  std::string title_;
};

} // namespace nbodysim::platform
