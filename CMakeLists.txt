cmake_minimum_required(VERSION 3.25)
project(shadow-engine)
set(CMAKE_CXX_STANDARD 20)

find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(shadow-engine src/main.cpp)
target_link_libraries(shadow-engine PRIVATE assimp::assimp imgui::imgui glad::glad glm::glm ${STB_INCLUDE_DIR} glfw)