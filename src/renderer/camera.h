#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 定义相机移动方向的枚举
enum class camera_movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UPWARD,
    DOWNWARD
};

// 默认相机参数常量
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.05f;
const float ZOOM        =  45.0f;

class Camera
{
public:
    // --- 相机属性 ---
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 world_up;

    // 欧拉角
    float yaw;
    float pitch;

    // 相机选项
    float movement_speed;
    float mouse_sensitivity;
    float zoom; // 实际就是 FOV (Field of View)

    // 视锥体裁剪平面 (新增，引擎通常需要配置这个)
    float near_plane;
    float far_plane;

    // --- 构造函数 ---
    // 向量构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW,
           float pitch = PITCH);

    // 标量构造函数
    Camera(float pos_x, float pos_y, float pos_z,
           float up_x, float up_y, float up_z,
           float yaw, float pitch);

    // --- 矩阵获取 (核心功能) ---

    // 获取 View 矩阵 (LookAt)
    // 负责将世界坐标转换为观察坐标
    glm::mat4 get_view_matrix() const;

    // 获取 Projection 矩阵 (Perspective)
    // 负责将观察坐标转换为裁剪坐标 (处理透视效果)
    // 需要传入当前窗口/视口的宽高来计算宽高比 (Aspect Ratio)
    glm::mat4 get_projection_matrix(float width, float height) const;

    // --- 输入处理 ---

    // 处理键盘输入 (位置移动)
    void process_keyboard(camera_movement direction, float delta_time);

    // 处理鼠标移动 (视角旋转)
    void process_mouse_movement(float xoffset, float yoffset, bool constrain_pitch = true);

    // 处理鼠标滚轮 (FOV缩放)
    void process_mouse_scroll(float yoffset);

private:
    // 根据当前的欧拉角更新 Front, Right, Up 向量
    void update_camera_vectors();
};