#include "simulation/simulation.hpp"

#include <format>
#include <string_view>

#include <glm/geometric.hpp>

#include "platform/types.hpp"
#include "utils/gl.hpp"
#include "utils/string.hpp"

namespace nbodysim::simulation {

namespace {

constexpr auto TILE_SIZE_ID {std::string_view {"%TILE_SIZE%"}};

constexpr auto UPDATE_POSITION_SHADER {std::string_view {
#include "shaders/update_position.comp"
}};

constexpr auto UPDATE_ACCELERATION_SHADER {std::string_view {
#include "shaders/update_acceleration.comp"
}};

} // namespace

SimulationGPU::SimulationGPU(const SimulationParametersGPU& parameters, const std::vector<Body>& bodies)
    : parameters_ {parameters} {
  const auto position_shader {
      utils::replace_all(UPDATE_POSITION_SHADER, TILE_SIZE_ID, std::format("{}", parameters_.tile_size))};
  const auto position_shader_id {utils::compile_shader(GL_COMPUTE_SHADER, position_shader.data())};
  position_program_id_ = utils::link_shaders(position_shader_id);

  glUseProgram(position_program_id_);
  uniform_position_count_ = glGetUniformLocation(position_program_id_, "count");
  uniform_position_dt_ = glGetUniformLocation(position_program_id_, "dt");
  glUseProgram(0);

  const auto acceleration_shader {
      utils::replace_all(UPDATE_ACCELERATION_SHADER, TILE_SIZE_ID, std::format("{}", parameters_.tile_size))};
  const auto acceleration_shader_id {utils::compile_shader(GL_COMPUTE_SHADER, acceleration_shader.data())};
  acceleration_program_id_ = utils::link_shaders(acceleration_shader_id);

  glUseProgram(acceleration_program_id_);
  uniform_acceleration_count_ = glGetUniformLocation(acceleration_program_id_, "count");
  uniform_acceleration_dt_ = glGetUniformLocation(acceleration_program_id_, "dt");
  uniform_acceleration_G_ = glGetUniformLocation(acceleration_program_id_, "G");
  uniform_acceleration_eps_ = glGetUniformLocation(acceleration_program_id_, "eps");
  glUseProgram(0);

  const auto flags {GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT};

  glCreateBuffers(1, &buffer_id_);
  glNamedBufferStorage(buffer_id_, bodies.size() * sizeof(Body), nullptr, flags);

  const auto buffer {glMapNamedBufferRange(buffer_id_, 0, bodies.size() * sizeof(Body), flags)};
  buffer_ = std::span {reinterpret_cast<Body*>(buffer), bodies.size()};

  std::ranges::copy(bodies, buffer_.begin());
}

SimulationGPU::~SimulationGPU() {
  glDeleteProgram(position_program_id_);
  glDeleteProgram(acceleration_program_id_);
  glDeleteBuffers(1, &buffer_id_);
}

auto SimulationGPU::step() -> void {
  glUseProgram(position_program_id_);
  glUniform1ui(uniform_position_count_, buffer_.size());
  glUniform1f(uniform_position_dt_, parameters_.dt);

  glDispatchCompute(parameters_.dispatch_size, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  glUseProgram(acceleration_program_id_);
  glUniform1ui(uniform_acceleration_count_, buffer_.size());
  glUniform1f(uniform_acceleration_dt_, parameters_.dt);
  glUniform1f(uniform_acceleration_G_, parameters_.G);
  glUniform1f(uniform_acceleration_eps_, parameters_.eps);

  glDispatchCompute(parameters_.dispatch_size, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

auto SimulationGPU::buffer_id() const -> GLuint {
  return buffer_id_;
}

auto SimulationGPU::count() const -> usize {
  return buffer_.size();
}

} // namespace nbodysim::simulation
