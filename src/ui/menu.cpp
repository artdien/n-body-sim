#include "ui/menu.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace nbodysim::ui {

namespace {

constexpr auto body_radius_min {0.01f};
constexpr auto body_radius_max {10.0f};

} // namespace

Menu::Menu(f32* body_radius, glm::vec3* body_color, bool visible)
    : body_radius_ {body_radius}, body_color_ {body_color}, visible_ {visible} {
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

  ImGui::ColorEdit3("Body Color", &body_color_->x);
  ImGui::SliderScalar("Body Radius", ImGuiDataType_Float, body_radius_, &body_radius_min, &body_radius_max);

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

auto Menu::toggle() -> void {
  visible_ = !visible_;
}

} // namespace nbodysim::ui