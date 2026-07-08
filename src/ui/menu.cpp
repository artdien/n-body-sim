#include "ui/menu.hpp"

#include <array>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "simulation/configuration.hpp"

namespace nbodysim::ui {

namespace {

constexpr auto CONFIGURATION_TYPE_DESCRIPTIONS {std::array {"Euler (3-Body)", "Lagrange (3-Body)", "Plummer (N-Body)"}};

constexpr auto BODY_RADIUS_MIN {0.01f};
constexpr auto BODY_RADIUS_MAX {10.0f};
constexpr auto FRUSTUM_SIZE_MIN {0.01f};
constexpr auto FRUSTUM_SIZE_MAX {100.0f};

auto adjust_render_settings(rendering::Renderer* renderer, const simulation::ConfigurationType& type) -> void {
  // Plummer model creates lots of bodies that are more spread out.
  // The frustum size is therefore larger for this setup.
  renderer->frustum_size = type == simulation::ConfigurationType::PLUMMER_N_BODY ? 100.0f : 3.0f;
  renderer->body_radius = 0.1f;
}

} // namespace

Menu::Menu(rendering::Renderer* renderer, std::unique_ptr<simulation::Simulation>* simulation,
           simulation::SimulationParametersCPU* parameters_cpu, simulation::SimulationParametersGPU* parameters_gpu,
           simulation::Configuration* configuration, bool visible)
    : renderer_ {renderer}, simulation_ {simulation}, parameters_cpu_ {parameters_cpu},
      parameters_gpu_ {parameters_gpu}, configuration_ {configuration}, visible_ {visible} {

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

  ImGui::Combo("N-Body Setup", reinterpret_cast<i32*>(&configuration_->type), CONFIGURATION_TYPE_DESCRIPTIONS.data(),
               CONFIGURATION_TYPE_DESCRIPTIONS.size());

  if (ImGui::Button("Apply Configuration")) {
    *simulation_ =
        std::make_unique<simulation::Simulation>(std::in_place_type<simulation::SimulationGPU>, *parameters_gpu_,
                                                 initialize_configuration(parameters_gpu_->general.G, *configuration_));
    adjust_render_settings(renderer_, configuration_->type);
  }

  ImGui::SeparatorText("Visualization");

  ImGui::SliderScalar("Body Radius", ImGuiDataType_Float, &renderer_->body_radius, &BODY_RADIUS_MIN, &BODY_RADIUS_MAX);
  ImGui::SliderScalar("Frustum Size", ImGuiDataType_Float, &renderer_->frustum_size, &FRUSTUM_SIZE_MIN,
                      &FRUSTUM_SIZE_MAX);

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

auto Menu::toggle() -> void {
  visible_ = !visible_;
}

} // namespace nbodysim::ui
