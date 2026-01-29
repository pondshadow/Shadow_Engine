#pragma once

// 引入共享参数结构
#include "../scene/light_params.h"

class GuiLayer {
public:
    // 初始化 (传入原生窗口指针 void* 是为了解耦，实现里强转)
    static void init(void* window);

    // 每一帧开始时调用
    static void begin_frame();

    // 每一帧结束时调用 (负责 Render)
    static void end_frame();

    // 核心：绘制属性面板
    // 我们传入指针，这样 UI 里的修改会直接反馈到 main 的变量里
    static void render_panel(
        glm::vec3* clear_color,
        bool* is_mouse_locked,
        DirLightParams* dir_light,
        PointLightParams* point_light,
        SpotLightParams* spot_light
    );

    // 清理资源
    static void shutdown();
};