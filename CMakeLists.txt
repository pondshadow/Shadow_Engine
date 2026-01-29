cmake_minimum_required(VERSION 3.25)
project(shadow-engine)
set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)


# 自动扫描 src 目录下的源码 (兼容用户自定义文件)
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")
if(NOT SOURCES)
    # 如果没扫描到，兜底使用默认文件 (防止报错)
    set(SOURCES src/main.cpp)
endif()

add_executable(shadow-engine ${SOURCES})
target_link_libraries(shadow-engine PRIVATE glfw ${STB_INCLUDE_DIR} glad::glad imgui::imgui glm::glm assimp::assimp)
