#include "simulation/simulation.hpp"

#include <format>
#include <string_view>

#include <glm/geometric.hpp>

#include "platform/types.hpp"
#include "utils/gl.hpp"
#include "utils/string.hpp"

namespace nbodysim::simulation {

namespace {

constexpr auto TILE_SIZE_REPLACE_TEXT {std::string_view {"%TILE_SIZE%"}};

constexpr auto UPDATE_POSITION_SHADER {std::string_view {
#include "shaders/update_position.comp"
}};

constexpr auto UPDATE_ACCELERATION_SHADER {std::string_view {
#include "shaders/update_acceleration.comp"
}};

} // namespace

SimulationGPU::SimulationGPU(const SimulationParametersGPU& parameters, const std::vector<Body>& bodies)
    : parameters_ {parameters}, bodies_count_ {bodies.size()} {
  const auto update_position_shader {
      utils::replace_all(UPDATE_POSITION_SHADER, TILE_SIZE_REPLACE_TEXT, std::format("{}", parameters_.tile_size))};
  const auto update_position_shader_id {utils::compile_shader(GL_COMPUTE_SHADER, update_position_shader.data())};
  update_position_program_id_ = utils::link_shaders(update_position_shader_id);

  const auto update_acceleration_shader {
      utils::replace_all(UPDATE_ACCELERATION_SHADER, TILE_SIZE_REPLACE_TEXT, std::format("{}", parameters_.tile_size))};
  const auto update_acceleration_shader_id {
      utils::compile_shader(GL_COMPUTE_SHADER, update_acceleration_shader.data())};
  update_acceleration_program_id_ = utils::link_shaders(update_acceleration_shader_id);

  const auto flags {GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT};

  glCreateBuffers(1, &buffer_id_);
  glNamedBufferStorage(buffer_id_, bodies.size() * sizeof(Body), nullptr, flags);

  const auto buffer {glMapNamedBufferRange(buffer_id_, 0, bodies.size() * sizeof(Body), flags)};
  buffer_ = std::span {reinterpret_cast<Body*>(buffer), bodies.size()};

  std::ranges::copy(bodies, buffer_.begin());
}

SimulationGPU::~SimulationGPU() {
  glDeleteProgram(update_position_program_id_);
  glDeleteProgram(update_acceleration_program_id_);
  glDeleteBuffers(1, &buffer_id_);
}

auto SimulationGPU::step() -> void {
  glUseProgram(update_position_program_id_);
  glUniform1ui(glGetUniformLocation(update_position_program_id_, "N"), bodies_count_);
  glUniform1f(glGetUniformLocation(update_position_program_id_, "dt"), parameters_.general.dt);

  glDispatchCompute(parameters_.dispatch_size, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  glUseProgram(update_acceleration_program_id_);
  glUniform1ui(glGetUniformLocation(update_acceleration_program_id_, "N"), bodies_count_);
  glUniform1f(glGetUniformLocation(update_acceleration_program_id_, "G"), parameters_.general.G);
  glUniform1f(glGetUniformLocation(update_acceleration_program_id_, "dt"), parameters_.general.dt);
  glUniform1f(glGetUniformLocation(update_acceleration_program_id_, "eps"), parameters_.general.eps);

  glDispatchCompute(parameters_.dispatch_size, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

auto SimulationGPU::buffer_id() const -> GLuint {
  return buffer_id_;
}

auto SimulationGPU::bodies_count() const -> usize {
  return bodies_count_;
}

} // namespace nbodysim::simulation
