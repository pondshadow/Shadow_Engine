#include "input.h"
#include <iostream>

// 初始化静态成员变量
GLFWwindow* Input::window = nullptr;
float Input::mouse_x = 0.0f;
float Input::mouse_y = 0.0f;
float Input::last_mouse_x = 0.0f;
float Input::last_mouse_y = 0.0f;
float Input::mouse_delta_x = 0.0f;
float Input::mouse_delta_y = 0.0f;
float Input::scroll_offset_x = 0.0f;
float Input::scroll_offset_y = 0.0f;
bool Input::first_mouse = true;

void Input::init(GLFWwindow* win) {
    window = win;

    // 设置 GLFW 回调
    glfwSetKeyCallback(window, key_callback_proxy);
    glfwSetCursorPosCallback(window, mouse_callback_proxy);
    glfwSetScrollCallback(window, scroll_callback_proxy);

    // 初始化鼠标位置为屏幕中心，防止第一帧跳变
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    last_mouse_x = width / 2.0f;
    last_mouse_y = height / 2.0f;
}

// --- 查询接口实现 ---

bool Input::is_key_pressed(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Input::is_mouse_button_pressed(int button) {
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

glm::vec2 Input::get_mouse_position() {
    return glm::vec2(mouse_x, mouse_y);
}

glm::vec2 Input::get_mouse_scroll() {
    return glm::vec2(scroll_offset_x, scroll_offset_y);
}

float Input::get_mouse_delta_x() {
    return mouse_delta_x;
}

float Input::get_mouse_delta_y() {
    return mouse_delta_y;
}

void Input::end_frame() {
    // 每一帧结束时，需要清空“瞬间”产生的数据，比如滚轮和鼠标移动增量
    // 这样如果下一帧鼠标不动，delta 就会归零
    mouse_delta_x = 0.0f;
    mouse_delta_y = 0.0f;
    scroll_offset_x = 0.0f;
    scroll_offset_y = 0.0f;
}

// --- GLFW 回调实现 ---

void Input::key_callback_proxy(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void Input::mouse_callback_proxy(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (first_mouse) {
        last_mouse_x = xpos;
        last_mouse_y = ypos;
        first_mouse = false;
    }

    // [核心修复] 使用 += 累加每一帧内的所有微小移动
    // 注意：Y轴是否取反取决于你的 Camera 逻辑，通常 OpenGL 也是 yposIn
    mouse_delta_x += (xpos - last_mouse_x);
    mouse_delta_y += (last_mouse_y - ypos); // 如果你的视角上下反了，改回 (ypos - last_mouse_y)

    last_mouse_x = xpos;
    last_mouse_y = ypos;

    // update current pos
    mouse_x = xpos;
    mouse_y = ypos;
}

void Input::scroll_callback_proxy(GLFWwindow* window, double xoffset, double yoffset) {
    scroll_offset_x = static_cast<float>(xoffset);
    scroll_offset_y = static_cast<float>(yoffset);
}