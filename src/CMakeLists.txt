find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(n-body-sim)
target_sources(n-body-sim PRIVATE main.cpp)
target_link_libraries(n-body-sim PRIVATE glfw glad::glad glm::glm imgui::imgui)
