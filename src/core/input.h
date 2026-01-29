#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Input {
public:
    // 初始化输入系统 (绑定回调)
    static void init(GLFWwindow* window);

    // [查询接口] 供外部 (如 Camera, Player) 调用
    static bool is_key_pressed(int key);           // 键盘是否按下
    static bool is_mouse_button_pressed(int button); // 鼠标是否按下
    static glm::vec2 get_mouse_position();         // 获取鼠标绝对位置
    static glm::vec2 get_mouse_scroll();           // 获取滚轮偏移 (X, Y)

    // 获取鼠标自上一帧以来的偏移量 (用于摄像机旋转)
    // 注意：需要在每帧结束时调用 reset_scroll_and_delta
    static float get_mouse_delta_x();
    static float get_mouse_delta_y();

    // [帧末重置] 必须在每一帧渲染结束前调用
    static void end_frame();

private:
    static GLFWwindow* window;

    // 鼠标状态
    static float mouse_x, mouse_y;
    static float last_mouse_x, last_mouse_y;
    static float mouse_delta_x, mouse_delta_y;
    static float scroll_offset_x, scroll_offset_y;
    static bool first_mouse;

    // GLFW 回调函数的静态包装器
    static void key_callback_proxy(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouse_callback_proxy(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback_proxy(GLFWwindow* window, double xoffset, double yoffset);
};