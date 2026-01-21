#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class Window {
public:
    // 构造函数：负责初始化窗口和OpenGL上下文
    Window(int width, int height, const char* title);
    // 析构函数：负责清理资源
    ~Window();

    // 检查窗口是否应该关闭
    bool shouldClose();
    // 交换缓冲区
    void swapBuffers();
    // 处理窗口事件（大小改变等）
    void processEvents();

    // Getter：获取原生窗口指针（ImGui 和 输入处理需要用到）
    GLFWwindow* getNativeWindow() const { return window; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    GLFWwindow* window;
    int width;
    int height;

    // 这是一个静态函数，因为 GLFW 的回调必须是静态的或全局的
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};