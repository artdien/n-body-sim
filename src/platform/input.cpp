#include "platform/input.hpp"

#include <queue>

namespace nbodysim::platform {

namespace {

std::queue<MouseInput> mouse_input_queue;
std::queue<KeyboardInput> keyboard_input_queue;

} // namespace

void add_mouse_input_event(MouseInput mouse_input) {
  mouse_input_queue.push(mouse_input);
}

void add_keyboard_input_event(KeyboardInput keyboard_input) {
  keyboard_input_queue.push(keyboard_input);
}

std::optional<MouseInput> get_mouse_input_event() {
  auto mouse_input = std::optional<MouseInput> {};
  if (!mouse_input_queue.empty()) {
    mouse_input.emplace(mouse_input_queue.front());
    mouse_input_queue.pop();
  }
  return mouse_input;
}

std::optional<KeyboardInput> get_keyboard_input_event() {
  auto keyboard_input = std::optional<KeyboardInput> {};
  if (!keyboard_input_queue.empty()) {
    keyboard_input.emplace(keyboard_input_queue.front());
    keyboard_input_queue.pop();
  }
  return keyboard_input;
}

} // namespace nbodysim::platform