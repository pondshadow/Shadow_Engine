#include <iostream>
#include <vector>
#include <string>

// ---------------------------------------------------------
// 引入依赖头文件
// ---------------------------------------------------------

// ImGui 核心 (虽然逻辑在 GuiLayer，但 main 需要知道一些基础类型)
#include "imgui.h"

// 核心系统 (Core)
#include "core/window.h"       // 窗口管理
#include "core/input.h"        // 输入系统 (键盘/鼠标)

// 渲染层 (Renderer)
#include "renderer/shader.h"   // 着色器程序封装
#include "renderer/texture.h"  // 纹理加载封装
#include "renderer/camera.h"   // 摄像机类
#include "renderer/mesh.h"     // 网格类 (封装了 VAO/VBO/纹理绑定)
#include "renderer/model.h"    // 模型类

// 场景与数据 (Scene)
#include "scene/transform.h"   // 变换组件 (Position/Rotation/Scale)
#include "scene/light_params.h"// 光照参数结构体 (共享数据)
#include "scene/primitives.h"  // 手写的基础图元数据 (如立方体)

// 编辑器层 (Editor)
#include "editor/gui_layer.h"  // UI 界面封装

// ---------------------------------------------------------
// 全局配置与状态
// ---------------------------------------------------------

// 屏幕分辨率常量
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 全局摄像机实例 (初始位置在 Z=3.0)
Camera main_camera(glm::vec3(0.0f, 0.0f, 3.0f));

// 鼠标光标状态
// false = 漫游模式 (FPS模式，鼠标隐藏并锁定)
// true  = UI 模式 (鼠标显示，可以点击面板)
bool is_cursor_visible = false;

// 时间管理 (用于保证移动速度与帧率无关)
float delta_time = 0.0f; // 当前帧与上一帧的时间差
float last_frame = 0.0f;

// 场景环境背景色 (也是环境光的基础颜色)
glm::vec3 clear_color = glm::vec3(0.05f, 0.05f, 0.05f);

// --- 光照参数实例 ---
// 这些变量会被传递给 GuiLayer 进行修改，同时传递给 Shader 进行渲染
DirLightParams dir_params;      // 定向光
PointLightParams point_params;  // 点光源 (共用参数)
SpotLightParams spot_params;    // 聚光灯

// 函数前置声明：负责处理每一帧的业务逻辑 (输入、移动等)
void process_engine_logic(GLFWwindow* window);

// =========================================================================
// MAIN 函数入口
// =========================================================================
int main()
{
    // -----------------------------------------------------
    // 初始化核心系统
    // -----------------------------------------------------
    // 创建窗口 (Window 类内部处理了 GLFW Init 和 Context 创建)
    Window app_window(SCR_WIDTH, SCR_HEIGHT, "Shadow Engine");
    GLFWwindow* native_win = app_window.getNativeWindow();

    // 初始化输入系统 (Input 类会自动接管 GLFW 的回调函数)
    Input::init(native_win);

    // 初始化 UI 系统 (ImGui 的配置)
    GuiLayer::init(native_win);

    // 设置初始输入模式：隐藏光标并锁定，适合 FPS 漫游
    glfwSetInputMode(native_win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 开启 OpenGL 深度测试，确保物体遮挡关系正确
    glEnable(GL_DEPTH_TEST);

    // -----------------------------------------------------
    // 加载渲染资源 (Shader & Texture)
    // -----------------------------------------------------
    // 加载主场景 Shader (处理光照计算)
    Shader main_shader("assets/shaders/main_vertex.glsl", "assets/shaders/main_fragment.glsl");
    // 加载光源 Shader (纯色，用于显示灯泡位置)
    Shader lamp_shader("assets/shaders/LightVS.glsl", "assets/shaders/LightFS.glsl");

    // 加载纹理 (Texture 类自动处理了 stbi_load 和 OpenGL 绑定)
    Texture diffuse_map("assets/textures/container2.png");
    Texture specular_map("assets/textures/container2_specular.png");

    // -----------------------------------------------------
    // 构建 Mesh (网格)
    // -----------------------------------------------------
    // 准备纹理列表
    // Mesh 类会根据 type (texture_diffuse/specular) 自动绑定到 Shader 中对应的采样器
    std::vector<TextureInfo> box_textures;
    box_textures.push_back({ diffuse_map.ID, "texture_diffuse", "" });
    box_textures.push_back({ specular_map.ID, "texture_specular", "" });

    // 实例化实体 Mesh
    // 数据来源：Primitives 类中的手写立方体顶点数据
    std::vector<Vertex> cube_vertices = Primitives::get_cube_vertices();
    std::vector<unsigned int> empty_indices; // 手写数据没有索引 (EBO)

    // cube_mesh 包含了 VAO/VBO 的生成和纹理管理
    Mesh cube_mesh(cube_vertices, empty_indices, box_textures);

    // 实例化光源 Mesh (复用顶点数据，但不需要纹理)
    Mesh light_mesh(cube_vertices, empty_indices, {});

    Model backpack_model("assets/models/teapot.fbx");

    // -----------------------------------------------------
    // 初始化场景对象 (使用 Transform 组件)
    // -----------------------------------------------------
    // 创建 10 个木箱子
    std::vector<Transform> box_transforms(10);
    glm::vec3 cube_positions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), glm::vec3( 2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f), glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f), glm::vec3( 1.5f,  2.0f, -2.5f),
        glm::vec3( 1.5f,  0.2f, -1.5f), glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    for(int i = 0; i < 10; i++) {
        box_transforms[i].position = cube_positions[i];
        // 设置不同的旋转角度，让场景看起来自然些
        box_transforms[i].rotation = glm::vec3(20.0f * i, 15.0f * i, 5.0f * i);
    }

    // 创建 4 个点光源 (可视化灯泡)
    std::vector<Transform> light_transforms(4);
    glm::vec3 point_light_positions[] = {
        glm::vec3( 0.7f,  0.2f,  2.0f), glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f), glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    for(int i = 0; i < 4; i++) {
        light_transforms[i].position = point_light_positions[i];
        light_transforms[i].scale = glm::vec3(0.2f); // 灯泡缩小一点
    }

    // =====================================================
    // 渲染循环 (RENDER LOOP)
    // =====================================================
    while (!app_window.shouldClose())
    {
        // -------------------------------------------------
        // [关键] 帧首重置输入增量
        // -------------------------------------------------
        // 确保上一帧的鼠标移动量 (Delta) 被清零，防止漂移
        Input::end_frame();

        // -------------------------------------------------
        // 逻辑与时间计算
        // -------------------------------------------------
        float current_frame = static_cast<float>(glfwGetTime());
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // [关键] 先处理事件，Input 类会在这里捕获新的鼠标/键盘状态
        app_window.processEvents();

        // 处理引擎逻辑 (读取 Input 状态，更新摄像机等)
        process_engine_logic(native_win);

        // -------------------------------------------------
        // 渲染准备
        // -------------------------------------------------
        // 清除颜色缓冲和深度缓冲
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // -------------------------------------------------
        // UI 帧开始
        // -------------------------------------------------
        GuiLayer::begin_frame();
        // 绘制属性面板，传入数据的指针以便 UI 可以直接修改它们
        GuiLayer::render_panel(&clear_color, &is_cursor_visible, &dir_params, &point_params, &spot_params);

        // -------------------------------------------------
        // 场景渲染 Pass 1: 实体物体 (箱子)
        // -------------------------------------------------
        main_shader.use();

        // 更新矩阵 (MVP 中的 V 和 P)
        glm::mat4 view = main_camera.get_view_matrix();
        glm::mat4 projection = main_camera.get_projection_matrix((float)SCR_WIDTH, (float)SCR_HEIGHT);
        main_shader.setMat4("projection", projection);
        main_shader.setMat4("view", view);
        main_shader.setVec3("viewPos", main_camera.position);

        // 设置光照 Uniforms (使用 light_params 中的数据)
        glm::vec3 bg_vec = clear_color;
        glm::vec3 zero(0.0f);

        // -> 定向光
        main_shader.setVec3("dirLight.direction", dir_params.direction);
        main_shader.setVec3("dirLight.ambient",   dir_params.enable ? bg_vec : zero);
        main_shader.setVec3("dirLight.diffuse",   dir_params.enable ? dir_params.color : zero);
        main_shader.setVec3("dirLight.specular",  dir_params.enable ? dir_params.color : zero);

        // -> 点光源 (循环设置 4 个)
        glm::vec3 pt_col = point_params.color;
        for(int i = 0; i < 4; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            main_shader.setVec3(base + ".position", light_transforms[i].position);
            main_shader.setVec3(base + ".ambient",  point_params.enable ? bg_vec : zero);
            main_shader.setVec3(base + ".diffuse",  point_params.enable ? pt_col : zero);
            main_shader.setVec3(base + ".specular", point_params.enable ? pt_col : zero);
            main_shader.setFloat(base + ".constant",  point_params.constant);
            main_shader.setFloat(base + ".linear",    point_params.linear);
            main_shader.setFloat(base + ".quadratic", point_params.quadratic);
        }

        // -> 聚光灯
        glm::vec3 spot_col = spot_params.color;
        main_shader.setBool("spotLight.enabled", spot_params.enable);
        main_shader.setVec3("spotLight.position", main_camera.position);
        main_shader.setVec3("spotLight.direction", main_camera.front);
        main_shader.setVec3("spotLight.ambient",  bg_vec);
        main_shader.setVec3("spotLight.diffuse",  spot_col);
        main_shader.setVec3("spotLight.specular", spot_col);
        main_shader.setFloat("spotLight.constant",  spot_params.constant);
        main_shader.setFloat("spotLight.linear",    spot_params.linear);
        main_shader.setFloat("spotLight.quadratic", spot_params.quadratic);
        main_shader.setFloat("spotLight.cutOff",      glm::cos(glm::radians(spot_params.cut_off)));
        main_shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(spot_params.outer_cut_off)));

        // 设置材质属性 (纹理已由 Mesh::Draw 自动绑定)
        main_shader.setFloat("material.shininess", 32.0f);
        // 注意：Mesh::Draw 会自动绑定纹理并设置 "material.texture_diffuse1" 等 Uniform
        // 只要你的 Shader 里的采样器命名符合 Mesh 的约定即可 (LearnOpenGL 风格)

        // 绘制所有箱子
        for(auto& box : box_transforms) {
            // 通过 Transform 组件获取模型矩阵 (Model Matrix)
            main_shader.setMat4("model", box.get_model_matrix());
            // [重点] 使用 Mesh 类进行绘制，它会自动绑定 VAO 和 Texture
            cube_mesh.Draw(main_shader);
        }

        glm::mat4 model = glm::mat4(1.0f); // 设置位置
        main_shader.setMat4("model", model);
        backpack_model.Draw(main_shader);

        // -------------------------------------------------
        // 场景渲染 Pass 2: 光源可视化 (画灯泡)
        // -------------------------------------------------
        lamp_shader.use();
        lamp_shader.setMat4("projection", projection);
        lamp_shader.setMat4("view", view);
        // 如果点光源开启，显示对应颜色；否则显示暗灰色
        lamp_shader.setVec3("lightColor", point_params.enable ? point_params.color : glm::vec3(0.1f));

        for(auto& light : light_transforms) {
            lamp_shader.setMat4("model", light.get_model_matrix());
            // [重点] 灯泡也是一个 Mesh，只是没有纹理
            light_mesh.Draw(lamp_shader);
        }

        // -------------------------------------------------
        // 6. 帧末处理 (End Frame)
        // -------------------------------------------------
        // 渲染 UI (Overlay)
        GuiLayer::end_frame();

        // 交换前后缓冲区
        app_window.swapBuffers();

        // 注意：Input::end_frame() 已经移到了循环最开始
    }

    // -----------------------------------------------------
    // 资源清理
    // -----------------------------------------------------
    GuiLayer::shutdown();
    // VBO/VAO 的清理现在由 Mesh 类的生命周期管理（如果不手动 delete，Mesh 析构时并不会自动 glDeleteBuffer，
    // 通常引擎中会有专门的 ResourceManager。在这个简单示例中，程序退出时操作系统会回收显存）

    return 0;
}

// =========================================================================
// 引擎逻辑处理函数
// =========================================================================
void process_engine_logic(GLFWwindow* window)
{
    // 退出检查
    if (Input::is_key_pressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    // 鼠标锁定切换逻辑 (按 Left Alt 切换)
    static bool alt_pressed = false;
    if (Input::is_key_pressed(GLFW_KEY_LEFT_ALT)) {
        if (!alt_pressed) {
            is_cursor_visible = !is_cursor_visible;
            // 切换 GLFW 鼠标模式
            glfwSetInputMode(window, GLFW_CURSOR, is_cursor_visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            alt_pressed = true;
        }
    } else {
        alt_pressed = false;
    }

    // 漫游控制 (仅在光标不可见时启用)
    if (!is_cursor_visible) {
        // 键盘移动
        if (Input::is_key_pressed(GLFW_KEY_W)) main_camera.process_keyboard(camera_movement::FORWARD, delta_time);
        if (Input::is_key_pressed(GLFW_KEY_S)) main_camera.process_keyboard(camera_movement::BACKWARD, delta_time);
        if (Input::is_key_pressed(GLFW_KEY_A)) main_camera.process_keyboard(camera_movement::LEFT, delta_time);
        if (Input::is_key_pressed(GLFW_KEY_D)) main_camera.process_keyboard(camera_movement::RIGHT, delta_time);
        if (Input::is_key_pressed(GLFW_KEY_E)) main_camera.process_keyboard(camera_movement::UPWARD, delta_time);
        if (Input::is_key_pressed(GLFW_KEY_Q)) main_camera.process_keyboard(camera_movement::DOWNWARD, delta_time);

        // 鼠标旋转 (使用 Input 类获取的 Delta 值)
        // 因为 Input::end_frame() 在循环开头，这里获取的是本帧 processEvents() 产生的新 Delta
        main_camera.process_mouse_movement(Input::get_mouse_delta_x(), Input::get_mouse_delta_y());

        // 滚轮缩放
        main_camera.process_mouse_scroll(Input::get_mouse_scroll().y);
    }
}