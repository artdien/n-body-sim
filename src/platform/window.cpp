#include "platform/window.hpp"

#include <print>
#include <stdexcept>

#include <glad/glad.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "platform/debug.hpp"
#include "platform/input.hpp"

namespace nbodysim::platform {

namespace {

void scroll_callback([[maybe_unused]] GLFWwindow* window, [[maybe_unused]] double offset_x, double offset_y) {
  if (offset_y > 0) {
    add_mouse_input_event({.scroll_direction = ScrollDirection::UP});
  }

  if (offset_y < 0) {
    add_mouse_input_event({.scroll_direction = ScrollDirection::DOWN});
  }
}

void key_callback([[maybe_unused]] GLFWwindow* window, int key, int scan_code, int action,
                  [[maybe_unused]] int modifiers) {
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_ESCAPE) {
      add_keyboard_input_event({.pressed_key = std::string {"esc"}});
    }
    if (const auto key_name {glfwGetKeyName(key, scan_code)}; key_name) {
      add_keyboard_input_event({.pressed_key = std::string {key_name}});
    }
  }
}

} // namespace

Window::Window(u32 width, u32 height, const std::string& title) : width_ {width}, height_ {height}, title_ {title} {
  glfwSetErrorCallback([](int error, const char* description) {
    std::println(stderr, "Error initializing window: [{}] {}", error, description);
  });

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  if constexpr (DEBUG_BUILD) {
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  }

  window_ = glfwCreateWindow(static_cast<i32>(width_), static_cast<i32>(height_), title_.c_str(), NULL, NULL);
  if (!window_) {
    glfwTerminate();
    throw std::runtime_error("Failed to create window");
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
  gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&glfwGetProcAddress));

  if constexpr (DEBUG_BUILD) {
    // Can only be enabled after OpenGL context is created
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    glDebugMessageCallback(debug_callback, nullptr);
  }

  glfwSetKeyCallback(window_, key_callback);
  glfwSetScrollCallback(window_, scroll_callback);

  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window_, true);
  ImGui_ImplOpenGL3_Init();

  std::println("Initialized window with OpenGL context");
  std::println("  OpenGL: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
  std::println("  GPU: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
}

Window::~Window() {
  glfwDestroyWindow(window_);
  glfwTerminate();
}

auto Window::open(std::function<void(MouseInput, KeyboardInput)> execute_per_frame) -> void {
  while (!glfwWindowShouldClose(window_)) {
    auto mouse_input {get_mouse_input_event()};
    if (mouse_input.has_value() && ImGui::GetIO().WantCaptureMouse) {
      mouse_input.reset();
    }

    auto keyboard_input {get_keyboard_input_event()};
    if (keyboard_input.has_value() && ImGui::GetIO().WantCaptureKeyboard) {
      keyboard_input.reset();
    }

    execute_per_frame(mouse_input.value_or({}), keyboard_input.value_or({}));

    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
}

auto Window::close() -> void {
  glfwSetWindowShouldClose(window_, GL_TRUE);
}

auto Window::set_title(const std::string& title) -> void {
  title_ = title;
  glfwSetWindowTitle(window_, title_.c_str());
}

} // namespace nbodysim::platform
