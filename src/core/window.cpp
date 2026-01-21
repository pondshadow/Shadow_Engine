#include "window.h"

Window::Window(int width, int height, const char* title)
    : width(width), height(height), window(nullptr)
{
    // 1. 初始化 GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 2. 创建窗口
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        // 这里可以使用异常或者错误码，暂时简单处理
        exit(-1);
    }

    // 3. 设置上下文
    glfwMakeContextCurrent(window);

    // 4. 设置窗口大小回调 (使用我们类内部的静态函数)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 5. 初始化 GLAD (必须在创建上下文之后)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }
}

Window::~Window() {
    // 资源清理
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(window);
}

void Window::processEvents() {
    // 处理所有挂起的事件
    glfwPollEvents();
}

// 静态回调函数的实现
void Window::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}