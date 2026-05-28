#pragma once

#include <optional>
#include <string>

namespace nbodysim::platform {

enum class ScrollDirection {
  NONE,
  UP,
  DOWN,
};

struct MouseInput {
  ScrollDirection scroll_direction {ScrollDirection::NONE};
};

struct KeyboardInput {
  std::string pressed_key {""};
};

/// Stores a mouse input event in a queue for later retrieval.
///
/// @param mouse_input Event to be stored.
void add_mouse_input_event(MouseInput mouse_input);

/// Stores a keyboard input event in a queue for later retrieval.
///
/// @param mouse_input Event to be stored.
void add_keyboard_input_event(KeyboardInput keyboard_input);

/// Retrieves a stored mouse input event.
///
/// Since events are stored in a queue, they are retrieved in FIFO order.
///
/// @return Optional containing mouse input event if queue is non-empty, otherwise std::nullopt.
std::optional<MouseInput> get_mouse_input_event();

/// Retrieves a stored keyboard input event.
///
/// Since events are stored in a queue, they are retrieved in FIFO order.
///
/// @return Optional containing keyboard input event if queue is non-empty, otherwise std::nullopt.
std::optional<KeyboardInput> get_keyboard_input_event();

} // namespace nbodysim::platform