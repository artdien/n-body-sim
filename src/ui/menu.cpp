#include "ui/menu.hpp"

#include <array>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace nbodysim::ui {

namespace {

constexpr auto N_BODY_SETUP_CHANGES = std::array {"Euler (3-Body)", "Lagrange (3-Body)", "Plummer (N-Body)"};

constexpr auto BODY_RADIUS_MIN {0.01f};
constexpr auto BODY_RADIUS_MAX {10.0f};
constexpr auto FRUSTUM_SIZE_MIN {0.01};
constexpr auto FRUSTUM_SIZE_MAX {100.0};

} // namespace

Menu::Menu(const VisualizationMenu& visualization_menu, SetupMenu setup_menu, bool visible)
    : visualization_menu_ {visualization_menu}, setup_menu_ {setup_menu}, visible_ {visible} {
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
}

auto Menu::display() -> void {
  if (!visible_) {
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Menu", &visible_, ImGuiWindowFlags_AlwaysAutoResize);

  ImGui::SeparatorText("N-Body Setup");

  if (setup_menu_.n_body_setup) {
    ImGui::Combo("N-Body Setup", reinterpret_cast<i32*>(setup_menu_.n_body_setup), N_BODY_SETUP_CHANGES.data(),
                 N_BODY_SETUP_CHANGES.size());
  }

  if (setup_menu_.on_n_body_setup_changed) {
    if (ImGui::Button("Load Setup")) {
      setup_menu_.on_n_body_setup_changed(*setup_menu_.n_body_setup);
    }
  }

  ImGui::SeparatorText("Visualization");

  if (visualization_menu_.body_radius) {
    ImGui::SliderScalar("Body Radius", ImGuiDataType_Float, visualization_menu_.body_radius, &BODY_RADIUS_MIN,
                        &BODY_RADIUS_MAX);
  }
  if (visualization_menu_.frustum_size) {
    ImGui::SliderScalar("Frustum Size", ImGuiDataType_Float, visualization_menu_.frustum_size, &FRUSTUM_SIZE_MIN,
                        &FRUSTUM_SIZE_MAX);
  }

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

auto Menu::toggle() -> void {
  visible_ = !visible_;
}

} // namespace nbodysim::ui