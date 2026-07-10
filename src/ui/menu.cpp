#include "ui/menu.hpp"

#include <array>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "simulation/configuration.hpp"

namespace nbodysim::ui {

namespace {

constexpr auto CONFIGURATION_TYPE_DESCRIPTIONS {std::array {"Euler (3-Body)", "Lagrange (3-Body)", "Plummer (N-Body)"}};

enum class SimulationMode {
  CPU,
  GPU,
};

auto simulation_mode {SimulationMode::GPU};

bool input_field_f32(const char* label, f32* value, f32 min = 0.0f, const char* format = "%.3f") {
  if (ImGui::InputScalar(label, ImGuiDataType_Float, value, nullptr, nullptr, format)) {
    if (*value < min) {
      *value = min;
    }
    return true;
  }
  return false;
}

bool input_field_u32(const char* label, u32* value, u32 min = 0u) {
  // Using data type ImGuiDataType_U32 does not prevent entering negative values,
  // they will instead be casted to unsigned values and thus wrapped around.
  // Thus, we use a temporary signed value to work around this.
  if (auto temp {static_cast<i32>(*value)}; ImGui::InputScalar(label, ImGuiDataType_S32, &temp)) {
    if (temp < 0) {
      temp = 0;
    }
    if (temp < static_cast<i32>(min)) {
      temp = static_cast<i32>(min);
    }
    *value = static_cast<u32>(temp);
    return true;
  }
  return false;
}

void center_button(const char* label) {
  auto menu_width {ImGui::GetContentRegionAvail().x};
  auto button_width {ImGui::CalcTextSize(label).x + 2.0f * ImGui::GetStyle().FramePadding.x};

  ImGui::SetCursorPosX(0.5f * (menu_width - button_width));
}

template <std::convertible_to<const char*>... Args>
requires(sizeof...(Args) > 0)
void center_radio_buttons(Args... labels) {
  auto menu_width {ImGui::GetContentRegionAvail().x};
  auto radio_buttons_width {0.0f};
  ((radio_buttons_width += ImGui::CalcTextSize(labels).x + 25.0f), ...);
  radio_buttons_width += ImGui::GetStyle().ItemSpacing.x;

  ImGui::SetCursorPosX(0.5f * (menu_width - radio_buttons_width));
}

auto adjust_rendering_settings(rendering::RenderingSettings* settings, const simulation::ConfigurationType& type)
    -> void {
  // Plummer model creates lots of bodies that are more spread out.
  // The frustum size is therefore larger for this setup.
  settings->frustum_size = type == simulation::ConfigurationType::PLUMMER_N_BODY ? 100.0f : 3.0f;
  settings->body_radius = 0.1f;
}

} // namespace

Menu::Menu(std::unique_ptr<simulation::Simulation>* simulation, simulation::SimulationParametersCPU* parameters_cpu,
           simulation::SimulationParametersGPU* parameters_gpu, simulation::Configuration* configuration,
           rendering::RenderingSettings* settings, bool visible)
    : simulation_ {simulation}, parameters_cpu_ {parameters_cpu}, parameters_gpu_ {parameters_gpu},
      configuration_ {configuration}, settings_ {settings}, visible_ {visible} {

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

  // --- RENDERING SECTION ---

  ImGui::SeparatorText("Rendering");

  input_field_f32("Body Radius", &settings_->body_radius, 0.1f);
  input_field_f32("Frustum Size", &settings_->frustum_size, 0.1f);

  // --- GENERAL SIMULATION SECTION ---

  ImGui::SeparatorText("Simulation (General)");

  center_radio_buttons("CPU", "GPU");
  ImGui::RadioButton("CPU", reinterpret_cast<i32*>(&simulation_mode), 0);
  ImGui::SameLine();
  ImGui::RadioButton("GPU", reinterpret_cast<i32*>(&simulation_mode), 1);

  ImGui::Combo("Configuration Type", reinterpret_cast<i32*>(&configuration_->type),
               CONFIGURATION_TYPE_DESCRIPTIONS.data(), CONFIGURATION_TYPE_DESCRIPTIONS.size());
  input_field_f32("Configuration Radius", &configuration_->radius, 0.1f);
  input_field_f32("Mass Of Each Body", &configuration_->mass, 0.1f);
  input_field_u32("Number Of Bodies", &configuration_->count, 1u);

  // --- SPECIFIC SIMULATION SECTION ---

  if (simulation_mode == SimulationMode::CPU) {
    input_field_f32("Gravitational Constant", &parameters_cpu_->general.G, 0.001f, "%.3e");
    input_field_f32("Time Step", &parameters_cpu_->general.dt, 0.001f);
    input_field_f32("Softening Factor", &parameters_cpu_->general.eps, 0.0f);

    ImGui::SeparatorText("Simulation (CPU Specific)");

    ImGui::Checkbox("Use Barnes-Hut", &parameters_cpu_->use_barnes_hut);
    if (parameters_cpu_->use_barnes_hut) {
      input_field_f32("Theta", &parameters_cpu_->theta, 0.0f);
    }
    input_field_u32("Thread Count", &parameters_cpu_->thread_count, 1u);
  }

  if (simulation_mode == SimulationMode::GPU) {
    input_field_f32("Gravitational Constant", &parameters_gpu_->general.G, 0.001f, "%.3e");
    input_field_f32("Time Step", &parameters_gpu_->general.dt, 0.001f);
    input_field_f32("Softening Factor", &parameters_gpu_->general.eps, 0.0f);

    ImGui::SeparatorText("Simulation (GPU Specific)");

    input_field_u32("Dispatch Size", &parameters_gpu_->dispatch_size, 1u);
    input_field_u32("Tile Size", &parameters_gpu_->tile_size, 1u);
  }

  ImGui::Dummy(ImVec2(0.0f, 10.0f));
  center_button("Create New Simulation");
  if (ImGui::Button("Create New Simulation")) {
    if (simulation_mode == SimulationMode::CPU) {
      *simulation_ = std::make_unique<simulation::Simulation>(
          std::in_place_type<simulation::SimulationCPU>, *parameters_cpu_,
          initialize_configuration(parameters_cpu_->general.G, *configuration_));
    } else if (simulation_mode == SimulationMode::GPU) {
      *simulation_ = std::make_unique<simulation::Simulation>(
          std::in_place_type<simulation::SimulationGPU>, *parameters_gpu_,
          initialize_configuration(parameters_gpu_->general.G, *configuration_));
    }

    adjust_rendering_settings(settings_, configuration_->type);
  }

  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

auto Menu::toggle() -> void {
  visible_ = !visible_;
}

} // namespace nbodysim::ui
