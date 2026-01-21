#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Core/shader.h"
#include "Core/camera.h"
#include "Utils/functions.h"
#include "Core/window.h"

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

//相机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// [ImGui] 鼠标控制状态
bool isCursorVisible = false;

// 时间
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 光照位置（可视化灯光方块用）
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// 全局背景色 (决定了所有光照的环境光分量)
float clearColor[3] = {0.05f, 0.05f, 0.05f}; // 默认为深灰

// 全局参数定义：用于 ImGui 调节
// 定向光参数
struct DirLightParams {
    bool enable = true;
    float direction[3] = {-0.2f, -1.0f, -0.3f};
    float color[3]     = {1.0f, 1.0f, 1.0f};
} dirLightParams;
// 点光源 (Point Light) 参数
struct PointLightParams {
    bool enable = true;
    float color[3]     = {1.0f, 1.0f, 1.0f};

    // 衰减系数
    float constant = 1.0f;
    float linear   = 0.09f;
    float quadratic = 0.032f;
} pointLightParams;
// 聚光灯参数
struct SpotLightParams {
    bool enable = true;
    float color[3]     = {1.0f, 1.0f, 1.0f};

    // 角度
    float cutOff = 12.5f;
    float outerCutOff = 17.5f;

    // 衰减
    float constant = 1.0f;
    float linear   = 0.09f;
    float quadratic = 0.032f;
} spotLightParams;

int main()
{
    // 窗口创建
    Window appWindow(SCR_WIDTH, SCR_HEIGHT, "Shadow_Engine");
    // 获取原生指针，供后续的回调和 ImGui 使用
    GLFWwindow* nativeWin = appWindow.getNativeWindow();

    glfwSetCursorPosCallback(nativeWin, mouse_callback);
    glfwSetScrollCallback(nativeWin, scroll_callback);

    // 默认先隐藏光标
    glfwSetInputMode(nativeWin, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    // [ImGui] 初始化
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(nativeWin, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 创建并编译着色器
    Shader lightingShader("assets/shaders/ColorVS.glsl", "assets/shaders/ColorFS.glsl");
    Shader lightCubeShader("assets/shaders/LightVS.glsl", "assets/shaders/LightFS.glsl");

    // 设置顶点数据（及缓冲区）并配置顶点属性
float vertices[] = {
    // positions          // normals           // texture coords
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
};
    // 物体位置
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    // 点光源位置
    glm::vec3 pointLightPositions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    // 生成 VAO VBO
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);
    // 绑定 VBO 并填充数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 绑定 VAO
    glBindVertexArray(cubeVAO);
    // 链接顶点位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 链接法线属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // 链接纹理坐标
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 生成并绑定 VAO (VBO 保持不变，这里用的同样的顶点数据)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 链接顶点位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 加载与创建贴图
    unsigned int diffuseMap = loadTexture("assets/textures/container2.png");
    unsigned int specularMap = loadTexture("assets/textures/container2_specular.png");
    // 渲染循环
    while (!glfwWindowShouldClose(nativeWin))
    {
        // 每帧的时间
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 输入函数
        // -----
        processInput(nativeWin);

        // 渲染部分
        glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // [ImGui] 新的一帧开始
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // [ImGui] 绘制 UI 面板
        ImGui::Begin("BowieEngine Lighting");

        ImGui::Text("Global Settings");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::ColorEdit3("Ambient/Background", clearColor);
        ImGui::Checkbox("Unlock Mouse (Left Alt)", &isCursorVisible);

        ImGui::Separator();

        // 定向光（太阳）
        if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Dir Enable", &dirLightParams.enable);
            ImGui::SliderFloat3("Direction", dirLightParams.direction, -1.0f, 1.0f);
            // 只有一个颜色控件，同时控制 Diffuse 和 Specular
            ImGui::ColorEdit3("Dir Color", dirLightParams.color);
        }
        // 点光源 (灯泡)
        // 2. 点光源
        if (ImGui::CollapsingHeader("Point Lights", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Point Enable", &pointLightParams.enable);
            ImGui::ColorEdit3("Point Color", pointLightParams.color);
            ImGui::Text("Attenuation");
            ImGui::SliderFloat("Linear", &pointLightParams.linear, 0.001f, 1.0f);
            ImGui::SliderFloat("Quadratic", &pointLightParams.quadratic, 0.0001f, 1.0f);
        }

        // 聚光灯 (手电筒)
        if (ImGui::CollapsingHeader("Spotlight", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Spot Enable", &spotLightParams.enable);
            ImGui::ColorEdit3("Spot Color", spotLightParams.color);
            ImGui::Text("Angles");
            ImGui::SliderFloat("Inner", &spotLightParams.cutOff, 1.0f, 45.0f);
            if (spotLightParams.outerCutOff < spotLightParams.cutOff) spotLightParams.outerCutOff = spotLightParams.cutOff;
            ImGui::SliderFloat("Outer", &spotLightParams.outerCutOff, spotLightParams.cutOff, 50.0f);
        }

        ImGui::End(); // 结束窗口

        //光源移动
        // lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        // lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;

        // 绘制立方体
        lightingShader.use();
        glm::vec3 bgVec = glm::make_vec3(clearColor); // 环境光直接取背景色
        glm::vec3 zero(0.0f);

        // 1. 定向光
        // Ambient = 背景色
        // Diffuse = 固有色
        // Specular = 固有色
        glm::vec3 dirCol = glm::make_vec3(dirLightParams.color);
        lightingShader.setVec3("dirLight.direction", dirLightParams.direction[0], dirLightParams.direction[1], dirLightParams.direction[2]);
        lightingShader.setVec3("dirLight.ambient",  dirLightParams.enable ? bgVec  : zero);
        lightingShader.setVec3("dirLight.diffuse",  dirLightParams.enable ? dirCol : zero);
        lightingShader.setVec3("dirLight.specular", dirLightParams.enable ? dirCol : zero);
        // 2. 点光源
        glm::vec3 ptCol = glm::make_vec3(pointLightParams.color);
        for(int i = 0; i < 4; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            lightingShader.setVec3(base + ".position", pointLightPositions[i]);

            lightingShader.setVec3(base + ".ambient",  pointLightParams.enable ? bgVec : zero);
            lightingShader.setVec3(base + ".diffuse",  pointLightParams.enable ? ptCol : zero);
            lightingShader.setVec3(base + ".specular", pointLightParams.enable ? ptCol : zero);

            lightingShader.setFloat(base + ".constant",  pointLightParams.constant);
            lightingShader.setFloat(base + ".linear",    pointLightParams.linear);
            lightingShader.setFloat(base + ".quadratic", pointLightParams.quadratic);
        }
        // 3. 聚光灯
        glm::vec3 spotCol = glm::make_vec3(spotLightParams.color);
        lightingShader.setBool("spotLight.enabled", spotLightParams.enable);
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);

        lightingShader.setVec3("spotLight.ambient",  bgVec);   // 环境光 = 背景色
        lightingShader.setVec3("spotLight.diffuse",  spotCol); // 漫反射 = 固有色
        lightingShader.setVec3("spotLight.specular", spotCol); // 镜面光 = 固有色

        lightingShader.setFloat("spotLight.constant",  spotLightParams.constant);
        lightingShader.setFloat("spotLight.linear",    spotLightParams.linear);
        lightingShader.setFloat("spotLight.quadratic", spotLightParams.quadratic);
        lightingShader.setFloat("spotLight.cutOff",      glm::cos(glm::radians(spotLightParams.cutOff)));
        lightingShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(spotLightParams.outerCutOff)));
        lightingShader.setVec3("viewPos", camera.Position);

        // 设置物体材质属性
        lightingShader.setFloat("material.shininess", 32.0f);
        lightingShader.setInt("material.diffuse", 0);
        lightingShader.setInt("material.specular", 1);
        // 绑定贴图到纹理单元
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);
        // MVP
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.Position);

        // 绘制 10 个箱子
        glBindVertexArray(cubeVAO);
        for(unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // 绘制 4 个灯泡 (点光源)
        lightCubeShader.use();
        // 让灯泡本身的颜色也跟随点光源的固有色，并且受开关影响
        glm::vec3 displayColor = pointLightParams.enable ? ptCol : glm::vec3(0.1f);
        lightCubeShader.setVec3("lightColor", displayColor);
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 4; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // [ImGui] 渲染 ImGui (必须在绘制完场景之后)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // GLFW: 交换颜色缓冲并检查有没有触发事件
        glfwSwapBuffers(nativeWin);
        glfwPollEvents();
    }

    // [ImGui] 清理资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // 可选项：在资源完成其使命后释放所有资源：
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // LFW：在渲染循环结束后释放之前分配的所有资源
    glfwTerminate();
    return 0;
}

// 处理所有输入: 查询 GLFW 是否在本帧内按下/释放了相关按键，并根据情况做出相应反应
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // [ImGui] 鼠标切换逻辑：按住左 Alt 显示鼠标
    // 只有当鼠标不可见（被相机捕获）时，才允许相机移动
    static bool altKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
        if (!altKeyPressed) {
            isCursorVisible = !isCursorVisible;
            if (isCursorVisible) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // 显示鼠标
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // 隐藏并锁定鼠标
                firstMouse = true; // 重置鼠标状态防止跳跃
            }
            altKeyPressed = true;
        }
    } else {
        altKeyPressed = false;
    }

    // 只有鼠标隐藏模式下，才允许移动相机
    if (!isCursorVisible) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.ProcessKeyboard(UPWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWNWARD, deltaTime);
    }
}

// GLFW：当用户改变窗口的大小的时候，调整视口
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// 鼠标回调函数
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // [ImGui] 如果鼠标可见，说明我们在操作 UI，不要旋转相机
    if (isCursorVisible) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// 滚轮回调函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // [ImGui] 同样，操作 UI 时不要缩放相机
    if (isCursorVisible) return;

    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

