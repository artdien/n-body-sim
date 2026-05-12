#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <stdexcept>

int main() {
  constexpr int window_width = 1920;
  constexpr int window_height = 1080;

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1920, 1080, "N-Body Simulation", NULL, NULL);
  if (!window) {
    throw std::runtime_error("Failed to create window");
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  gladLoadGLLoader(reinterpret_cast<GLADloadproc>(&glfwGetProcAddress));

  std::cout << "Initialized GLFW with OpenGL" << std::endl;
  std::cout << "  Version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "  GPU: " << glGetString(GL_RENDERER) << std::endl;

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io {ImGui::GetIO()};
  io.IniFilename = nullptr;

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init();

  while (!glfwWindowShouldClose(window)) {
    glViewport(0, 0, window_width, window_height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}