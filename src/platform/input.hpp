#pragma once

#include <optional>
#include <string>

#include <glm/vec2.hpp>

namespace nbodysim::platform {

enum class ScrollDirection {
  NONE,
  UP,
  DOWN,
};

struct MouseInput {
  ScrollDirection scroll_direction {ScrollDirection::NONE};
  bool dragging {false};
  glm::vec2 delta {0.0};
};

struct KeyboardInput {
  std::string pressed_key {""};
};

/// Stores a mouse input event in a queue for later retrieval.
///
/// @param mouse_input Event to be stored.
auto add_mouse_input_event(MouseInput mouse_input) -> void;

/// Stores a keyboard input event in a queue for later retrieval.
///
/// @param mouse_input Event to be stored.
auto add_keyboard_input_event(KeyboardInput keyboard_input) -> void;

/// Retrieves a stored mouse input event.
///
/// Since events are stored in a queue, they are retrieved in FIFO order.
///
/// @return Optional containing mouse input event if queue is non-empty, otherwise std::nullopt.
auto get_mouse_input_event() -> std::optional<MouseInput>;

/// Retrieves a stored keyboard input event.
///
/// Since events are stored in a queue, they are retrieved in FIFO order.
///
/// @return Optional containing keyboard input event if queue is non-empty, otherwise std::nullopt.
auto get_keyboard_input_event() -> std::optional<KeyboardInput>;

} // namespace nbodysim::platform
