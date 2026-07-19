find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(n-body-sim)
target_link_libraries(
  n-body-sim
  PRIVATE
    glfw
    glad::glad
    glm::glm
    imgui::imgui
)
target_include_directories(
  n-body-sim
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_sources(
  n-body-sim
  PRIVATE
    platform/debug.cpp
    platform/input.cpp
    platform/window.cpp
    rendering/renderer.cpp
    simulation/configuration.cpp
    simulation/octree.cpp
    simulation/simulation_cpu.cpp
    simulation/simulation_gpu.cpp
    simulation/thread_pool.cpp
    ui/menu.cpp
    main.cpp
)
